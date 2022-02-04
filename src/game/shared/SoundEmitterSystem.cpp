//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include <ctype.h>
#include <KeyValues.h>
#include "engine/IEngineSound.h"
#include "SoundEmitterSystem/isoundemittersystembase.h"
#include "igamesystem.h"
#include "soundchars.h"
#include "filesystem.h"
#include "tier0/vprof.h"
#include "checksum_crc.h"
#include "tier0/icommandline.h"

// NVNT haptic utils
#include "haptics/haptic_utils.h"

#ifndef CLIENT_DLL
#include "envmicrophone.h"
#include "sceneentity.h"
#else
#include <vgui_controls/Controls.h>
#include <vgui/IVGui.h>
#include "hud_closecaption.h"
#define CRecipientFilter C_RecipientFilter
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static ConVar sv_soundemitter_trace("sv_soundemitter_trace", "0", FCVAR_REPLICATED, "Show all EmitSound calls including their symbolic name and the actual wave file they resolved to\n");

#ifdef STAGING_ONLY
static ConVar sv_snd_filter("sv_snd_filter", "", FCVAR_REPLICATED, "Filters out all sounds not containing the specified string before being emitted\n");
#endif // STAGING_ONLY

extern ISoundEmitterSystemBase *soundemitterbase;
static ConVar *g_pClosecaption = NULL;

static bool g_bPermitDirectSoundPrecache = false;

#if !defined( CLIENT_DLL )

void ClearModelSoundsCache();

#endif // !CLIENT_DLL

#if defined( CLIENT_DLL )
#define UTIL_GetLocalPlayer() C_BasePlayer::GetLocalPlayer()
#endif

// The new code is responsible for adding a delay
// if the emitted sound is reasonably far away. This is done
// in order to make the system to respect the speed of sound
// so the gameplay experience is a bit more realistic :)
// The most simple way to achieve such behaviour was to
// cut IEngineSound calls away and use OpenAL (see openal branch)
// but it required... well... a clusterfuck of things to fix.
// It was just easier for me to re-invent the way SoundEmitterSystem
// calls the engine EmitSound() functions: now it adds them into
// a queue with a specific timer value; these queues are updated
// in a separate thread (CDeferredSoundWorkerThread) and invoked
// when the time comes.
// I called the system "deferred" audio because you can _defer_
// the explicit call to engine[sound]->Emit[Ambient]Sound[ByHandle]()
// in order to add some effects like Latency of Sound (LoS).

struct DeferredCall_EmitSound_t {
    float flTriggerTime;
    CRecipientFilter filter;
    int iEntIndex;
    EmitSound_t ep;
    bool bHasOrigin;
    Vector vOrigin;
    CUtlString soundname;
};

struct DeferredCall_EmitSoundByHandle_t {
    float flTriggerTime;
    int iEntIndex;
    EmitSound_t ep;
    bool bHasOrigin;
    Vector vOrigin;
    CUtlString soundname;
    CRecipientFilter filter;
    HSOUNDSCRIPTHANDLE hSoundHandle;
};

struct DeferredCall_EmitAmbientSound_t {
    float flTriggerTime;
    int iEntIndex;
    Vector vOrigin;
    CUtlString soundname;
    float flVolume;
    int iFlags;
    int iPitch;
    float flSoundTime;
};

struct DeferredCall_EmitAmbientSound2_t {
    float flTriggerTime;
    int iEntIndex;
    Vector vOrigin;
    CUtlString sample;
    float flVolume;
    soundlevel_t soundlevel;
    int iFlags;
    int iPitch;
    float flSoundTime;
};

struct DeferredCall_StopSound_t {
    float flTriggerTime;
    int iEntIndex;
    CUtlString soundname;
    HSOUNDSCRIPTHANDLE hSoundHandle;
};

struct DeferredCall_StopSound2_t {
    float flTriggerTime;
    int iEntIndex;
    int iChannel;
    CUtlString sample;
};

class CDeferredSoundWorkerThread : public CWorkerThread {
public:
    CDeferredSoundWorkerThread();
    int Run();
    bool Init();
    void OnExit();
    void Cleanup();

    void EmitSound(float flTriggerTime, IRecipientFilter &filter, int iEntIndex, const EmitSound_t &ep);
    void EmitSoundByHandle(float flTriggerTime, int iEntIndex, const EmitSound_t &ep, IRecipientFilter &filter, HSOUNDSCRIPTHANDLE hSoundHandle);
    void EmitAmbientSound(float flTriggerTime, int iEntIndex, const Vector &vOrigin, const char *pszSoundName, float flVolume, int iFlags, int iPitch, float flSoundTime);
    void EmitAmbientSound2(float flTriggerTime, int iEntIndex, const Vector &vOrigin, const char *pszSample, float flVolume, soundlevel_t soundlevel, int iFlags, int iPitch, float flSoundTime);
    void StopSound(float flTriggerTime, int iEntIndex, const char *pszSoundName, HSOUNDSCRIPTHANDLE hSoundHandle = SOUNDEMITTER_INVALID_HANDLE);
    void StopSound2(float flTriggerTime, int iEntIndex, int iChannel, const char *pszSample);

private:
    CUtlVectorMT<CUtlVector<DeferredCall_EmitSound_t *>> m_utlEmitSound;
    CUtlVectorMT<CUtlVector<DeferredCall_EmitSoundByHandle_t *>> m_utlEmitSoundByHandle;
    CUtlVectorMT<CUtlVector<DeferredCall_EmitAmbientSound_t *>> m_utlEmitAmbientSound;
    CUtlVectorMT<CUtlVector<DeferredCall_EmitAmbientSound2_t *>> m_utlEmitAmbientSound2;
    CUtlVectorMT<CUtlVector<DeferredCall_StopSound_t *>> m_utlStopSound;
    CUtlVectorMT<CUtlVector<DeferredCall_StopSound2_t *>> m_utlStopSound2;

public:
    enum { SND_WORKER_EXIT = 42 };
};

static ConVar snd_los_enabled("snd_los_enabled", "1", FCVAR_ARCHIVE | FCVAR_REPLICATED, "Enable Latency of Sound (LoS) (0 - disabled, otherwise - enabled)");
static ConVar snd_los_speed_of_sound("snd_los_speed_of_sound", "343", FCVAR_CHEAT | FCVAR_REPLICATED, "Speed of sound (in m/s)");
static ConVar snd_los_min_distance("snd_los_min_distance", "512", FCVAR_ARCHIVE | FCVAR_REPLICATED, "Minimal distance at which LoS works (in inches)");
static ConVar snd_los_debug("snd_los_debug", "0", FCVAR_REPLICATED, "Print sound delay when the event is queued (0 - disabled, otherwise - enabled)");

static bool LOS_CalcDelay(const Vector &vOrigin, float &flDelay)
{
    if(snd_los_enabled.GetBool()) {
        CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
        if(pPlayer) {
            Vector vDelta = vOrigin;
            vDelta -= pPlayer->GetAbsOrigin();
            float flDistance = vDelta.Length();
            if(flDistance >= snd_los_min_distance.GetFloat()) {
                flDelay = flDistance * (METERS_PER_INCH / snd_los_speed_of_sound.GetFloat());
                if(snd_los_debug.GetBool())
                    ConDColorMsg(LOG_COLOR_CYAN, "LOS_CalcDelay:  delay: %fs dist: %.2f\n", flDelay, flDistance);
                return true;
            }
        }
    }

    return false;
}

void WaveTrace(char const *wavname, char const *funcname)
{
    static CUtlSymbolTable s_WaveTrace;

    // Make sure we only show the message once
    if(UTL_INVAL_SYMBOL == s_WaveTrace.Find(wavname)) {
        ConDColorMsg(LOG_COLOR_CYAN, "%s directly referenced wave %s (should use game_sounds.txt system instead)\n", funcname, wavname);
        s_WaveTrace.AddString(wavname);
    }
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  : &src -
//-----------------------------------------------------------------------------
EmitSound_t::EmitSound_t(const CSoundParameters &src)
{
    m_nChannel = src.channel;
    m_pSoundName = src.soundname;
    m_flVolume = src.volume;
    m_SoundLevel = src.soundlevel;
    m_nFlags = 0;
    m_nPitch = src.pitch;
    m_nSpecialDSP = 0;
    m_pOrigin = 0;
    m_flSoundTime = (src.delay_msec == 0) ? 0.0f : gpGlobals->curtime + ((float)src.delay_msec / 1000.0f);
    m_pflSoundDuration = 0;
    m_bEmitCloseCaption = true;
    m_bWarnOnMissingCloseCaption = false;
    m_bWarnOnDirectWaveReference = false;
    m_nSpeakerEntity = -1;
}

void Hack_FixEscapeChars(char *str)
{
    int len = Q_strlen(str) + 1;
    char *i = str;
    char *o = (char *)_alloca(len);
    char *osave = o;

    while(*i) {
        if(*i == '\\') {
            switch(*(i + 1)) {
                case 'n':
                    *o = '\n';
                    ++i;
                    break;
                default:
                    *o = *i;
                    break;
            }
        }
        else {
            *o = *i;
        }

        ++i;
        ++o;
    }

    *o = 0;

    Q_strncpy(str, osave, len);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
class CSoundEmitterSystem : public CBaseGameSystem {
public:
    virtual char const *Name()
    {
        return "CSoundEmitterSystem";
    }

    CDeferredSoundWorkerThread m_workerThread;

#if !defined( CLIENT_DLL )
    bool m_bLogPrecache;
    FileHandle_t m_hPrecacheLogFile;
    CUtlSymbolTable m_PrecachedScriptSounds;

public:
    CSoundEmitterSystem(char const *pszName) : m_bLogPrecache(false), m_hPrecacheLogFile(FILESYSTEM_INVALID_HANDLE)
    {
    }

    void LogPrecache(char const *soundname)
    {
        if(!m_bLogPrecache)
            return;

        // Make sure we only show the message once
        if(UTL_INVAL_SYMBOL != m_PrecachedScriptSounds.Find(soundname))
            return;

        if(m_hPrecacheLogFile == FILESYSTEM_INVALID_HANDLE)
            StartLog();

        m_PrecachedScriptSounds.AddString(soundname);

        if(m_hPrecacheLogFile != FILESYSTEM_INVALID_HANDLE) {
            filesystem->Write("\"", 1, m_hPrecacheLogFile);
            filesystem->Write(soundname, Q_strlen(soundname), m_hPrecacheLogFile);
            filesystem->Write("\"\n", 2, m_hPrecacheLogFile);
        }
        else {
            Warning("CSoundEmitterSystem: Disabling precache logging due to file i/o problem!!!\n");
            m_bLogPrecache = false;
        }
    }

    void StartLog()
    {
        m_PrecachedScriptSounds.RemoveAll();

        if(!m_bLogPrecache)
            return;

        if(FILESYSTEM_INVALID_HANDLE != m_hPrecacheLogFile)
            return;

        filesystem->CreateDirHierarchy("reslists", "DEFAULT_WRITE_PATH");

        // open the new level reslist
        char path[_MAX_PATH];
        Q_snprintf(path, sizeof(path), "reslists\\%s.snd", gpGlobals->mapname.ToCStr());
        m_hPrecacheLogFile = filesystem->Open(path, "wt", "GAME");
    }

    void FinishLog()
    {
        if(FILESYSTEM_INVALID_HANDLE != m_hPrecacheLogFile) {
            filesystem->Close(m_hPrecacheLogFile);
            m_hPrecacheLogFile = FILESYSTEM_INVALID_HANDLE;
        }

        m_PrecachedScriptSounds.RemoveAll();
    }

#else
    CSoundEmitterSystem(char const *name)
    {
    }
#endif

    // IServerSystem stuff
    virtual bool Init()
    {
        ConColorMsg(LOG_COLOR_CYAN, "CSoundEmitterSystem::Init()\n");

        Assert(soundemitterbase);

#if !defined( CLIENT_DLL )
        m_bLogPrecache = CommandLine()->CheckParm("-makereslists") ? true : false;
#endif

        g_pClosecaption = cvar->FindVar("closecaption");
        Assert(g_pClosecaption);

        if(!m_workerThread.IsAlive())
            m_workerThread.Start();

        return soundemitterbase->ModInit();
    }

    virtual void Shutdown()
    {
        Assert(soundemitterbase);

        if(m_workerThread.IsAlive())
            m_workerThread.CallWorker(CDeferredSoundWorkerThread::SND_WORKER_EXIT);

#if !defined( CLIENT_DLL )
        FinishLog();
#endif

        soundemitterbase->ModShutdown();
    }

    void ReloadSoundEntriesInList(IFileList *pFilesToReload)
    {
        soundemitterbase->ReloadSoundEntriesInList(pFilesToReload);
    }

    virtual void TraceEmitSound(char const *fmt, ...)
    {
        if(!sv_soundemitter_trace.GetBool())
            return;

        va_list argptr;
        char string[256];
        va_start(argptr, fmt);
        Q_vsnprintf(string, sizeof(string), fmt, argptr);
        va_end(argptr);

        // Spew to console
        ConColorMsg(LOG_COLOR_CYAN, "CSoundEmitterSystem: %s %s", CBaseEntity::IsServer() ? "(sv)" : "(cl)", string);
    }

    // Precache all wave files referenced in wave or rndwave keys
    virtual void LevelInitPreEntity()
    {
        char mapname[256];

#if !defined( CLIENT_DLL )
        StartLog();
        Q_snprintf(mapname, sizeof(mapname), "maps/%s", STRING(gpGlobals->mapname));
#else
        Q_strncpy(mapname, engine->GetLevelName(), sizeof(mapname));
#endif

        Q_FixSlashes(mapname);
        Q_strlower(mapname);

        // Load in any map specific overrides
        char scriptfile[512];
        Q_StripExtension(mapname, scriptfile, sizeof(scriptfile));
        Q_strncat(scriptfile, "_level_sounds.txt", sizeof(scriptfile), COPY_ALL_CHARACTERS);

        if(filesystem->FileExists(scriptfile, "GAME")) {
            soundemitterbase->AddSoundOverrides(scriptfile);
        }

#if !defined( CLIENT_DLL )
        for(int i = soundemitterbase->First(); i != soundemitterbase->InvalidIndex(); i = soundemitterbase->Next(i)) {
            CSoundParametersInternal *pParams = soundemitterbase->InternalGetParametersForSound(i);
            if(pParams->ShouldPreload()) {
                InternalPrecacheWaves(i);
            }
        }
#endif
    }

    virtual void LevelInitPostEntity()
    {
    }

    virtual void LevelShutdownPostEntity()
    {
        soundemitterbase->ClearSoundOverrides();

        m_workerThread.Cleanup();

#if !defined( CLIENT_DLL )
        FinishLog();
#endif
    }

    void InternalPrecacheWaves(int soundIndex)
    {
        CSoundParametersInternal *internal = soundemitterbase->InternalGetParametersForSound(soundIndex);
        if(!internal)
            return;

        int waveCount = internal->NumSoundNames();
        if(!waveCount) {
            ConDColorMsg(LOG_COLOR_CYAN, "CSoundEmitterSystem: sounds.txt entry '%s' has no waves listed under 'wave' or 'rndwave' key!!!\n", soundemitterbase->GetSoundName(soundIndex));
        }
        else {
            g_bPermitDirectSoundPrecache = true;
            for(int wave = 0; wave < waveCount; wave++)
                CBaseEntity::PrecacheSound(soundemitterbase->GetWaveName(internal->GetSoundNames()[wave].symbol));
            g_bPermitDirectSoundPrecache = false;
        }
    }

    void InternalPrefetchWaves(int soundIndex)
    {
        CSoundParametersInternal *internal = soundemitterbase->InternalGetParametersForSound(soundIndex);
        if(!internal)
            return;

        int waveCount = internal->NumSoundNames();
        if(!waveCount) {
            ConDColorMsg(LOG_COLOR_CYAN, "CSoundEmitterSystem: sounds.txt entry '%s' has no waves listed under 'wave' or 'rndwave' key!!!\n", soundemitterbase->GetSoundName(soundIndex));
        }
        else {
            for(int wave = 0; wave < waveCount; wave++)
                CBaseEntity::PrefetchSound(soundemitterbase->GetWaveName(internal->GetSoundNames()[wave].symbol));
        }
    }

    HSOUNDSCRIPTHANDLE PrecacheScriptSound(const char *soundname)
    {
        int soundIndex = soundemitterbase->GetSoundIndex(soundname);
        if(!soundemitterbase->IsValidIndex(soundIndex)) {
            if(Q_stristr(soundname, ".wav") || Q_strstr(soundname, ".mp3")) {
                g_bPermitDirectSoundPrecache = true;
                CBaseEntity::PrecacheSound(soundname);
                g_bPermitDirectSoundPrecache = false;
                return SOUNDEMITTER_INVALID_HANDLE;
            }

#if !defined( CLIENT_DLL )
            if(soundname[0]) {
                static CUtlSymbolTable s_PrecacheScriptSoundFailures;

                // Make sure we only show the message once
                if(UTL_INVAL_SYMBOL == s_PrecacheScriptSoundFailures.Find(soundname)) {
                    ConDColorMsg(LOG_COLOR_CYAN, "CSoundEmitterSystem::PrecacheScriptSound: caching '%s' failed, no such sound script entry\n", soundname);
                    s_PrecacheScriptSoundFailures.AddString(soundname);
                }
            }
#endif
            return (HSOUNDSCRIPTHANDLE)soundIndex;
        }

#if !defined( CLIENT_DLL )
        LogPrecache(soundname);
#endif

        InternalPrecacheWaves(soundIndex);

        return (HSOUNDSCRIPTHANDLE)soundIndex;
    }

    void PrefetchScriptSound(const char *soundname)
    {
        int soundIndex = soundemitterbase->GetSoundIndex(soundname);
        if(!soundemitterbase->IsValidIndex(soundIndex)) {
            if(Q_stristr(soundname, ".wav") || Q_strstr(soundname, ".mp3")) {
                CBaseEntity::PrefetchSound(soundname);
            }
            return;
        }

        InternalPrefetchWaves(soundIndex);
    }
public:
    void Deferred_EmitSound(DeferredCall_EmitSound_t *pCall)
    {
        const char *pszSoundName = pCall->soundname.Get();

#if defined(STAGING_ONLY)
        const char *pszSndFilterString = sv_snd_filter.GetString();
        if(pszSndFilterString[0] && !Q_stristr(pszSoundName, pszSndFilterString))
            return;
#endif

        if(pCall->ep.m_bWarnOnDirectWaveReference && Q_stristr(pszSoundName, ".wav"))
            WaveTrace(pszSoundName, "EmitSound");

#if defined( _DEBUG ) && !defined( CLIENT_DLL )
        if(!enginesound->IsSoundPrecached(pszSoundName))
            ConColorMsg(LOG_COLOR_CYAN, "Deferred_EmitSound: Sound %s was not precached\n", pszSoundName);
#endif

        enginesound->EmitSound(pCall->filter, pCall->iEntIndex, pCall->ep.m_nChannel, pszSoundName, pCall->ep.m_flVolume, pCall->ep.m_SoundLevel, pCall->ep.m_nFlags, pCall->ep.m_nPitch, pCall->ep.m_nSpecialDSP, pCall->bHasOrigin ? &pCall->vOrigin : NULL, NULL, &pCall->ep.m_UtlVecSoundOrigin, true, pCall->ep.m_flSoundTime, pCall->ep.m_nSpeakerEntity);

        TraceEmitSound("Deferred_EmitSound: Raw wave emitted '%s' (ent %i)\n", pszSoundName, pCall->iEntIndex);
    }

    void Deferred_EmitSoundByHandle(DeferredCall_EmitSoundByHandle_t *pCall)
    {
        const char *pszSoundName = pCall->soundname.Get();

        gender_t gender = GENDER_NONE; // attack helicopter
        CBaseEntity *pEntity = CBaseEntity::Instance(pCall->iEntIndex);
        if(pEntity)
            gender = soundemitterbase->GetActorGender(STRING(pEntity->GetModelName()));

        CSoundParameters params;
        if(!soundemitterbase->GetParametersForSoundEx(pszSoundName, pCall->hSoundHandle, params, gender, true))
            return;

        if(!params.soundname[0])
            return;

#if defined(STAGING_ONLY)
        const char *pszSndFilterString = sv_snd_filter.GetString();
        if(pszSndFilterString[0] && !Q_stristr(params.soundname, pszSndFilterString))
            return;
#endif

        if(!Q_strnicmp(params.soundname, "vo", 2) && !(params.channel == CHAN_STREAM || params.channel == CHAN_VOICE || params.channel == CHAN_VOICE2))
            DevWarning("Deferred_EmitSoundByHandle: Voice wave file %s doesn't specify CHAN_VOICE, CHAN_VOICE2 or CHAN_STREAM for sound %s\n", params.soundname, pCall->ep.m_pSoundName);

        // Handle SND_CHANGE_PITCH and SND_CHANGE_VOL
        if(pCall->ep.m_nFlags & SND_CHANGE_PITCH)
            params.pitch = pCall->ep.m_nPitch;
        if(pCall->ep.m_nFlags & SND_CHANGE_VOL)
            params.volume = pCall->ep.m_flVolume;

#if defined(_DEBUG) && !defined(CLIENT_DLL)
        if(!enginesound->IsSoundPrecached(params.soundname))
            ConColorMsg(LOG_COLOR_CYAN, "Deferred_EmitSoundByHandle: Sound %s:%s was not precached\n", pszSoundName, params.soundname);
#endif

        float st = pCall->ep.m_flSoundTime;
        if(!st && params.delay_msec)
            st = gpGlobals->curtime + (float)params.delay_msec / 1000.0f;

        enginesound->EmitSound(pCall->filter, pCall->iEntIndex, params.channel, params.soundname, params.volume, params.soundlevel, pCall->ep.m_nFlags, params.pitch, pCall->ep.m_nSpecialDSP, pCall->bHasOrigin ? &pCall->vOrigin : NULL, NULL, &pCall->ep.m_UtlVecSoundOrigin, true, st, pCall->ep.m_nSpeakerEntity);

        TraceEmitSound("Deferred_EmitSoundByHandle: '%s' emitted as '%s' (ent %i)\n", pszSoundName, params.soundname, pCall->iEntIndex);

        // Don't caption modulations to the sound
        if(!(pCall->ep.m_nFlags & (SND_CHANGE_PITCH | SND_CHANGE_VOL)))
            EmitCloseCaption(pCall->filter, pCall->iEntIndex, params, pCall->ep);

#if defined(_WIN32)
        // NVNT notify the haptics system of this sound
        HapticProcessSound(pszSoundName, pCall->iEntIndex);
#endif
    }

    void Deferred_EmitAmbientSound(DeferredCall_EmitAmbientSound_t *pCall)
    {
        const char *pszSoundName = pCall->soundname.Get();

        CSoundParameters params;
        if(!soundemitterbase->GetParametersForSound(pszSoundName, params, GENDER_NONE))
            return;

#if defined(STAGING_ONLY)
        const char *pszSndFilterString = sv_snd_filter.GetString();
        if(pszSndFilterString[0] && !Q_stristr(params.soundname, pszSndFilterString))
            return;
#endif

        // Handle SND_CHANGE_PITCH and SND_CHANGE_VOL
        if(pCall->iFlags & SND_CHANGE_PITCH)
            params.pitch = pCall->iPitch;
        if(pCall->iFlags & SND_CHANGE_VOL)
            params.volume = pCall->flVolume;

#if defined( CLIENT_DLL )
        enginesound->EmitAmbientSound(params.soundname, params.volume, params.pitch, pCall->iFlags, pCall->flSoundTime);
#else
        engine->EmitAmbientSound(pCall->iEntIndex, pCall->vOrigin, params.soundname, params.volume, params.soundlevel, pCall->iFlags, params.pitch, pCall->flSoundTime);
#endif

        TraceEmitSound("Deferred_EmitAmbientSound: '%s' emitted as '%s' (ent %i)\n", pszSoundName, params.soundname, pCall->iEntIndex);

        if(!(pCall->iFlags & (SND_STOP | SND_CHANGE_VOL | SND_CHANGE_PITCH))) {
            CRecipientFilter filter;
            filter.AddAllPlayers();
            filter.MakeReliable();

            CUtlVector<Vector> utlDummy;
            EmitCloseCaption(filter, pCall->iEntIndex, false, pszSoundName, utlDummy, enginesound->GetSoundDuration(params.soundname), false);
        }
    }

    void Deferred_EmitAmbientSound2(DeferredCall_EmitAmbientSound2_t *pCall)
    {
        const char *pszSample = pCall->sample.Get();

#if defined(STAGING_ONLY)
        const char *pszSndFilterString = sv_snd_filter.GetString();
        if(pszSndFilterString[0] && !Q_stristr(pszSample, pszSndFilterString))
            return;
#endif

#if defined(CLIENT_DLL)
        enginesound->EmitAmbientSound(pszSample, pCall->flVolume, pCall->iPitch, pCall->iFlags, pCall->flSoundTime);
#else
        engine->EmitAmbientSound(pCall->iEntIndex, pCall->vOrigin, pszSample, pCall->flVolume, pCall->soundlevel, pCall->iFlags, pCall->iPitch, pCall->flSoundTime);
#endif

        TraceEmitSound("Deferred_EmitAmbientSound2: Raw wave emitted '%s' (ent %i)\n", pszSample, pCall->iEntIndex);
    }

    void Deferred_StopSound(DeferredCall_StopSound_t *pCall)
    {
        const char *pszSoundName = pCall->soundname.Get();

        if(pCall->hSoundHandle == SOUNDEMITTER_INVALID_HANDLE)
            pCall->hSoundHandle = (HSOUNDSCRIPTHANDLE)soundemitterbase->GetSoundIndex(pszSoundName);
        if(pCall->hSoundHandle == SOUNDEMITTER_INVALID_HANDLE)
            return;

        CSoundParametersInternal *pParams = soundemitterbase->InternalGetParametersForSound((int)pCall->hSoundHandle);
        if(!pParams)
            return;

        int iNumSounds = pParams->NumSoundNames();
        for(int i = 0; i < iNumSounds; i++) {
            const char *pszWaveName = soundemitterbase->GetWaveName(pParams->GetSoundNames()[i].symbol);
            AssertMsg(pszWaveName, "Wave name is null!");
            enginesound->StopSound(pCall->iEntIndex, pParams->GetChannel(), pszWaveName);
            TraceEmitSound("Deferred_StopSound:  '%s' stopped as '%s' (ent %i)\n", pszSoundName, pszWaveName, pCall->iEntIndex);
        }
    }

    void Deferred_StopSound2(DeferredCall_StopSound2_t *pCall)
    {
        const char *pszSample = pCall->sample.Get();
        enginesound->StopSound(pCall->iEntIndex, pCall->iChannel, pszSample);
        TraceEmitSound("Deferred_StopSound2:  Raw wave stopped '%s' (ent %i)\n", pszSample, pCall->iEntIndex);
    }

    void EmitSoundByHandle(IRecipientFilter &filter, int entindex, const EmitSound_t &ep, HSOUNDSCRIPTHANDLE &handle)
    {
        gender_t gender = GENDER_NONE; // attack helicopter
        CBaseEntity *pEntity = CBaseEntity::Instance(entindex);
        if(pEntity)
            gender = soundemitterbase->GetActorGender(STRING(pEntity->GetModelName()));

        CSoundParameters params;
        if(!soundemitterbase->GetParametersForSoundEx(ep.m_pSoundName, handle, params, gender, true))
            return;

        if(!params.soundname[0])
            return;

        // Handle SND_CHANGE_PITCH and SND_CHANGE_VOL
        if(ep.m_nFlags & SND_CHANGE_PITCH)
            params.pitch = ep.m_nPitch;
        if(ep.m_nFlags & SND_CHANGE_VOL)
            params.volume = ep.m_flVolume;

#if !defined(CLIENT_DLL)
        if(CEnvMicrophone::OnSoundPlayed(entindex, params.soundname, params.soundlevel, params.volume, ep.m_nFlags, params.pitch, ep.m_pOrigin, ep.m_flSoundTime, ep.m_UtlVecSoundOrigin))
            return;
#endif

        float flDelay = 0.0f;
        bool bShouldDelay = false;
        if(!TestSoundChar(params.soundname, CHAR_DRYMIX)) {
            if(ep.m_pOrigin) {
                // Use origin pointer.
                bShouldDelay = LOS_CalcDelay(*ep.m_pOrigin, flDelay);
            }
            else if(ep.m_UtlVecSoundOrigin.Count()) {
                // Use origin list.
                bShouldDelay = LOS_CalcDelay(ep.m_UtlVecSoundOrigin.Tail(), flDelay);
            }
            else if(pEntity && entindex) {
                // Use entity's origin. Too bad!
                bShouldDelay = LOS_CalcDelay(pEntity->GetAbsOrigin(), flDelay);
            }
        }

        float flTriggerTime = 0.0f;
        if(bShouldDelay)
            flTriggerTime = gpGlobals->curtime + flDelay;
        m_workerThread.EmitSoundByHandle(flTriggerTime, entindex, ep, filter, handle);

        if(ep.m_pflSoundDuration)
            *ep.m_pflSoundDuration = enginesound->GetSoundDuration(ep.m_pSoundName);
    }

    void EmitSound(IRecipientFilter &filter, int entindex, const EmitSound_t &ep)
    {
#ifdef STAGING_ONLY
        if(sv_snd_filter.GetString()[0] && !V_stristr(ep.m_pSoundName, sv_snd_filter.GetString()))
            return;
#endif // STAGING_ONLY

        if(ep.m_pSoundName && (Q_stristr(ep.m_pSoundName, ".wav") || Q_stristr(ep.m_pSoundName, ".mp3") || ep.m_pSoundName[0] == '!')) {
#if !defined( CLIENT_DLL )
            if(CEnvMicrophone::OnSoundPlayed(entindex, ep.m_pSoundName, ep.m_SoundLevel, ep.m_flVolume, ep.m_nFlags, ep.m_nPitch, ep.m_pOrigin, ep.m_flSoundTime, ep.m_UtlVecSoundOrigin))
                return;
#endif

            float flDelay = 0.0f;
            bool bShouldDelay = false;
            if(!TestSoundChar(ep.m_pSoundName, CHAR_DRYMIX)) {
                if(ep.m_pOrigin) {
                    // Use origin pointer.
                    bShouldDelay = LOS_CalcDelay(*ep.m_pOrigin, flDelay);
                }
                else if(ep.m_UtlVecSoundOrigin.Count()) {
                    // Use origin list.
                    bShouldDelay = LOS_CalcDelay(ep.m_UtlVecSoundOrigin.Tail(), flDelay);
                }
            }

            float flTriggerTime = 0.0f;
            if(bShouldDelay)
                flTriggerTime = gpGlobals->curtime + flDelay;
            m_workerThread.EmitSound(flTriggerTime, filter, entindex, ep);

            if(ep.m_pflSoundDuration)
                *ep.m_pflSoundDuration = enginesound->GetSoundDuration(ep.m_pSoundName);

            return;
        }

        if(ep.m_hSoundScriptHandle == SOUNDEMITTER_INVALID_HANDLE)
            ep.m_hSoundScriptHandle = (HSOUNDSCRIPTHANDLE)soundemitterbase->GetSoundIndex(ep.m_pSoundName);

        if(ep.m_hSoundScriptHandle == -1)
            return;

        EmitSoundByHandle(filter, entindex, ep, ep.m_hSoundScriptHandle);
    }

    void EmitCloseCaption(IRecipientFilter &filter, int entindex, bool fromplayer, char const *token, CUtlVector< Vector > &originlist, float duration, bool warnifmissing /*= false*/)
    {
        // No close captions in multiplayer...
        if(gpGlobals->maxClients > 1 || (gpGlobals->maxClients == 1 && !g_pClosecaption->GetBool())) {
            return;
        }

        // A negative duration means fill it in from the wav file if possible
        if(duration < 0.0f) {
            char const *wav = soundemitterbase->GetWavFileForSound(token, GENDER_NONE);
            if(wav)
                duration = enginesound->GetSoundDuration(wav);
            else
                duration = 2.0f;
        }

        char lowercase[256];
        Q_strncpy(lowercase, token, sizeof(lowercase));
        Q_strlower(lowercase);
        if(Q_strstr(lowercase, "\\")) {
            Hack_FixEscapeChars(lowercase);
        }

        // NOTE:  We must make a copy or else if the filter is owned by a SoundPatch, we'll end up destructively removing
        //  all players from it!!!!
        CRecipientFilter filterCopy;
        filterCopy.CopyFrom((CRecipientFilter &)filter);

        // Remove any players who don't want close captions
        CBaseEntity::RemoveRecipientsIfNotCloseCaptioning((CRecipientFilter &)filterCopy);

#if !defined( CLIENT_DLL )
        {
            // Defined in sceneentity.cpp
            bool AttenuateCaption(const char *token, const Vector & listener, CUtlVector< Vector >&soundorigins);

            if(filterCopy.GetRecipientCount() > 0) {
                int c = filterCopy.GetRecipientCount();
                for(int i = c - 1; i >= 0; --i) {
                    CBasePlayer *player = UTIL_PlayerByIndex(filterCopy.GetRecipientIndex(i));
                    if(!player)
                        continue;

                    Vector playerOrigin = player->GetAbsOrigin();

                    if(AttenuateCaption(lowercase, playerOrigin, originlist)) {
                        filterCopy.RemoveRecipient(player);
                    }
                }
            }
        }
#endif
        // Anyone left?
        if(filterCopy.GetRecipientCount() > 0) {

#if !defined( CLIENT_DLL )

            byte byteflags = 0;
            if(warnifmissing)
                byteflags |= CLOSE_CAPTION_WARNIFMISSING;
            if(fromplayer)
                byteflags |= CLOSE_CAPTION_FROMPLAYER;

            CBaseEntity *pActor = CBaseEntity::Instance(entindex);
            if(pActor) {
                char const *pszActorModel = STRING(pActor->GetModelName());
                gender_t gender = soundemitterbase->GetActorGender(pszActorModel);
                if(gender == GENDER_MALE)
                    byteflags |= CLOSE_CAPTION_GENDER_MALE;
                else if(gender == GENDER_FEMALE)
                    byteflags |= CLOSE_CAPTION_GENDER_FEMALE;
            }

            // Send caption and duration hint down to client
            UserMessageBegin(filterCopy, "CloseCaption");
            WRITE_STRING(lowercase);
            WRITE_SHORT(MIN(255, (int)(duration * 10.0f)));
            WRITE_BYTE(byteflags);
            MessageEnd();
#else
            // Direct dispatch
            CHudCloseCaption *cchud = GET_HUDELEMENT(CHudCloseCaption);
            if(cchud)
                cchud->ProcessCaption(lowercase, duration, fromplayer);
#endif
        }
    }

    void EmitCloseCaption(IRecipientFilter &filter, int entindex, const CSoundParameters &params, const EmitSound_t &ep)
    {
        // No close captions in multiplayer...
        if(gpGlobals->maxClients > 1 || (gpGlobals->maxClients == 1 && !g_pClosecaption->GetBool())) {
            return;
        }

        if(!ep.m_bEmitCloseCaption) {
            return;
        }

        // NOTE:  We must make a copy or else if the filter is owned by a SoundPatch, we'll end up destructively removing
        //  all players from it!!!!
        CRecipientFilter filterCopy;
        filterCopy.CopyFrom((CRecipientFilter &)filter);

        // Remove any players who don't want close captions
        CBaseEntity::RemoveRecipientsIfNotCloseCaptioning((CRecipientFilter &)filterCopy);

        // Anyone left?
        if(filterCopy.GetRecipientCount() <= 0) {
            return;
        }

        float duration = 0.0f;
        if(ep.m_pflSoundDuration)
            duration = *ep.m_pflSoundDuration;
        else
            duration = enginesound->GetSoundDuration(params.soundname);

        bool fromplayer = false;
        CBaseEntity *ent = CBaseEntity::Instance(entindex);
        if(ent) {
            while(ent) {
                if(ent->IsPlayer()) {
                    fromplayer = true;
                    break;
                }

                ent = ent->GetOwnerEntity();
            }
        }

        EmitCloseCaption(filter, entindex, fromplayer, ep.m_pSoundName, ep.m_UtlVecSoundOrigin, duration, ep.m_bWarnOnMissingCloseCaption);
    }

    void EmitAmbientSound(int entindex, const Vector &origin, const char *soundname, float flVolume, int iFlags, int iPitch, float soundtime /*= 0.0f*/, float *duration /*=NULL*/)
    {
        float flDelay;
        CSoundParameters params;
        if((!soundemitterbase->GetParametersForSound(soundname, params, GENDER_NONE, false) || !params.soundname[0] || TestSoundChar(params.soundname, CHAR_DRYMIX)) || !LOS_CalcDelay(origin, flDelay))
            flDelay = 0.0f;

        m_workerThread.EmitAmbientSound(gpGlobals->curtime + flDelay, entindex, origin, soundname, flVolume, iFlags, iPitch, soundtime);

        if(duration)
            *duration = enginesound->GetSoundDuration(soundname);
    }

    void StopSoundByHandle(int entindex, const char *soundname, HSOUNDSCRIPTHANDLE &handle)
    {
        float flDelay;
        float flTriggerTime = 0.0f;
        CBaseEntity *pEntity = CBaseEntity::Instance(entindex);
        if(!TestSoundChar(soundname, CHAR_DRYMIX) && pEntity && LOS_CalcDelay(pEntity->GetAbsOrigin(), flDelay))
            flTriggerTime = gpGlobals->curtime + flDelay;
        m_workerThread.StopSound(flTriggerTime, entindex, soundname, handle);
    }

    void StopSound(int entindex, const char *soundname)
    {
        HSOUNDSCRIPTHANDLE handle = (HSOUNDSCRIPTHANDLE)soundemitterbase->GetSoundIndex(soundname);
        if(handle == SOUNDEMITTER_INVALID_HANDLE)
            return;
        StopSoundByHandle(entindex, soundname, handle);
    }

    void StopSound(int iEntIndex, int iChannel, const char *pSample)
    {
        if(pSample && (Q_stristr(pSample, ".wav") || Q_stristr(pSample, ".mp3") || pSample[0] == '!')) {
            float flDelay;
            float flTriggerTime = 0.0f;
            CBaseEntity *pEntity = CBaseEntity::Instance(iEntIndex);
            if(!TestSoundChar(pSample, CHAR_DRYMIX) && pEntity && LOS_CalcDelay(pEntity->GetAbsOrigin(), flDelay))
                flTriggerTime = gpGlobals->curtime + flDelay;
            m_workerThread.StopSound2(flTriggerTime, iEntIndex, iChannel, pSample);
            return;
        }

        // Look it up in sounds.txt and ignore other parameters
        StopSound(iEntIndex, pSample);
    }

    void EmitAmbientSound(int entindex, const Vector &origin, const char *pSample, float volume, soundlevel_t soundlevel, int flags, int pitch, float soundtime /*= 0.0f*/, float *duration /*=NULL*/)
    {
#if !defined( CLIENT_DLL )
        CUtlVector<Vector> utlDummy;
        if(CEnvMicrophone::OnSoundPlayed(entindex, pSample, soundlevel, volume, flags, pitch, &origin, soundtime, utlDummy))
            return;
#endif

        float flDelay;
        float flTriggerTime = 0.0f;
        if(!TestSoundChar(pSample, CHAR_DRYMIX) && LOS_CalcDelay(origin, flDelay))
            flTriggerTime = gpGlobals->curtime + flDelay;
        m_workerThread.EmitAmbientSound2(flTriggerTime, entindex, origin, pSample, volume, soundlevel, flags, pitch, soundtime);

        if(duration)
            *duration = enginesound->GetSoundDuration(pSample);
    }
};

static CSoundEmitterSystem g_SoundEmitterSystem("CSoundEmitterSystem");

IGameSystem *SoundEmitterSystem()
{
    return &g_SoundEmitterSystem;
}

#if defined( CLIENT_DLL )
void ReloadSoundEntriesInList(IFileList *pFilesToReload)
{
    g_SoundEmitterSystem.ReloadSoundEntriesInList(pFilesToReload);
}
#endif

void S_SoundEmitterSystemFlush(void)
{
#if !defined( CLIENT_DLL )
    if(!UTIL_IsCommandIssuedByServerAdmin())
        return;
#endif

    // save the current soundscape
    // kill the system
    g_SoundEmitterSystem.Shutdown();

    // restart the system
    g_SoundEmitterSystem.Init();

#if !defined( CLIENT_DLL )
    // Redo precache all wave files... (this should work now that we have dynamic string tables)
    g_SoundEmitterSystem.LevelInitPreEntity();

    // These store raw sound indices for faster precaching, blow them away.
    ClearModelSoundsCache();
#endif

    // TODO:  when we go to a handle system, we'll need to invalidate handles somehow
}

#if defined (CLIENT_DLL)
CON_COMMAND_F(cl_soundemitter_flush, "Flushes the sounds.txt system", FCVAR_CHEAT)
{
    S_SoundEmitterSystemFlush();
}
#endif

#if defined(_RETAIL)
void Playgamesound_f(const CCommand &args)
{
    CBasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
    if(pPlayer) {
        if(args.ArgC() > 2) {
            Vector position = pPlayer->EyePosition();
            Vector forward;
            pPlayer->GetVectors(&forward, NULL, NULL);
            position += atof(args[2]) * forward;
            CPASAttenuationFilter filter(pPlayer);
            EmitSound_t params;
            params.m_pSoundName = args[1];
            params.m_pOrigin = &position;
            params.m_flVolume = 0.0f;
            params.m_nPitch = 0;
            g_SoundEmitterSystem.EmitSound(filter, 0, params);
        }
        else {
            pPlayer->EmitSound(args[1]);
        }
    }
    else {
        ConColorMsg(LOG_COLOR_CYAN, "Can't play until a game is started.\n");
        // UNDONE: Make something like this work?
        //CBroadcastRecipientFilter filter;
        //g_SoundEmitterSystem.EmitSound( filter, 1, args[1], 0.0, 0, 0, &vec3_origin, 0, NULL );
    }
}

static int GamesoundCompletion(const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH])
{
    int current = 0;

    const char *cmdname = "playgamesound";
    char *substring = NULL;
    int substringLen = 0;
    if(Q_strstr(partial, cmdname) && strlen(partial) > strlen(cmdname) + 1) {
        substring = (char *)partial + strlen(cmdname) + 1;
        substringLen = strlen(substring);
    }

    for(int i = soundemitterbase->GetSoundCount() - 1; i >= 0 && current < COMMAND_COMPLETION_MAXITEMS; i--) {
        const char *pSoundName = soundemitterbase->GetSoundName(i);
        if(pSoundName) {
            if(!substring || !Q_strncasecmp(pSoundName, substring, substringLen)) {
                Q_snprintf(commands[current], sizeof(commands[current]), "%s %s", cmdname, pSoundName);
                current++;
            }
        }
    }

    return current;
}

static ConCommand Command_Playgamesound("playgamesound", Playgamesound_f, "Play a sound from the game sounds txt file", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_SERVER_CAN_EXECUTE, GamesoundCompletion);
#endif

//-----------------------------------------------------------------------------
// Purpose:  Non-static override for doing the general case of CPASAttenuationFilter( this ), and EmitSound( filter, entindex(), etc. );
// Input  : *soundname -
//-----------------------------------------------------------------------------
void CBaseEntity::EmitSound(const char *soundname, float soundtime /*= 0.0f*/, float *duration /*=NULL*/)
{
    VPROF_BUDGET("CBaseEntity::EmitSound", _T("CBaseEntity::EmitSound"));

    CPASAttenuationFilter filter(this, soundname);

    EmitSound_t params;
    params.m_pSoundName = soundname;
    params.m_flSoundTime = soundtime;
    params.m_pflSoundDuration = duration;
    params.m_bWarnOnDirectWaveReference = true;

    EmitSound(filter, entindex(), params);
}

//-----------------------------------------------------------------------------
// Purpose:  Non-static override for doing the general case of CPASAttenuationFilter( this ), and EmitSound( filter, entindex(), etc. );
// Input  : *soundname -
//-----------------------------------------------------------------------------
void CBaseEntity::EmitSound(const char *soundname, HSOUNDSCRIPTHANDLE &handle, float soundtime /*= 0.0f*/, float *duration /*=NULL*/)
{
    VPROF_BUDGET("CBaseEntity::EmitSound", _T("CBaseEntity::EmitSound"));

    CPASAttenuationFilter filter(this, soundname, handle);

    EmitSound_t params;
    params.m_pSoundName = soundname;
    params.m_flSoundTime = soundtime;
    params.m_pflSoundDuration = duration;
    params.m_bWarnOnDirectWaveReference = true;

    EmitSound(filter, entindex(), params, handle);
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  : filter -
//          iEntIndex -
//          *soundname -
//          *pOrigin -
//-----------------------------------------------------------------------------
void CBaseEntity::EmitSound(IRecipientFilter &filter, int iEntIndex, const char *soundname, const Vector *pOrigin /*= NULL*/, float soundtime /*= 0.0f*/, float *duration /*=NULL*/)
{
    if(!soundname)
        return;

    VPROF_BUDGET("CBaseEntity::EmitSound", _T("CBaseEntity::EmitSound"));

    EmitSound_t params;
    params.m_pSoundName = soundname;
    params.m_flSoundTime = soundtime;
    params.m_pOrigin = pOrigin;
    params.m_pflSoundDuration = duration;
    params.m_bWarnOnDirectWaveReference = true;

    EmitSound(filter, iEntIndex, params, params.m_hSoundScriptHandle);
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  : filter -
//          iEntIndex -
//          *soundname -
//          *pOrigin -
//-----------------------------------------------------------------------------
void CBaseEntity::EmitSound(IRecipientFilter &filter, int iEntIndex, const char *soundname, HSOUNDSCRIPTHANDLE &handle, const Vector *pOrigin /*= NULL*/, float soundtime /*= 0.0f*/, float *duration /*=NULL*/)
{
    VPROF_BUDGET("CBaseEntity::EmitSound", _T("CBaseEntity::EmitSound"));

    EmitSound_t params;
    params.m_pSoundName = soundname;
    params.m_flSoundTime = soundtime;
    params.m_pOrigin = pOrigin;
    params.m_pflSoundDuration = duration;
    params.m_bWarnOnDirectWaveReference = true;

    EmitSound(filter, iEntIndex, params, handle);
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  : filter -
//          iEntIndex -
//          params -
//-----------------------------------------------------------------------------
void CBaseEntity::EmitSound(IRecipientFilter &filter, int iEntIndex, const EmitSound_t &params)
{
    VPROF_BUDGET("CBaseEntity::EmitSound", _T("CBaseEntity::EmitSound"));

#ifdef GAME_DLL
    CBaseEntity *pEntity = UTIL_EntityByIndex(iEntIndex);
#else
    C_BaseEntity *pEntity = ClientEntityList().GetEnt(iEntIndex);
#endif

    if(pEntity) {
        pEntity->ModifyEmitSoundParams(const_cast<EmitSound_t &>(params));
    }

    // Call into the sound emitter system...
    g_SoundEmitterSystem.EmitSound(filter, iEntIndex, params);
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  : filter -
//          iEntIndex -
//          params -
//-----------------------------------------------------------------------------
void CBaseEntity::EmitSound(IRecipientFilter &filter, int iEntIndex, const EmitSound_t &params, HSOUNDSCRIPTHANDLE &handle)
{
    VPROF_BUDGET("CBaseEntity::EmitSound", _T("CBaseEntity::EmitSound"));

#ifdef GAME_DLL
    CBaseEntity *pEntity = UTIL_EntityByIndex(iEntIndex);
#else
    C_BaseEntity *pEntity = ClientEntityList().GetEnt(iEntIndex);
#endif

    if(pEntity) {
        pEntity->ModifyEmitSoundParams(const_cast<EmitSound_t &>(params));
    }

    // Call into the sound emitter system...
    g_SoundEmitterSystem.EmitSoundByHandle(filter, iEntIndex, params, handle);
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  : *soundname -
//-----------------------------------------------------------------------------
void CBaseEntity::StopSound(const char *soundname)
{
#if defined( CLIENT_DLL )
    if(entindex() == -1) {
        // If we're a clientside entity, we need to use the soundsourceindex instead of the entindex
        StopSound(GetSoundSourceIndex(), soundname);
        return;
    }
#endif

    StopSound(entindex(), soundname);
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  : *soundname -
//-----------------------------------------------------------------------------
void CBaseEntity::StopSound(const char *soundname, HSOUNDSCRIPTHANDLE &handle)
{
#if defined( CLIENT_DLL )
    if(entindex() == -1) {
        // If we're a clientside entity, we need to use the soundsourceindex instead of the entindex
        StopSound(GetSoundSourceIndex(), soundname);
        return;
    }
#endif

    g_SoundEmitterSystem.StopSoundByHandle(entindex(), soundname, handle);
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  : iEntIndex -
//          *soundname -
//-----------------------------------------------------------------------------
void CBaseEntity::StopSound(int iEntIndex, const char *soundname)
{
    g_SoundEmitterSystem.StopSound(iEntIndex, soundname);
}

void CBaseEntity::StopSound(int iEntIndex, int iChannel, const char *pSample)
{
    g_SoundEmitterSystem.StopSound(iEntIndex, iChannel, pSample);
}

soundlevel_t CBaseEntity::LookupSoundLevel(const char *soundname)
{
    return soundemitterbase->LookupSoundLevel(soundname);
}


soundlevel_t CBaseEntity::LookupSoundLevel(const char *soundname, HSOUNDSCRIPTHANDLE &handle)
{
    return soundemitterbase->LookupSoundLevelByHandle(soundname, handle);
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  : *entity -
//          origin -
//          flags -
//          *soundname -
//-----------------------------------------------------------------------------
void CBaseEntity::EmitAmbientSound(int entindex, const Vector &origin, const char *soundname, int flags, float soundtime /*= 0.0f*/, float *duration /*=NULL*/)
{
    g_SoundEmitterSystem.EmitAmbientSound(entindex, origin, soundname, 0.0, flags, 0, soundtime, duration);
}

// HACK HACK:  Do we need to pull the entire SENTENCEG_* wrapper over to the client .dll?
#if defined( CLIENT_DLL )
int SENTENCEG_Lookup(const char *sample)
{
    return engine->SentenceIndexFromName(sample + 1);
}
#endif

void UTIL_EmitAmbientSound(int entindex, const Vector &vecOrigin, const char *samp, float vol, soundlevel_t soundlevel, int fFlags, int pitch, float soundtime /*= 0.0f*/, float *duration /*=NULL*/)
{
#ifdef STAGING_ONLY
    if(sv_snd_filter.GetString()[0] && !V_stristr(samp, sv_snd_filter.GetString())) {
        return;
    }
#endif // STAGING_ONLY

    if(samp && *samp == '!') {
        int sentenceIndex = SENTENCEG_Lookup(samp);
        if(sentenceIndex >= 0) {
            char name[32];
            Q_snprintf(name, sizeof(name), "!%d", sentenceIndex);

#if !defined( CLIENT_DLL )
            engine->EmitAmbientSound(entindex, vecOrigin, name, vol, soundlevel, fFlags, pitch, soundtime);
#else
            enginesound->EmitAmbientSound(name, vol, pitch, fFlags, soundtime);
#endif

            if(duration)
                *duration = enginesound->GetSoundDuration(name);

            g_SoundEmitterSystem.TraceEmitSound("UTIL_EmitAmbientSound:  Sentence emitted '%s' (ent %i)\n", name, entindex);
        }
    }
    else {
        g_SoundEmitterSystem.EmitAmbientSound(entindex, vecOrigin, samp, vol, soundlevel, fFlags, pitch, soundtime, duration);
    }
}

static const char *UTIL_TranslateSoundName(const char *soundname, const char *actormodel)
{
    Assert(soundname);

    if(Q_stristr(soundname, ".wav") || Q_stristr(soundname, ".mp3")) {
        if(Q_stristr(soundname, ".wav")) {
            WaveTrace(soundname, "UTIL_TranslateSoundName");
        }
        return soundname;
    }

    return soundemitterbase->GetWavFileForSound(soundname, actormodel);
}

void CBaseEntity::GenderExpandString(char const *in, char *out, int maxlen)
{
    soundemitterbase->GenderExpandString(STRING(GetModelName()), in, out, maxlen);
}

bool CBaseEntity::GetParametersForSound(const char *soundname, CSoundParameters &params, const char *actormodel)
{
    gender_t gender = soundemitterbase->GetActorGender(actormodel);
    return soundemitterbase->GetParametersForSound(soundname, params, gender);
}

bool CBaseEntity::GetParametersForSound(const char *soundname, HSOUNDSCRIPTHANDLE &handle, CSoundParameters &params, const char *actormodel)
{
    gender_t gender = soundemitterbase->GetActorGender(actormodel);
    return soundemitterbase->GetParametersForSoundEx(soundname, handle, params, gender);
}

HSOUNDSCRIPTHANDLE CBaseEntity::PrecacheScriptSound(const char *soundname)
{
#if !defined( CLIENT_DLL )
    return g_SoundEmitterSystem.PrecacheScriptSound(soundname);
#else
    return soundemitterbase->GetSoundIndex(soundname);
#endif
}

void CBaseEntity::PrefetchScriptSound(const char *soundname)
{
    g_SoundEmitterSystem.PrefetchScriptSound(soundname);
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  : *soundname -
// Output : float
//-----------------------------------------------------------------------------
float CBaseEntity::GetSoundDuration(const char *soundname, char const *actormodel)
{
    return enginesound->GetSoundDuration(PSkipSoundChars(UTIL_TranslateSoundName(soundname, actormodel)));
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  : filter -
//          *token -
//          duration -
//          warnifmissing -
//-----------------------------------------------------------------------------
void CBaseEntity::EmitCloseCaption(IRecipientFilter &filter, int entindex, char const *token, CUtlVector< Vector > &soundorigin, float duration, bool warnifmissing /*= false*/)
{
    bool fromplayer = false;
    CBaseEntity *ent = CBaseEntity::Instance(entindex);
    while(ent) {
        if(ent->IsPlayer()) {
            fromplayer = true;
            break;
        }
        ent = ent->GetOwnerEntity();
    }

    g_SoundEmitterSystem.EmitCloseCaption(filter, entindex, fromplayer, token, soundorigin, duration, warnifmissing);
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  : *name -
//          preload -
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CBaseEntity::PrecacheSound(const char *name)
{
    if(IsPC() && !g_bPermitDirectSoundPrecache) {
        Warning("Direct precache of %s\n", name);
    }

    // If this is out of order, warn
    if(!CBaseEntity::IsPrecacheAllowed()) {
        if(!enginesound->IsSoundPrecached(name)) {
            Assert(!"CBaseEntity::PrecacheSound:  too late");

            Warning("Late precache of %s\n", name);
        }
    }

    bool bret = enginesound->PrecacheSound(name, true);
    return bret;
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  : *name -
//-----------------------------------------------------------------------------
void CBaseEntity::PrefetchSound(const char *name)
{
    enginesound->PrefetchSound(name);
}

CDeferredSoundWorkerThread::CDeferredSoundWorkerThread() : m_utlEmitSound(), m_utlEmitSoundByHandle(), m_utlEmitAmbientSound(), m_utlEmitAmbientSound2()
{

}

int CDeferredSoundWorkerThread::Run()
{
    unsigned uCall;
    while(IsAlive()) {
        if(PeekCall(&uCall) && uCall == SND_WORKER_EXIT) {
            Reply(SND_WORKER_EXIT);
            break;
        }

        AUTO_LOCK_FM(m_utlEmitSound);
        for(int i = 0; i < m_utlEmitSound.Count(); i++) {
            DeferredCall_EmitSound_t *pCall = m_utlEmitSound.Element(i);
            if(gpGlobals->curtime >= pCall->flTriggerTime) {
                g_SoundEmitterSystem.Deferred_EmitSound(pCall);
                m_utlEmitSound.Remove(i);
                delete pCall;
            }
        }

        AUTO_LOCK_FM(m_utlEmitSoundByHandle);
        for(int i = 0; i < m_utlEmitSoundByHandle.Count(); i++) {
            DeferredCall_EmitSoundByHandle_t *pCall = m_utlEmitSoundByHandle[i];
            if(gpGlobals->curtime >= pCall->flTriggerTime) {
                g_SoundEmitterSystem.Deferred_EmitSoundByHandle(pCall);
                m_utlEmitSoundByHandle.Remove(i);
                delete pCall;
            }
        }

        AUTO_LOCK_FM(m_utlEmitAmbientSound);
        for(int i = 0; i < m_utlEmitAmbientSound.Count(); i++) {
            DeferredCall_EmitAmbientSound_t *pCall = m_utlEmitAmbientSound[i];
            if(gpGlobals->curtime >= pCall->flTriggerTime) {
                g_SoundEmitterSystem.Deferred_EmitAmbientSound(pCall);
                m_utlEmitAmbientSound.Remove(i);
                delete pCall;
            }
        }

        AUTO_LOCK_FM(m_utlEmitAmbientSound2);
        for(int i = 0; i < m_utlEmitAmbientSound2.Count(); i++) {
            DeferredCall_EmitAmbientSound2_t *pCall = m_utlEmitAmbientSound2[i];
            if(gpGlobals->curtime >= pCall->flTriggerTime) {
                g_SoundEmitterSystem.Deferred_EmitAmbientSound2(pCall);
                m_utlEmitAmbientSound2.Remove(i);
                delete pCall;
            }
        }

        AUTO_LOCK_FM(m_utlStopSound);
        for(int i = 0; i < m_utlStopSound.Count(); i++) {
            DeferredCall_StopSound_t *pCall = m_utlStopSound[i];
            if(gpGlobals->curtime >= pCall->flTriggerTime) {
                g_SoundEmitterSystem.Deferred_StopSound(pCall);
                m_utlStopSound.Remove(i);
                delete pCall;
            }
        }

        AUTO_LOCK_FM(m_utlStopSound2);
        for(int i = 0; i < m_utlStopSound2.Count(); i++) {
            DeferredCall_StopSound2_t *pCall = m_utlStopSound2[i];
            if(gpGlobals->curtime >= pCall->flTriggerTime) {
                g_SoundEmitterSystem.Deferred_StopSound2(pCall);
                m_utlStopSound2.Remove(i);
                delete pCall;
            }
        }
    }

    return 0;
}

bool CDeferredSoundWorkerThread::Init()
{
    return true;
}

void CDeferredSoundWorkerThread::OnExit()
{

}

void CDeferredSoundWorkerThread::Cleanup()
{
    int iCallsRemaining = 0;
    iCallsRemaining += m_utlEmitSound.Count();
    iCallsRemaining += m_utlEmitSoundByHandle.Count();
    iCallsRemaining += m_utlEmitAmbientSound.Count();
    iCallsRemaining += m_utlEmitAmbientSound2.Count();
    iCallsRemaining += m_utlStopSound.Count();

    if(iCallsRemaining) {
        ConColorMsg(LOG_COLOR_CYAN, "CDeferredSoundWorkerThread: Purging %d uninvoked calls\n", iCallsRemaining);

        AUTO_LOCK_FM(m_utlEmitSound);
        for(int i = 0; i < m_utlEmitSound.Count(); i++)
            delete m_utlEmitSound[i];
        m_utlEmitSound.RemoveAll();

        AUTO_LOCK_FM(m_utlEmitSoundByHandle);
        for(int i = 0; i < m_utlEmitSoundByHandle.Count(); i++)
            delete m_utlEmitSoundByHandle[i];
        m_utlEmitSoundByHandle.RemoveAll();

        AUTO_LOCK_FM(m_utlEmitAmbientSound);
        for(int i = 0; i < m_utlEmitAmbientSound.Count(); i++)
            delete m_utlEmitAmbientSound[i];
        m_utlEmitAmbientSound.RemoveAll();

        AUTO_LOCK_FM(m_utlEmitAmbientSound2);
        for(int i = 0; i < m_utlEmitAmbientSound2.Count(); i++)
            delete m_utlEmitAmbientSound2[i];
        m_utlEmitAmbientSound2.RemoveAll();

        AUTO_LOCK_FM(m_utlStopSound);
        for(int i = 0; i < m_utlStopSound.Count(); i++)
            delete m_utlStopSound[i];
        m_utlStopSound.RemoveAll();

        AUTO_LOCK_FM(m_utlStopSound2);
        for(int i = 0; i < m_utlStopSound2.Count(); i++)
            delete m_utlStopSound2[i];
        m_utlStopSound2.RemoveAll();
    }
}

void CDeferredSoundWorkerThread::EmitSound(float flTriggerTime, IRecipientFilter &filter, int iEntIndex, const EmitSound_t &ep)
{
    DeferredCall_EmitSound_t *pCall = new DeferredCall_EmitSound_t;
    pCall->flTriggerTime = flTriggerTime;
    pCall->filter.CopyFrom((CRecipientFilter &)filter);
    pCall->iEntIndex = iEntIndex;
    pCall->ep = ep;
    if(ep.m_pOrigin)
        pCall->vOrigin.Init(ep.m_pOrigin->x, ep.m_pOrigin->y, ep.m_pOrigin->z);
    pCall->soundname = ep.m_pSoundName;

    if(flTriggerTime) {
        AUTO_LOCK_FM(m_utlEmitSound);
        m_utlEmitSound.AddToTail(pCall);
    }
    else {
        g_SoundEmitterSystem.Deferred_EmitSound(pCall);
        delete pCall;
    }
}

void CDeferredSoundWorkerThread::EmitSoundByHandle(float flTriggerTime, int iEntIndex, const EmitSound_t &ep, IRecipientFilter &filter, HSOUNDSCRIPTHANDLE hSoundHandle)
{
    DeferredCall_EmitSoundByHandle_t *pCall = new DeferredCall_EmitSoundByHandle_t;
    pCall->flTriggerTime = flTriggerTime;
    pCall->iEntIndex = iEntIndex;
    pCall->ep = ep;
    if(ep.m_pOrigin)
        pCall->vOrigin.Init(ep.m_pOrigin->x, ep.m_pOrigin->y, ep.m_pOrigin->z);
    pCall->soundname = ep.m_pSoundName;
    pCall->filter.CopyFrom((CRecipientFilter &)filter);
    pCall->hSoundHandle = hSoundHandle;

    if(flTriggerTime) {
        AUTO_LOCK_FM(m_utlEmitSoundByHandle);
        m_utlEmitSoundByHandle.AddToTail(pCall);
    }
    else {
        g_SoundEmitterSystem.Deferred_EmitSoundByHandle(pCall);
        delete pCall;
    }
}

void CDeferredSoundWorkerThread::EmitAmbientSound(float flTriggerTime, int iEntIndex, const Vector &vOrigin, const char *pszSoundName, float flVolume, int iFlags, int iPitch, float flSoundTime)
{
    DeferredCall_EmitAmbientSound_t *pCall = new DeferredCall_EmitAmbientSound_t;
    pCall->flTriggerTime = flTriggerTime;
    pCall->iEntIndex = iEntIndex;
    pCall->vOrigin = vOrigin;
    pCall->soundname = pszSoundName;
    pCall->flVolume = flVolume;
    pCall->iFlags = iFlags;
    pCall->iPitch = iPitch;
    pCall->flSoundTime = flSoundTime;

    if(flTriggerTime) {
        AUTO_LOCK_FM(m_utlEmitAmbientSound);
        m_utlEmitAmbientSound.AddToTail(pCall);
    }
    else {
        g_SoundEmitterSystem.Deferred_EmitAmbientSound(pCall);
        delete pCall;
    }
}

void CDeferredSoundWorkerThread::EmitAmbientSound2(float flTriggerTime, int iEntIndex, const Vector &vOrigin, const char *pszSample, float flVolume, soundlevel_t soundlevel, int iFlags, int iPitch, float flSoundTime)
{
    DeferredCall_EmitAmbientSound2_t *pCall = new DeferredCall_EmitAmbientSound2_t;
    pCall->flTriggerTime = flTriggerTime;
    pCall->iEntIndex = iEntIndex;
    pCall->vOrigin = vOrigin;
    pCall->sample = pszSample;
    pCall->flVolume = flVolume;
    pCall->soundlevel = soundlevel;
    pCall->iFlags = iFlags;
    pCall->iPitch = iPitch;
    pCall->flSoundTime = flSoundTime;

    if(flTriggerTime) {
        AUTO_LOCK_FM(m_utlEmitAmbientSound2);
        m_utlEmitAmbientSound2.AddToTail(pCall);
    }
    else {
        g_SoundEmitterSystem.Deferred_EmitAmbientSound2(pCall);
        delete pCall;
    }
}

void CDeferredSoundWorkerThread::StopSound(float flTriggerTime, int iEntIndex, const char *pszSoundName, HSOUNDSCRIPTHANDLE hSoundHandle)
{
    DeferredCall_StopSound_t *pCall = new DeferredCall_StopSound_t;
    pCall->flTriggerTime = flTriggerTime;
    pCall->iEntIndex = iEntIndex;
    pCall->soundname = pszSoundName;
    pCall->hSoundHandle = hSoundHandle;

    if(flTriggerTime) {
        AUTO_LOCK_FM(m_utlStopSound);
        m_utlStopSound.AddToTail(pCall);
    }
    else {
        g_SoundEmitterSystem.Deferred_StopSound(pCall);
        delete pCall;
    }
}

void CDeferredSoundWorkerThread::StopSound2(float flTriggerTime, int iEntIndex, int iChannel, const char *pszSample)
{
    DeferredCall_StopSound2_t *pCall = new DeferredCall_StopSound2_t;
    pCall->flTriggerTime = flTriggerTime;
    pCall->iEntIndex = iEntIndex;
    pCall->iChannel = iChannel;
    pCall->sample = pszSample;

    if(flTriggerTime) {
        AUTO_LOCK_FM(m_utlStopSound2);
        m_utlStopSound2.AddToTail(pCall);
    }
    else {
        g_SoundEmitterSystem.Deferred_StopSound2(pCall);
        delete pCall;
    }
}

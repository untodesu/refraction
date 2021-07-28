#include "cbase.h"
#include "AL/efx.h"
#include "AL/salad.h"
#include "openal/al_sound.h"
#include "openal/al_sound_stream.h"
#include "openal/al_system.h"
#include "openal/al_utils.h"
#include "tier0/icommandline.h"
#include "tier1/interface.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CALWorkerThread : public CWorkerThread {
public:
    CALWorkerThread();
    virtual ~CALWorkerThread();

    int Run();
    bool Init();
    void OnExit();
};

typedef void( AL_APIENTRY *PFN_alSpeedOfSound )( ALfloat speed );

static CALSystem s_OpenAL;
static CALWorkerThread s_ALWorkerThread;
static PFN_alSpeedOfSound alSpeedOfSound = NULL;
static ConVarRef s_cvarVolume( "volume", false );
static CSysModule *s_pOpenALDll = NULL;

CALSystem *g_pOpenAL = &s_OpenAL;

static void * SALAD_APIENTRY Salad_LoadFunc( const char *pszProcName, void *pArg )
{
    return Sys_GetProcAddress( (CSysModule *)pArg, pszProcName );
}

CON_COMMAND( al_playstream, "Plays a stream" )
{
    if( args.ArgC() >= 2 ) {
        const char *pszPath = args.ArgV()[1];
        CALSourcePool *pPool = g_pOpenAL->GetGlobalPool();
        CALSoundStream *pStream = pPool->GetFreeSource<CALSoundStream>();
        pStream->EnablePositional( false );
        pStream->SetFilename( pszPath );
        pStream->SetLooping( false );
        if( pStream->Play() )
            return;
    }

    Msg( "Unable to play or fetch the sound name!" );
}

CALSystem::CALSystem() : m_pContext( NULL ), m_pDevice( NULL ), m_bEffectsAvailable( false ), m_vpBuffers(), m_vpPools(), m_pPool( NULL )
{

}

// FIXME_MAPBASE: POSIX support?
#define OPENAL_LIBNAME "OpenAL32"

bool CALSystem::Init()
{
    char szOpenALDllName[1024] = { 0 };
    
    // NOTELICENSE: LGPL REQUIRES software to provide a way
    // to replace the distributed binary with anything else.
    do {
        if( CommandLine()->FindParm( "-openalpath" ) ) {
            const char *pszOpenALDllPathReplace = CommandLine()->ParmValue( "-openalpath" );
            if( pszOpenALDllPathReplace ) {
                ConColorMsg( LOG_COLOR_CYAN, "CALSystem::Init: using alternative OpenAL path: %s\n", pszOpenALDllPathReplace );
                Q_strncpy( szOpenALDllName, pszOpenALDllPathReplace, sizeof( szOpenALDllName ) );
                break;
            }
        }

        Q_snprintf( szOpenALDllName, sizeof( szOpenALDllName ), "%s/bin/%s", engine->GetGameDirectory(), OPENAL_LIBNAME );
        ConColorMsg( LOG_COLOR_CYAN, "CALSystem::Init: using default OpenAL path: %s\n", szOpenALDllName );
    } while( false );

    s_pOpenALDll = Sys_LoadModule( szOpenALDllName );
    if( !s_pOpenALDll ) {
        Error( "CALSystem::Init: unable to load OpenAL library\n" );
        return false;
    }

    if( !saladLoadALFunc( Salad_LoadFunc, s_pOpenALDll ) ) {
        Error( "CALSystem::Init: unable to load OpenAL functions\n" );
        return false;
    }

    // FIXME: loader really should fetch this in future.
    alSpeedOfSound = (PFN_alSpeedOfSound)alGetProcAddress( "alSpeedOfSound" );
    if( !alSpeedOfSound ) {
        Error( "CALSystem::Init: unable to load alSpeedOfSound()\n" );
        return false;
    }

    m_pDevice = alcOpenDevice( NULL );
    if( !m_pDevice ) {
        Error( "CALSystem::Init: unable to open an OpenAL device\n" );
        return false;
    }

    m_bEffectsAvailable = alcIsExtensionPresent( m_pDevice, ALC_EXT_EFX_NAME );

    m_pContext = alcCreateContext( m_pDevice, NULL );
    if( !m_pContext ) {
        Error( "CALSystem::Init: unable to create an OpenAL context\n" );
        return false;
    }

    alcMakeContextCurrent( m_pContext );
    
    float flGain = 0.0f;
    alListenerfv( AL_GAIN, &flGain );

    alSpeedOfSound( 343.0f / METERS_PER_INCH );

    m_pPool = AllocPool();

    Update( -1.0f );

    if( !s_ALWorkerThread.IsAlive() )
        s_ALWorkerThread.Start();

    if( m_bEffectsAvailable ) {
        ALint iNumSends;
        alcGetIntegerv( m_pDevice, ALC_MAX_AUXILIARY_SENDS, 1, &iNumSends );
        ConColorMsg( LOG_COLOR_CYAN, "CALSystem::Init: EFX is available with %d AS/source\n", (int)iNumSends );
    }

    ConColorMsg( LOG_COLOR_CYAN, "CALSystem::Init: AL_RENDERER: %s\n", alGetString( AL_RENDERER ) );
    ConColorMsg( LOG_COLOR_CYAN, "CALSystem::Init: AL_VENDOR:   %s\n", alGetString( AL_VENDOR ) );
    ConColorMsg( LOG_COLOR_CYAN, "CALSystem::Init: AL_VERSION:  %s\n", alGetString( AL_VERSION ) );

    return true;
}

void CALSystem::Shutdown()
{
    if( s_ALWorkerThread.IsAlive() )
        s_ALWorkerThread.CallWorker( 42 );

    AUTO_LOCK_FM( m_vpPools );
    AUTO_LOCK_FM( m_vpBuffers );

    for( int i = 0; i < m_vpPools.Count(); i++ )
        delete m_vpPools[i];
    m_vpPools.RemoveAll();

    for( int i = 0; i < m_vpBuffers.Count(); i++ )
        delete m_vpBuffers[i];
    m_vpBuffers.RemoveAll();

    if( m_pDevice ) {
        if( m_pContext ) {
            alcMakeContextCurrent( NULL );
            alcDestroyContext( m_pContext );
            m_pContext = NULL;
        }

        alcCloseDevice( m_pDevice );
        m_pDevice = NULL;
    }

    Sys_UnloadModule( s_pOpenALDll );
    s_pOpenALDll = NULL;
}

void CALSystem::Update( float frametime )
{
    ALfloat aflPosition[3], aflOrientation[6];

    C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
    if( pPlayer ) {
        Vector vPosition = pPlayer->EarPosition();
        aflPosition[0] = vPosition.x;
        aflPosition[1] = vPosition.y;
        aflPosition[2] = vPosition.z;

        Vector vForward, vUp;
        AngleVectors( pPlayer->EyeAngles(), &vForward, NULL, &vUp );
        aflOrientation[0] = vForward.x;
        aflOrientation[1] = vForward.y;
        aflOrientation[2] = vForward.z;
        aflOrientation[3] = vUp.x;
        aflOrientation[4] = vUp.y;
        aflOrientation[5] = vUp.z;
    }
    else {
        aflPosition[0] = 0.0f;
        aflPosition[1] = 0.0f;
        aflPosition[2] = 0.0f;

        aflOrientation[0] = 0.0f;
        aflOrientation[1] = 0.0f;
        aflOrientation[2] = 0.0f;
        aflOrientation[3] = 0.0f;
        aflOrientation[4] = 0.0f;
        aflOrientation[5] = 0.0f;
    }

    float flGain = 0.0f;
    if( s_cvarVolume.IsValid() )
        flGain = s_cvarVolume.GetFloat();

    alListenerfv( AL_POSITION, aflPosition );
    alListenerfv( AL_ORIENTATION, aflOrientation );
    alListenerfv( AL_GAIN, &flGain );
}

void CALSystem::ThreadUpdate()
{
    AUTO_LOCK_FM( m_vpPools );
    for( int i = 0; i < m_vpPools.Count(); i++ ) {
        CALSourcePool *pPool = m_vpPools[i];
        if( pPool ) {
            pPool->Update();
            continue;
        }

        m_vpPools.Remove( i );
    }
}

const char *CALSystem::Name()
{
    return "OpenAL";
}

CALBuffer *CALSystem::LoadBuffer( const char *pszSoundName )
{
    CALBuffer *pBuffer = FindBuffer( pszSoundName );
    if( pBuffer )
        return pBuffer;

    CBaseALDecoder *pDecoder = AL_OpenFile( pszSoundName );
    if( !pDecoder )
        return NULL;

    pBuffer = new CALBuffer();
    pBuffer->Read( pDecoder );

    m_vpBuffers.AddToTail( pBuffer );

    return pBuffer;
}

CALBuffer *CALSystem::FindBuffer( const char *pszSoundName )
{
    for( int i = 0; i < m_vpBuffers.Count(); i++ ) {
        if( !Q_strcmp( pszSoundName, m_vpBuffers[i]->GetFilename() ) )
            return m_vpBuffers[i];
    }

    return NULL;
}

CALSourcePool *CALSystem::AllocPool()
{
    CALSourcePool *pPool = new CALSourcePool();
    m_vpPools.AddToTail( pPool );
    return pPool;
}

void CALSystem::FreePool( CALSourcePool *pPool )
{
    int iIndex = m_vpPools.Find( pPool );
    if( !m_vpPools.IsValidIndex( iIndex ) )
        return;
    delete m_vpPools[iIndex];
    m_vpPools.Remove( iIndex );
}

CALSourcePool *CALSystem::GetGlobalPool()
{
    return m_pPool;
}

CALWorkerThread::CALWorkerThread()
{

}

CALWorkerThread::~CALWorkerThread()
{

}

bool CALWorkerThread::Init()
{
    return true;
}

void CALWorkerThread::OnExit()
{

}

int CALWorkerThread::Run()
{
    unsigned uCall;
    while( IsAlive() ) {
        if( PeekCall( &uCall ) && uCall == 42 ) {
            Reply( 1 );
            break;
        }

        s_OpenAL.ThreadUpdate();
    }

    return 0;
}

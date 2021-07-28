#ifndef AL_SYSTEM_H
#define AL_SYSTEM_H 1
#include "AL/al.h"
#include "AL/alc.h"
#include "igamesystem.h"
#include "vphysics_interface.h"
#include "openal/al_buffer.h"
#include "openal/al_source_pool.h"
#include "tier1/utlvector.h"

class CALSystem : public CBaseGameSystemPerFrame {
    DECLARE_CLASS_GAMEROOT( CALSystem, CBaseGameSystemPerFrame );

public:
    CALSystem();

    bool Init();
    void Shutdown();
    void Update( float frametime );
    void ThreadUpdate();
    const char *Name();

    CALBuffer *LoadBuffer( const char *pszSoundName );
    CALBuffer *FindBuffer( const char *pszSoundName );

    CALSourcePool *AllocPool();
    void FreePool( CALSourcePool *pPool );

    CALSourcePool *GetGlobalPool();

private:
    ALCcontext *m_pContext;
    ALCdevice *m_pDevice;
    bool m_bEffectsAvailable;
    CUtlVectorMT<CUtlVector<CALBuffer *>> m_vpBuffers;
    CUtlVectorMT<CUtlVector<CALSourcePool *>> m_vpPools;
    CALSourcePool *m_pPool;
};

extern CALSystem *g_pOpenAL;

#endif

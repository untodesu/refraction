#ifndef AL_SOURCE_POOL_H
#define AL_SOURCE_POOL_H 1
#include "tier1/utlvector.h"

#define ALSOURCEPOOL_MIN_SIZE 16

class CALSystem;
class CALSound;
class CALSoundStream;

// CALSourcePool is just a class that is used in order
// to get a new sound source whenever it is needed.
// The first ALSOURCEPOOL_MIN_SIZE sources of both types are PERSISTENT.
class CALSourcePool {
public:
    CALSourcePool();
    virtual ~CALSourcePool();
    void Update();
    template<typename T> T *GetFreeSource();

private:
    CUtlVector<CALSound *> m_vpSounds;
    CUtlVector<CALSoundStream *> m_vpSoundStreams;
};

#endif

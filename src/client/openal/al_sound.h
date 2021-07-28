#ifndef AL_SOUND_H
#define AL_SOUND_H 1
#include "openal/al_buffer.h"
#include "openal/al_source.h"

// CALSound class is a sound that is being emitted
// multiple times and thus needs to be preloaded. It
// uses CALBuffer (reference-counted) to get the wave data.
class CALSound : public CBaseALSource {
public:
    CALSound();
    virtual ~CALSound();

    void SetBuffer( CALBuffer *pBuffer );

    bool IsStreaming();

    bool Play();
    void Pause();
    void Stop();
    
    void Update();

private:
    CALBuffer *m_pBuffer;
};

#endif

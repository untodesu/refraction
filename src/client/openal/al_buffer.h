#ifndef AL_BUFFER_H
#define AL_BUFFER_H 1
#include "openal/al_decoder.h"

// CALBuffer is a reference-counted class which is loaded
// and managed by the sound system. This class is being used
// by CALSound as a source of the PCM wave data.
class CALBuffer {
public:
    CALBuffer();
    virtual ~CALBuffer();

    int IncreaseRefCount();
    int DecreaseRefCount();
    int RefCount();

    void Read( CBaseALDecoder *pDecoder );
    const char *GetFilename();
    ALuint GetBuffer();

private:
    int m_iRefCount;
    ALuint m_hBuffer;
    char m_szFilename[ALDECODER_MAX_FILENAME];
};

#endif

#ifndef AL_SOUND_STREAM_H
#define AL_SOUND_STREAM_H 1
#include "openal/al_decoder.h"
#include "openal/al_source.h"

#define NUM_ALBUFFERS 4
#define ALBUFFER_SIZE 16384
#define STREAM_STRLEN 128

// CALSoundStream is a sound source that is
// being streamed from disk (IFileSystem). It
// opens the file when Play() is called and closes
// it when Stop() is called. It uses CBaseALDecoder.
class CALSoundStream : public CBaseALSource {
public:
    CALSoundStream();
    virtual ~CALSoundStream();

    void SetFilename( const char *pszFilename );

    bool IsStreaming();

    bool Play();
    void Pause();
    void Stop();

    void Update();

private:
    bool ReadChunk( ALuint hBuffer );

private:
    CBaseALDecoder *m_pDecoder;
    ALuint m_ahBuffers[NUM_ALBUFFERS];
    char m_szFilename[STREAM_STRLEN];
};

#endif

#ifndef AL_DECODER_WAV_H
#define AL_DECODER_WAV_H 1
#include "openal/al_decoder.h"
#include "dr_wav.h"

class CALDecoder_WAV : public CBaseALDecoder {
public:
    CALDecoder_WAV( IFileSystem *pFilesystem );

    bool Create( FileHandle_t hFileHandle );
    void Destroy();

    ALenum GetFormat();
    void Rewind();
    uint32 Read( uint16 *pOut, uint32 uRequestSize );
    uint32 GetSize();
    uint32 GetSampleRate();

private:
    static size_t OnRead( void *pArg, void *pOut, size_t zuSize );
    static drwav_bool32 OnSeek( void *pArg, int iOffset, drwav_seek_origin origin );

private:
    drwav m_WAV;
};

#endif

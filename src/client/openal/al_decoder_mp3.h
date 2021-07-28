#ifndef AL_DECODER_MP3_H
#define AL_DECODER_MP3_H 1
#include "openal/al_decoder.h"
#include "dr_mp3.h"

class CALDecoder_MP3 : public CBaseALDecoder {
public:
    CALDecoder_MP3( IFileSystem *pFilesystem );

    bool Create( FileHandle_t hFileHandle );
    void Destroy();

    ALenum GetFormat();
    void Rewind();
    uint32 Read( uint16 *pOut, uint32 uRequestSize );
    uint32 GetSize();
    uint32 GetSampleRate();

private:
    static size_t OnRead( void *pArg, void *pOut, size_t zuSize );
    static drmp3_bool32 OnSeek( void *pArg, int iOffset, drmp3_seek_origin origin );

private:
    drmp3 m_MP3;
    drmp3_uint64 m_uFrameCount;
};

#endif

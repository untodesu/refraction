#ifndef AL_DECODER_H
#define AL_DECODER_H 1
#include "AL/al.h"
#include "filesystem.h"
#include "tier0/platform.h"

#define ALDECODER_MAX_FILENAME 256

// CBaseALDecoder is a base class used in order
// to provide a format-agnostic API for CBaseALBuffer
// and CBaseALStream to read and write PCM data from
// various formats and libraries such as dr_wav or libxm.
class CBaseALDecoder {
public:
    CBaseALDecoder( IFileSystem *pFilesystem );
    virtual ~CBaseALDecoder();

    IFileSystem *GetFilesystem();
    FileHandle_t GetFileHandle();

    bool OpenFile( const char *pszFilename );
    const char *GetFilename();

    virtual bool Create( FileHandle_t hFileHandle );
    virtual void Destroy();

    virtual ALenum GetFormat();
    virtual void Rewind();
    virtual uint32 Read( uint16 *pOut, uint32 uRequestSize );
    virtual uint32 GetSize();
    virtual uint32 GetSampleRate();

protected:
    IFileSystem *m_pFilesystem;
    FileHandle_t m_hFileHandle;
    char m_szFilename[ALDECODER_MAX_FILENAME];
};

#endif

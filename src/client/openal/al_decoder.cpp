#include "cbase.h"
#include "openal/al_decoder.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CBaseALDecoder::CBaseALDecoder(IFileSystem *pFilesystem) : m_pFilesystem(pFilesystem), m_hFileHandle(FILESYSTEM_INVALID_HANDLE)
{

}

CBaseALDecoder::~CBaseALDecoder()
{
    Destroy();
}

IFileSystem *CBaseALDecoder::GetFilesystem()
{
    return m_pFilesystem;
}

FileHandle_t CBaseALDecoder::GetFileHandle()
{
    return m_hFileHandle;
}

bool CBaseALDecoder::OpenFile( const char *pszFilename )
{
    FileHandle_t hFileHandle = m_pFilesystem->Open( pszFilename, "rb", "GAME" );
    if( !Create( hFileHandle ) ) {
        m_szFilename[0] = 0;
        DevWarning( "CBaseALDecoder: unable to open %s\n", pszFilename );
        return false;
    }

    Q_strncpy( m_szFilename, pszFilename, ALDECODER_MAX_FILENAME );
    return true;
}

const char *CBaseALDecoder::GetFilename()
{
    return m_szFilename;
}

bool CBaseALDecoder::Create( FileHandle_t hFileHandle )
{
    Destroy();
    m_hFileHandle = hFileHandle;
    return m_hFileHandle != FILESYSTEM_INVALID_HANDLE;
}

void CBaseALDecoder::Destroy()
{
    m_hFileHandle = FILESYSTEM_INVALID_HANDLE;
}

ALenum CBaseALDecoder::GetFormat()
{
    return AL_NONE;
}

void CBaseALDecoder::Rewind()
{

}

uint32 CBaseALDecoder::Read( uint16 *pOut, uint32 uRequestSize )
{
    // Read nothing
    return 0;
}

uint32 CBaseALDecoder::GetSize()
{
    return 0;
}

uint32 CBaseALDecoder::GetSampleRate()
{
    return 0;
}

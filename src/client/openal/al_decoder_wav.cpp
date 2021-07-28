#include "cbase.h"
#define DR_WAV_IMPLEMENTATION 1
#define DR_WAV_NO_STDIO 1
#include "openal/al_decoder_wav.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static void *DRWAV_MallocImpl( size_t sz, void *pArg )
{
    (void)( pArg );
    return malloc( sz );
}

static void *DRWAV_ReallocImpl( void *p, size_t sz, void *pArg )
{
    (void)( pArg );
    return realloc( p, sz );
}

static void DRWAV_FreeImpl( void *p, void *pArg )
{
    (void)( pArg );
    free( p );
}

// HACKHACK: memdbgon redefines malloc(), realloc() and free()
static drwav_allocation_callbacks s_allocationCallbacks = {
    NULL,
    DRWAV_MallocImpl,
    DRWAV_ReallocImpl,
    DRWAV_FreeImpl
};

CALDecoder_WAV::CALDecoder_WAV( IFileSystem *pFilesystem ) : CBaseALDecoder( pFilesystem ), m_WAV()
{

}

bool CALDecoder_WAV::Create( FileHandle_t hFileHandle )
{
    if( !CBaseALDecoder::Create( hFileHandle ) )
        return false;
    return !!drwav_init( &m_WAV, OnRead, OnSeek, this, &s_allocationCallbacks );
}

void CALDecoder_WAV::Destroy()
{
    drwav_uninit( &m_WAV );
    Q_memset( &m_WAV, 0, sizeof( m_WAV ) );
    CBaseALDecoder::Destroy();
}

ALenum CALDecoder_WAV::GetFormat()
{
    if( m_WAV.channels == 2 )
        return AL_FORMAT_STEREO16;
    return AL_FORMAT_MONO16;
}

void CALDecoder_WAV::Rewind()
{
    if( m_hFileHandle == FILESYSTEM_INVALID_HANDLE )
        return;
    drwav_seek_to_pcm_frame( &m_WAV, 0 );
}

uint32 CALDecoder_WAV::Read( uint16 *pOut, uint32 uRequestSize )
{
    if( m_hFileHandle != FILESYSTEM_INVALID_HANDLE )
        return (uint32)drwav_read_pcm_frames_s16( &m_WAV, (drwav_uint64)uRequestSize, (drwav_int16 *)pOut );
    return 0;
}

uint32 CALDecoder_WAV::GetSize()
{
    if( m_hFileHandle != FILESYSTEM_INVALID_HANDLE )
        return (uint32)m_WAV.totalPCMFrameCount;
    return 0;
}

uint32 CALDecoder_WAV::GetSampleRate()
{
    return m_WAV.sampleRate;
}

size_t CALDecoder_WAV::OnRead( void *pArg, void *pOut, size_t zuSize )
{
    // NOTE: it seems that IBaseFilesystem::Read doesn't return negative values
    CALDecoder_WAV *pThis = (CALDecoder_WAV *)pArg;
    if( pThis && pThis->m_hFileHandle != FILESYSTEM_INVALID_HANDLE )
        return (size_t)pThis->m_pFilesystem->Read( pOut, (int)zuSize, pThis->m_hFileHandle );
    return 0;
}

drwav_bool32 CALDecoder_WAV::OnSeek( void *pArg, int iOffset, drwav_seek_origin origin )
{
    CALDecoder_WAV *pThis = (CALDecoder_WAV *)pArg;
    if( pThis && pThis->m_hFileHandle != FILESYSTEM_INVALID_HANDLE ) {
        FileSystemSeek_t fsSeek = FILESYSTEM_SEEK_CURRENT;
        switch( origin ) {
            case drwav_seek_origin_current:
                fsSeek = FILESYSTEM_SEEK_CURRENT;
            case drwav_seek_origin_start:
                fsSeek = FILESYSTEM_SEEK_HEAD;
        }

        pThis->m_pFilesystem->Seek( pThis->m_hFileHandle, iOffset, fsSeek );
        return DRWAV_TRUE;
    }

    return DRWAV_FALSE;
}

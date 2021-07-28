#include "cbase.h"
#define DR_MP3_IMPLEMENTATION 1
#define DR_MP3_NO_STDIO 1
#include "openal/al_decoder_mp3.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static void *DRMP3_MallocImpl( size_t sz, void *pArg )
{
    (void)( pArg );
    return malloc( sz );
}

static void *DRMP3_ReallocImpl( void *p, size_t sz, void *pArg )
{
    (void)( pArg );
    return realloc( p, sz );
}

static void DRMP3_FreeImpl( void *p, void *pArg )
{
    (void)( pArg );
    free( p );
}

// HACKHACK: memdbgon redefines malloc(), realloc() and free()
static drmp3_allocation_callbacks s_allocationCallbacks = {
    NULL,
    DRMP3_MallocImpl,
    DRMP3_ReallocImpl,
    DRMP3_FreeImpl
};

CALDecoder_MP3::CALDecoder_MP3( IFileSystem *pFilesystem ) : CBaseALDecoder( pFilesystem ), m_MP3(), m_uFrameCount( 0 )
{

}

bool CALDecoder_MP3::Create( FileHandle_t hFileHandle )
{
    if( !CBaseALDecoder::Create( hFileHandle ) )
        return false;
    if( !drmp3_init( &m_MP3, OnRead, OnSeek, this, &s_allocationCallbacks ) )
        return false;
    drmp3_get_mp3_and_pcm_frame_count( &m_MP3, NULL, &m_uFrameCount );
    return true;
}

void CALDecoder_MP3::Destroy()
{
    m_uFrameCount = 0;
    drmp3_uninit( &m_MP3 );
    Q_memset( &m_MP3, 0, sizeof( m_MP3 ) );
    CBaseALDecoder::Destroy();
}

ALenum CALDecoder_MP3::GetFormat()
{
    if( m_MP3.channels == 2 )
        return AL_FORMAT_STEREO16;
    return AL_FORMAT_MONO16;
}

void CALDecoder_MP3::Rewind()
{
    if( m_hFileHandle == FILESYSTEM_INVALID_HANDLE )
        return;
    drmp3_seek_to_pcm_frame( &m_MP3, 0 );
}

uint32 CALDecoder_MP3::Read( uint16 *pOut, uint32 uRequestSize )
{
    if( m_hFileHandle != FILESYSTEM_INVALID_HANDLE )
        return (uint32)drmp3_read_pcm_frames_s16( &m_MP3, (drmp3_uint64)uRequestSize, (drmp3_int16 *)pOut );
    return 0;
}

uint32 CALDecoder_MP3::GetSize()
{
    return (uint32)m_uFrameCount;
}

uint32 CALDecoder_MP3::GetSampleRate()
{
    return m_MP3.sampleRate;
}

size_t CALDecoder_MP3::OnRead( void *pArg, void *pOut, size_t zuSize )
{
    // NOTE: it seems that IBaseFilesystem::Read doesn't return negative values
    CALDecoder_MP3 *pThis = (CALDecoder_MP3 *)pArg;
    if( pThis && pThis->m_hFileHandle != FILESYSTEM_INVALID_HANDLE ) {
        DevMsg( "POSITION: %u SIZE: %u\n", pThis->m_pFilesystem->Tell( pThis->m_hFileHandle ), (unsigned int)zuSize );
        return (size_t)pThis->m_pFilesystem->Read( pOut, (int)zuSize, pThis->m_hFileHandle );
    }
    return 0;
}

drmp3_bool32 CALDecoder_MP3::OnSeek( void *pArg, int iOffset, drmp3_seek_origin origin )
{
    CALDecoder_MP3 *pThis = (CALDecoder_MP3 *)pArg;
    if( pThis && pThis->m_hFileHandle != FILESYSTEM_INVALID_HANDLE ) {
        FileSystemSeek_t fsSeek = FILESYSTEM_SEEK_CURRENT;
        switch( origin ) {
            case drmp3_seek_origin_current:
                fsSeek = FILESYSTEM_SEEK_CURRENT;
            case drmp3_seek_origin_start:
                fsSeek = FILESYSTEM_SEEK_HEAD;
        }

        pThis->m_pFilesystem->Seek( pThis->m_hFileHandle, iOffset, fsSeek );
        return DRMP3_TRUE;
    }

    return DRMP3_FALSE;
}

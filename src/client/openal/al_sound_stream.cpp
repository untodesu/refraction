#include "cbase.h"
#include "openal/al_sound_stream.h"
#include "openal/al_utils.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CALSoundStream::CALSoundStream() : m_pDecoder(NULL)
{

}

CALSoundStream::~CALSoundStream()
{
    if( m_pDecoder )
        AL_CloseFile( m_pDecoder );
}

void CALSoundStream::SetFilename( const char *pszFilename )
{
    if( GetStatus() == ALSOURCE_PLAYING )
        DevWarning( "CALSoundStream: setting filename will take effect after stopping\n" );
    Q_strncpy( m_szFilename, pszFilename, STREAM_STRLEN - 1 );
}

bool CALSoundStream::IsStreaming()
{
    return true;
}

bool CALSoundStream::Play()
{
    if( GetStatus() == ALSOURCE_PLAYING )
        return true;

    if( m_pDecoder )
        AL_CloseFile( m_pDecoder );

    m_pDecoder = AL_OpenFile( m_szFilename );
    if( !m_pDecoder ) {
        MarkForDeletion();
        return false;
    }

    ALsizei iQueueSize = 0;
    for( int i = 0; i < NUM_ALBUFFERS; i++ ) {
        if( ReadChunk( m_ahBuffers[i] ) )
            iQueueSize++;
    }

    if( iQueueSize == 0 ) {
        DevWarning( "CALSoundStream::Play: unable to play: the stream is empty\n" );
        return false;
    }

    alSourceQueueBuffers( m_hSource, iQueueSize, m_ahBuffers );
    alSourcePlay( m_hSource );

    return true;
}

void CALSoundStream::Pause()
{
    alSourcePause( m_hSource );
}

void CALSoundStream::Stop()
{
    if( GetStatus() == ALSOURCE_STOPPED )
        return;

    // Streams should be gone after playing
    // with an exception for first 16 streams in
    // a source pool (just to keep this thing optimized)
    MarkForDeletion();

    alSourceRewind( m_hSource );
    alSourcei( m_hSource, AL_BUFFER, 0 );

    if( m_pDecoder ) {
        AL_CloseFile( m_pDecoder );
        m_pDecoder = NULL;
    }
}

void CALSoundStream::Update()
{
    CBaseALSource::Update();

    ALSourceStatus status = GetStatus();

    ALint iProcessed;
    alGetSourcei( m_hSource, AL_BUFFERS_PROCESSED, &iProcessed );
    while( iProcessed-- ) {
        ALuint hBuffer;
        alSourceUnqueueBuffers( m_hSource, 1, &hBuffer );
        if( ReadChunk( hBuffer ) ) {
            alSourceQueueBuffers( m_hSource, 1, &hBuffer );
            if( status != ALSOURCE_PLAYING && status != ALSOURCE_PAUSED )
                alSourcePlay( m_hSource );
        }
    }

    m_bDirty = false;
}

bool CALSoundStream::ReadChunk( ALuint hBuffer )
{
    if( IsMarkedForDeletion() )
        return false;

    uint16 auData[ALBUFFER_SIZE];
    uint32 uSize = m_pDecoder->Read( auData, ALBUFFER_SIZE );

    if( uSize == 0 ) {
        DevWarning( "CALSoundStream::CheckStream: read 0 bytes!\n" );
        return false;
    }

    if( uSize < ALBUFFER_SIZE ) {
        if( !m_bLooping ) {
            MarkForDeletion();
            return false;
        }

        m_pDecoder->Rewind();
    }

    alBufferData( hBuffer, m_pDecoder->GetFormat(), auData, (ALsizei)uSize, (ALsizei)m_pDecoder->GetSampleRate() );
    return uSize == ALBUFFER_SIZE;
}

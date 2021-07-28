#include "cbase.h"
#include "openal/al_sound.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CALSound::CALSound() : CBaseALSource(), m_pBuffer( NULL )
{

}

CALSound::~CALSound()
{
    if( m_pBuffer )
        m_pBuffer->DecreaseRefCount();
}

void CALSound::SetBuffer( CALBuffer *pBuffer )
{
    if( m_pBuffer ) {
        alSourcei( m_hSource, AL_BUFFER, 0 );
        m_pBuffer->DecreaseRefCount();
    }

    m_pBuffer = pBuffer;

    if( m_pBuffer ) {
        alSourcei( m_hSource, AL_BUFFER, (ALint)m_pBuffer->GetBuffer() );
        m_pBuffer->IncreaseRefCount();
    }
}

bool CALSound::IsStreaming()
{
    return false;
}

bool CALSound::Play()
{
    alSourcePlay( m_hSource );
    return true;
}

void CALSound::Pause()
{
    alSourcePause( m_hSource );
}

void CALSound::Stop()
{
    alSourceRewind( m_hSource );
}

void CALSound::Update()
{
    CBaseALSource::Update();
    m_bDirty = false;
}

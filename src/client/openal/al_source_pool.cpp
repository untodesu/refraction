#include "cbase.h"
#include "openal/al_sound.h"
#include "openal/al_sound_stream.h"
#include "openal/al_source_pool.h"
#include "openal/al_system.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CALSourcePool::CALSourcePool() : m_vpSounds(), m_vpSoundStreams()
{
    m_vpSounds.SetCount( ALSOURCEPOOL_MIN_SIZE );
    m_vpSoundStreams.SetCount( ALSOURCEPOOL_MIN_SIZE );
    
    for( int i = 0; i < ALSOURCEPOOL_MIN_SIZE; i++ ) {
        CALSound *pSound = new CALSound();
        pSound->Persist();
        m_vpSounds[i] = pSound;

        CALSoundStream *pSoundStream = new CALSoundStream();
        pSoundStream->Persist();
        m_vpSoundStreams[i] = pSoundStream;
    }
}

CALSourcePool::~CALSourcePool()
{
    for( int i = 0; i < m_vpSounds.Count(); i++ )
        delete m_vpSounds[i];
    m_vpSounds.RemoveAll();

    for( int i = 0; i < m_vpSoundStreams.Count(); i++ )
        delete m_vpSoundStreams[i];
    m_vpSoundStreams.RemoveAll();
}

void CALSourcePool::Update()
{
    // Sources before ALSOURCEPOOL_MIN_SIZE are persistent by definition
    for( int i = ALSOURCEPOOL_MIN_SIZE; i < m_vpSounds.Count(); i++ ) {
        if( m_vpSounds[i]->GetStatus() == ALSOURCE_PLAYING )
            m_vpSounds[i]->Update();
        if( m_vpSounds[i]->IsMarkedForDeletion() ) {
            delete m_vpSounds[i];
            m_vpSounds.Remove( i );
        }
    }

    // Sources before ALSOURCEPOOL_MIN_SIZE are persistent by definition
    for( int i = ALSOURCEPOOL_MIN_SIZE; i < m_vpSoundStreams.Count(); i++ ) {
        if( m_vpSoundStreams[i]->GetStatus() == ALSOURCE_PLAYING )
            m_vpSoundStreams[i]->Update();
        if( m_vpSoundStreams[i]->IsMarkedForDeletion() ) {
            delete m_vpSoundStreams[i];
            m_vpSoundStreams.Remove( i );
        }
    }
}

template<> CALSound *CALSourcePool::GetFreeSource<CALSound>()
{
    CALSound *pSound = NULL;

    for( int i = 0; i < m_vpSounds.Count(); i++ ) {
        pSound = m_vpSounds[i];
        ALSourceStatus status = pSound->GetStatus();
        if( !pSound->IsMarkedForDeletion() && status != ALSOURCE_PLAYING && status != ALSOURCE_PAUSED )
            return pSound;
    }

    pSound = new CALSound();
    m_vpSounds.AddToTail( pSound );
    return pSound;
}

template<> CALSoundStream *CALSourcePool::GetFreeSource<CALSoundStream>()
{
    CALSoundStream *pSoundStream = NULL;

    for( int i = 0; i < m_vpSoundStreams.Count(); i++ ) {
        pSoundStream = m_vpSoundStreams[i];
        ALSourceStatus status = pSoundStream->GetStatus();
        if( !pSoundStream->IsMarkedForDeletion() && status != ALSOURCE_PLAYING && status != ALSOURCE_PAUSED )
            return pSoundStream;
    }

    pSoundStream = new CALSoundStream();
    m_vpSoundStreams.AddToTail( pSoundStream );
    return pSoundStream;
}

#include "cbase.h"
#include "openal/al_source.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CBaseALSource::CBaseALSource() : m_bDirty( true ), m_bPositional( false ), m_bMarkedForDeletion( false ), m_bPersistent( false ), m_pEntity( NULL ), m_flGain( 1.0f ), m_bLooping( false )
{
    m_vPosition.Init( 0.0f, 0.0f, 0.0f );
    m_vVelocity.Init( 0.0f, 0.0f, 0.0f );

    alGenSources( 1, &m_hSource );
    alSourcef( m_hSource, AL_REFERENCE_DISTANCE, 1.0f / METERS_PER_INCH );
    EnablePositional( false );
}

CBaseALSource::~CBaseALSource()
{
    alDeleteSources( 1, &m_hSource );
}

ALSourceStatus CBaseALSource::GetStatus()
{
    ALint status;
    alGetSourcei( m_hSource, AL_SOURCE_STATE, &status );

    switch( status ) {
        case AL_STOPPED:
            return ALSOURCE_STOPPED;
        case AL_PAUSED:
            return ALSOURCE_PAUSED;
        case AL_PLAYING:
            return ALSOURCE_PLAYING;
    }

    return ALSOURCE_INVALID;
}

void CBaseALSource::MarkForDeletion()
{
    m_bMarkedForDeletion = true;
}

bool CBaseALSource::IsMarkedForDeletion()
{
    ALSourceStatus status = GetStatus();
    return !m_bPersistent && m_bMarkedForDeletion && status != ALSOURCE_PLAYING && status != ALSOURCE_PAUSED;
}

void CBaseALSource::Persist()
{
    m_bPersistent = true;
}

bool CBaseALSource::IsPersistent()
{
    return m_bPersistent;
}

void CBaseALSource::EnablePositional( bool bEnable )
{
    ALint iIsRelative;
    alGetSourcei( m_hSource, AL_SOURCE_RELATIVE, &iIsRelative );
    if( iIsRelative )
        return;

    m_bPositional = bEnable;
    alSourcei( m_hSource, AL_SOURCE_RELATIVE, m_bPositional ? AL_FALSE : AL_TRUE );
    alSourcef( m_hSource, AL_ROLLOFF_FACTOR, m_bPositional ? 1.0f : 0.0f );

    m_bDirty = true;
}

bool CBaseALSource::IsPositional()
{
    return m_bPositional;
}

void CBaseALSource::SetPosition( const Vector &vPosition )
{
    m_vPosition = vPosition;
    m_bDirty = true;
}

void CBaseALSource::SetVelocity( const Vector &vVelocity )
{
    m_vVelocity = vVelocity;
    m_bDirty = true;
}

void CBaseALSource::SetEntity( C_BaseEntity *pEntity )
{
    m_pEntity = pEntity;
}

void CBaseALSource::Update()
{
    if( !m_bDirty && !m_pEntity )
        return;

    float aflPos[3], aflVel[3];
    if( m_bPositional ) {
        if( m_pEntity ) {
            Vector vPos = m_pEntity->GetLocalOrigin();
            aflPos[0] = vPos.x;
            aflPos[1] = vPos.y;
            aflPos[2] = vPos.z;

            Vector vVel = m_pEntity->GetLocalVelocity();
            aflVel[0] = vVel.x;
            aflVel[1] = vVel.y;
            aflVel[2] = vVel.z;
        }
        else {
            aflPos[0] = m_vPosition.x;
            aflPos[1] = m_vPosition.y;
            aflPos[2] = m_vPosition.z;

            aflVel[0] = m_vVelocity.x;
            aflVel[1] = m_vVelocity.y;
            aflVel[2] = m_vVelocity.z;
        }
    }
    else {
        aflPos[0] = 0.0f;
        aflPos[1] = 0.0f;
        aflPos[2] = 0.0f;

        aflVel[0] = 0.0f;
        aflVel[1] = 0.0f;
        aflVel[2] = 0.0f;
    }

    alSourcefv( m_hSource, AL_POSITION, aflPos );
    alSourcefv( m_hSource, AL_VELOCITY, aflVel );
    alSourcef( m_hSource, AL_GAIN, m_flGain );
    alSourcei( m_hSource, AL_LOOPING, m_bLooping ? AL_TRUE : AL_FALSE );

    // NOTE: this should be set after derived code.
    // m_bDirty = false;
}

void CBaseALSource::SetGain( float flGain )
{
    m_flGain = flGain;
    m_bDirty = true;
}

float CBaseALSource::GetGain()
{
    return m_flGain;
}

void CBaseALSource::SetLooping( bool bLooping )
{
    m_bLooping = bLooping;
    m_bDirty = true;
}

bool CBaseALSource::IsLooping()
{
    return m_bLooping;
}

ALuint CBaseALSource::GetSource()
{
    return m_hSource;
}

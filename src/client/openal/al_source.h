#ifndef AL_SOURCE_H
#define AL_SOURCE_H 1
#include "AL/al.h"
#include "mathlib/vector.h"

class C_BaseEntity;

enum ALSourceStatus {
    ALSOURCE_INVALID,
    ALSOURCE_STOPPED,
    ALSOURCE_PLAYING,
    ALSOURCE_PAUSED
};

// CBaseALSource is a base class representing a
// generic positional/direct sound in the world.
// This class is meant to be derived from by
// CALSound (cached) and CALSoundStream (streaming) classes.
class CBaseALSource {
public:
    CBaseALSource();
    virtual ~CBaseALSource();

    virtual bool IsStreaming() = 0;

    virtual bool Play() = 0;
    virtual void Pause() = 0;
    virtual void Stop() = 0;

    ALSourceStatus GetStatus();

    void MarkForDeletion();
    bool IsMarkedForDeletion();

    void Persist();
    bool IsPersistent();

    void EnablePositional( bool bEnable );
    bool IsPositional();

    void SetPosition( const Vector &vPosition );
    void SetVelocity( const Vector &vVelocity );
    void SetEntity( C_BaseEntity *pEntity );

    virtual void Update();

    void SetGain( float flGain );
    float GetGain();

    void SetLooping( bool bLooping );
    bool IsLooping();

    ALuint GetSource();

protected:
    ALuint m_hSource;
    bool m_bDirty;
    bool m_bPositional;
    bool m_bMarkedForDeletion;
    bool m_bPersistent;
    Vector m_vPosition;
    Vector m_vVelocity;
    C_BaseEntity *m_pEntity;
    float m_flGain;
    bool m_bLooping;
};

#endif

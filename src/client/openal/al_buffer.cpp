#include "cbase.h"
#include "openal/al_buffer.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CALBuffer::CALBuffer() : m_iRefCount(0), m_hBuffer(0)
{
    alGenBuffers( 1, &m_hBuffer );
}

CALBuffer::~CALBuffer()
{
    alDeleteBuffers( 1, &m_hBuffer );
}

int CALBuffer::IncreaseRefCount()
{
    return ++m_iRefCount;
}

int CALBuffer::DecreaseRefCount()
{
    return --m_iRefCount;
}

int CALBuffer::RefCount()
{
    return m_iRefCount;
}

void CALBuffer::Read( CBaseALDecoder *pDecoder )
{
    uint32 uRequestSize = pDecoder->GetSize();
    uint16 *pData = new uint16[uRequestSize];
    uint32 uRead = pDecoder->Read( pData, uRequestSize );
    alBufferData( m_hBuffer, pDecoder->GetFormat(), pData, sizeof( uint16 ) * uRead, (ALsizei)pDecoder->GetSampleRate() );
    Q_strncpy( m_szFilename, pDecoder->GetFilename(), ALDECODER_MAX_FILENAME );
    delete[] pData;
}

const char *CALBuffer::GetFilename()
{
    return m_szFilename;
}

ALuint CALBuffer::GetBuffer()
{
    return m_hBuffer;
}

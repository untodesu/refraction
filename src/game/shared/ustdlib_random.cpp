#include "cbase.h"
#include "ustdlib_random.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static CMT19937UniformRandomStream s_Mt19937;
IUniformRandomStream * g_pMt19937 = (IUniformRandomStream *)&s_Mt19937;

CMT19937UniformRandomStream::CMT19937UniformRandomStream() : m_mutex(), m_random()
{

}

void CMT19937UniformRandomStream::SetSeed( int iSeed )
{
    AUTO_LOCK( m_mutex );
    m_random.init( uint64_t( iSeed ) );
}

int CMT19937UniformRandomStream::RandomInt( int iLow, int iHigh )
{
    AUTO_LOCK( m_mutex );
    return (int)m_random.getInt( (uint64_t)iLow, (uint64_t)iHigh );
}

float CMT19937UniformRandomStream::RandomFloat( float flLow, float flHigh )
{
    AUTO_LOCK( m_mutex );
    return (float)m_random.getDouble( flLow, flHigh );
}

float CMT19937UniformRandomStream::RandomFloatExp( float flMinVal, float flMaxVal, float flExponent )
{
    AUTO_LOCK( m_mutex );
    float result = float( m_random.getDouble() );
    if( flExponent != 1.0f ) {
        result = powf( result, flExponent );
    }
    return ( result * ( flMaxVal - flMinVal ) ) + flMinVal;
}


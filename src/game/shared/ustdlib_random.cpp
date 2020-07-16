/**
* What the shit is this?
* This is a MT19937-64 pseudo-random number generator
* ported from ustdlib (https://github.com/undbsd/ustdlib) to refraction-mod
* to replace vstdlib's random piece of shit.
*/
#include "cbase.h"
#include "ustdlib_random.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

namespace ustd
{
    // Original algorithm has two loops and one workaround
    // which are pretty similar except TWO values, so I
    // decided to make this function to determine one of them
    // and pack all stuff into one single loop.
    // Another value just can be looped using this: [(i + 1) % NN]
    static inline const uint64_t MT_GetIndex( const uint64_t i )
    {
        if( i < ( Random::NN - Random::MM ) )   return ( i + Random::MM );
        if( i < ( Random::NN - 1 ) )            return ( i + ( Random::MM - Random::NN ) );
        return ( Random::MM - 1 );
    }

    Random::Random()
    {
        // What the fuck?
        // const uint64_t seed = QMath::floatToInt64Bits(ustd::GetUnixTime());
        const double cur_time = Sys_FloatTime();    // ^^ same shit probably
        Msg( "RandomSeed: %ld\n", *( (uint64_t*)&cur_time ) );
        initialize( *( (uint64_t *)&cur_time ) );
    }

    Random::Random( uint64_t seed )
    {
        // Initialize with used's seed!
        initialize( seed );
    }

    /**
    * Initializes _state[NN] with a seed
    */
    void Random::initialize( uint64_t seed )
    {
        _index = NN;
        _state[0] = seed;
        for( uint64_t i = 1; i < NN; i++ ) {
            uint64_t prev = _state[i - 1];
            _state[i] = ( 0x5851F42D4C957F2D * ( prev ^ ( prev >> 62 ) ) + i );
        }
    }

    /**
    * Generate NN words at one time
    */
    void Random::twist()
    {
        _index = 0;

        // So this loop contains all stuff from two loops and workaround.
        // God dammit, why are people choose to make longer code?
        // To make it look like brainfuck?
        for( uint64_t i = 0; i < NN; i++ ) {
            const uint64_t idx = MT_GetIndex( i );
            const uint64_t y = ( _state[i] & UM ) | ( _state[( i + 1 ) % NN] & LM );
            _state[i] = _state[idx] ^ ( y >> 1 ) ^ ( ( y & 1 ) * MATRIX_A );
        }
    }

    /**
    * Generates pseudo-random unsigned 64-bit integer
    */
    uint64_t Random::nextUInt64()
    {
        // So here, we are twisting values if it so needed.
        // Also there is no initialize(), because we're seeding
        // our generator in constructors.
        if( _index >= NN ) {
            twist();
        }

        uint64_t y = _state[_index++];
        y ^= ( y >> 29 ) & 0x5555555555555555;
        y ^= ( y << 17 ) & 0x71D67FFFEDA60000;
        y ^= ( y << 37 ) & 0xFFF7EEE000000000;
        y ^= ( y >> 43 );
        
        // Phew, FINALLY it yields same results as
        // std::mt19937_64. And also this is more readable
        // than most of this algorithm implementations.
        // Now I can be proud of myself...
        return y;
    }

    /**
    * Generates an unsigned pseudo-random 64-bit number
    * on the specified interval
    */
    uint64_t Random::nextUInt64( uint64_t min, uint64_t max )
    {
        return ( nextUInt64() * ( max - min ) + min );
    }

    /**
    * Generates a double-precision floating-point number
    * on [0.0 ... 1.0] interval.
    */
    double Random::nextFloat()
    {
        // [0.0 ... 1.0]
        return (double)( nextUInt64() >> 11 ) / (double)0x001FFFFFFFFFFFFF;
    }

    /**
    * Generates a double-precision floating-point number
    * on the specified interval
    */
    double Random::nextFloat( double min, double max )
    {
        return ( nextFloat() * ( max - min ) + min );
    }
}

// And this makes our random GREAT AS FUCK
static CMT19937UniformRandomStream s_Mt19937;
IUniformRandomStream * g_pMt19937 = (IUniformRandomStream *)&s_Mt19937;

CMT19937UniformRandomStream::CMT19937UniformRandomStream() : m_mutex(), m_random()
{

}

void CMT19937UniformRandomStream::SetSeed( int iSeed )
{
    AUTO_LOCK( m_mutex );
    m_random.initialize( uint64( iSeed ) );
}

int CMT19937UniformRandomStream::RandomInt( int iLow, int iHigh )
{
    AUTO_LOCK( m_mutex );
    // und: Seems that RandomInt() should return value in range [iLow..iHigh],
    // but the my MT19937 implementation has range [iLow..iHigh) (Never will be iHigh!!!)
    // So value = nextfloat(iLow, iHigh + 1) % iHigh + 1
    return (int)( m_random.nextFloat( iLow, iHigh + 1 ) ) % ( iHigh + 1 );
}

float CMT19937UniformRandomStream::RandomFloat( float flLow, float flHigh )
{
    AUTO_LOCK( m_mutex );
    return (float)m_random.nextFloat( flLow, flHigh );
}

float CMT19937UniformRandomStream::RandomFloatExp( float flMinVal, float flMaxVal, float flExponent )
{
    AUTO_LOCK( m_mutex );
    float result = float( m_random.nextFloat() );
    if( flExponent != 1.0f ) {
        result = powf( result, flExponent );
    }
    return ( result * ( flMaxVal - flMinVal ) ) + flMinVal;
}


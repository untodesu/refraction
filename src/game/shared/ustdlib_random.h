/**
 * What the shit is this?
 * This is a MT19937-64 pseudo-random number generator
 * ported from ustdlib (https://github.com/undbsd/ustdlib) to refraction-mod
 * to replace vstdlib's random piece of shit.
 */
#ifndef USTDLIB_RANDOM_H
#define USTDLIB_RANDOM_H
#ifdef _WIN32
#pragma once
#endif
#include <stdint.h>
#include <vstdlib/random.h>

namespace ustd
{
    /**
    * Mersenne twister (MT19937) pseudo-random number generator.
    * This is an alternative to std::mt19937_64.
    */
    class Random {
    public:
        static const uint64_t NN = 312;
        static const uint64_t MM = 156;
        static const uint64_t UM = 0xFFFFFFFF80000000;
        static const uint64_t LM = 0x000000007FFFFFFF;
        static const uint64_t MATRIX_A = 0xB5026F5AA96619E9;

    private:
        uint64_t _state[NN];
        uint64_t _index = 0;

    public:
        Random();
        Random( uint64_t seed );

        void initialize( uint64_t seed = 5489 );
        void twist();

        uint64_t nextUInt64();
        uint64_t nextUInt64( uint64_t min, uint64_t max );

        double nextFloat();
        double nextFloat( double min, double max );
    };
}

class CMT19937UniformRandomStream : public IUniformRandomStream {
private:
    ustd::Random m_random;
    CThreadFastMutex m_mutex;

public:
    CMT19937UniformRandomStream();

    // Sets the seed of the random number generator
    // MT19937: Refills the shift register
    void SetSeed( int iSeed );

    // Generates random numbers
    virtual int     RandomInt( int iMinVal, int iMaxVal );
    virtual float   RandomFloat( float flMinVal = 0.0f, float flMaxVal = 1.0f );
    virtual float   RandomFloatExp( float flMinVal = 0.0f, float flMaxVal = 1.0f, float flExponent = 1.0f );

};
extern IUniformRandomStream * g_pMt19937;

#endif

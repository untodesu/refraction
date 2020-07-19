// USTDLIB_RANDOM_H conflicts with <ustdlib/random.h>
#ifndef USTDLIB_RANDOM_REPLACEMENT_H
#define USTDLIB_RANDOM_REPLACEMENT_H
#ifdef _WIN32
#pragma once
#endif
#include <stdint.h>
#include <vstdlib/random.h>
#include <ustdlib/random.h>

class CMT19937UniformRandomStream : public IUniformRandomStream {
private:
    ustd::MT19937_RandomStream m_random;
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

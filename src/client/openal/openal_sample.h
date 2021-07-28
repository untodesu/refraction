#ifndef OPENAL_SAMPLE_H
#define OPENAL_SAMPLE_H 1
#pragma once
#include "AL/al.h"
#include "KeyValues.h"

class CBaseOpenALSample {
public:
    CBaseOpenALSample();
    virtual ~CBaseOpenALSample();

    void Init();
    void Shutdown();

    void Update( float flLastUpdate );



};

#endif

//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#ifndef FLASHLIGHTEFFECT_H
#define FLASHLIGHTEFFECT_H
#ifdef _WIN32
#pragma once
#endif

class CFlashlightEffect {
public:

    CFlashlightEffect( int nEntIndex = 0 );
    virtual ~CFlashlightEffect();

    void UpdateLight( const Vector &vecPos, const Vector &vecDir, const Vector &vecRight, const Vector &vecUp );
    void TurnOn();
    void TurnOff();

    inline bool IsOn()
    {
        return m_bIsOn;
    }

    inline ClientShadowHandle_t GetFlashlightHandle()
    {
        return m_hFlashlightHandle;
    }

    inline void SetFlashlightHandle( ClientShadowHandle_t h )
    {
        m_hFlashlightHandle = h;
    }

protected:
    void LightOff();
    bool m_bIsOn;
    int m_nEntIndex;
    float m_flDistMod;
    CTextureReference m_FlashlightTexture;
    ClientShadowHandle_t m_hFlashlightHandle;
};

class CHeadlightEffect : public CFlashlightEffect {
public:

    CHeadlightEffect();
    ~CHeadlightEffect();

    virtual void UpdateLight( const Vector &vecPos, const Vector &vecDir, const Vector &vecRight, const Vector &vecUp, int nDistance );
};



#endif // FLASHLIGHTEFFECT_H

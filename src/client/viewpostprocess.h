//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================

#ifndef VIEWPOSTPROCESS_H
#define VIEWPOSTPROCESS_H
#include "igamesystem.h"
#include "tier1/convar.h"

class IPostProcessSystem : public CBaseGameSystem {
public:
    virtual void DoBloomAndTonemapping( int iX, int iY, int iWidth, int iHeight, bool bFlashlight, bool bPostVGui ) = 0;
    virtual void DoMotionBlur( const CViewSetup &view, int iX, int iY, int iWidth, int iHeight ) = 0;
    virtual void DoCustomPostEffects( int iX, int iY, int iWidth, int iHeight ) = 0;
    virtual void DumpRT( int iWidth, int iHeight, const char *pszFilename ) = 0;
    virtual void EnableCustomPostEffect( const char *pszName, bool bEnable, bool bHold ) = 0;
    virtual void UnholdAllCustomPostEffects() = 0;
};

extern IPostProcessSystem *g_pPostProcess;

#endif // VIEWPOSTPROCESS_H

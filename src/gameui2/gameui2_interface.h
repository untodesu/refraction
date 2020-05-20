#ifndef GAMEUI2_INTERFACE_H
#define GAMEUI2_INTERFACE_H
#ifdef _WIN32
#pragma once
#endif

#include "engineinterface.h"
#include "GameUI/IGameUI2.h"
#include "vgui_controls/AnimationController.h"
#include "gameui2_rootpanel.h"
#include "gameui2_api.h"

class CGameUI2 : public IGameUI2 {
public:
    IGameUI *m_pGameUI1;
    CGameUI2RootPanel *m_pRootPanel;
    vgui::AnimationController *m_pAnimController;

public:
    virtual void Initialize( CreateInterfaceFn appSystemFactory );
    virtual void Shutdown();
    virtual void VidInit();

    virtual void PostInit();
    virtual void PreShutdown();
    virtual void OnUpdate();
    virtual void LevelInitPreEntity( const char *pMapName );
    virtual void LevelInitPostEntity();
    virtual void LevelShutdown();

    vgui::VPANEL GetVPanel();
};
extern CGameUI2 g_GameUI2;

#endif

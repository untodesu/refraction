#ifndef GAMEUI2_ROOTPANEL_H
#define GAMEUI2_ROOTPANEL_H
#ifdef _WIN32
#pragma once
#endif

#include "vgui_controls/Panel.h"

class CGameUI2RootPanel : public vgui::Panel {
    DECLARE_CLASS_SIMPLE( CGameUI2RootPanel, vgui::Panel );

public:
    CGameUI2RootPanel( vgui::VPANEL parent );
    virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
};

#endif

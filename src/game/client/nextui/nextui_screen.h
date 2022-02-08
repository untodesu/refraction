#ifndef _NEXTUI_SCREEN_H_
#define _NEXTUI_SCREEN_H_ 1
#pragma once
#include "vgui_controls/Panel.h"

class CNextUIScreen : public vgui::Panel {
    DECLARE_CLASS_SIMPLE(CNextUIScreen, vgui::Panel);

public:
    CNextUIScreen(vgui::Panel *pParent, const char *pszPanelName);
    virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
    virtual void OnActivate();
    void Activate();

private:
    bool m_bSchemeLoaded;
    bool m_bQueuedActivate;
};

#endif

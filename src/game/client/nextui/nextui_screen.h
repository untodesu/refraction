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
    virtual void Paint();
    void SetTitle(const char *pszTitle);
    void SetTitle(const wchar_t *pszTitle);
    void Activate();

private:
    wchar_t m_szTitle[128];
    vgui::HFont m_hTitleFont = 0;
    Color m_TitleColor = Color();
    float m_flTitleX = 0.0f;
    float m_flTitleY = 0.0f;

private:
    bool m_bSchemeLoaded;
    bool m_bQueuedActivate;
};

#endif

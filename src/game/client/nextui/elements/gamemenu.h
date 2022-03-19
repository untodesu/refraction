#ifndef _NEXTUI_GAMEMENU_H_
#define _NEXTUI_GAMEMENU_H_ 1
#pragma once
#include "nextui/nextui_screen.h"
#include "tier1/utlvector.h"

class CGameMenuButton;
class CGameMenu : public CNextUIScreen {
    DECLARE_CLASS_SIMPLE(CGameMenu, CNextUIScreen);

public:
    CGameMenu(vgui::Panel *pParent);
    void ApplySchemeSettings(vgui::IScheme *pScheme) override;
    void Paint() override;
    void PaintBackground() override;
    void OnThink() override;
    void Rearrange();
    void RemoveAll();

private:
    vgui::HFont m_hTitleFont = 0;
    Color m_FadeColor = Color();
    Color m_TitleColor = Color();
    float m_flGradientWidthPercent = 0.0f;
    float m_flButtonStackX = 0.0f;
    float m_flButtonStackY = 0.0f;
    float m_flTitleX = 0.0f;
    float m_flTitleY = 0.0f;

private:
    bool m_bWasInGame = false;
    CUtlVector<CGameMenuButton *> m_vecButtons;
    wchar_t m_szTitleText[128];

private:
    int m_iScreenW = 0;
    int m_iScreenH = 0;
    int m_iButtonHeight = 0;
};

#endif

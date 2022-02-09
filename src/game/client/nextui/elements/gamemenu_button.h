#ifndef _NEXTUI_GAMEMENU_BUTTON_H_
#define _NEXTUI_GAMEMENU_BUTTON_H_ 1
#pragma once
#include "nextui/nextui_button.h"
#include "vgui_controls/AnimationController.h"

class CGameMenuButton : public CNextUIButton {
    DECLARE_CLASS_SIMPLE(CGameMenuButton, CNextUIButton);
    friend class CGameMenu;

public:
    CGameMenuButton(vgui::Panel *pParent, const char *pszCommand = NULL);
    void ApplySchemeSettings(vgui::IScheme *pScheme) override;
    void OnThink() override;
    void PaintBackground() override;
    void Paint() override;

private:
    bool m_bLastHighlight = false;
    float m_flResWidth = 0.0f;
    float m_flResHeight = 0.0f;

private:
    CPanelAnimationVarAliasType(float, m_flHighlightWidth, "HighlightWidth", "0", "proportional_float");
    CPanelAnimationVarAliasType(float, m_flTextOffsetX, "TextOffsetX", "0", "proportional_float");
    CPanelAnimationVarAliasType(float, m_flTextOffsetY, "TextOffsetY", "0", "proportional_float");
    CPanelAnimationVar(Color, m_BackgroundColor, "BackgroundColor", "0 0 0 0");
    CPanelAnimationVar(Color, m_HighlightColor, "HighlightColor", "0 0 0 0");
    CPanelAnimationVar(Color, m_TextColor, "TextColor", "0 0 0 0");

private:
    vgui::HFont m_hFont = 0;
    float m_flWidth = 0.0f;
    float m_flHeight = 0.0f;
    float m_flHighlightWidth_Normal = 0.0f;
    float m_flHighlightWidth_Highlight = 0.0f;
    float m_flTextOffsetX_Normal = 0.0f;
    float m_flTextOffsetX_Highlight = 0.0f;
    float m_flTextOffsetY_Normal = 0.0f;
    float m_flTextOffsetY_Highlight = 0.0f;
    Color m_BackgroundColor_Normal = Color();
    Color m_BackgroundColor_Highlight = Color();
    Color m_HighlightColor_Normal = Color();
    Color m_HighlightColor_Highlight = Color();
    Color m_TextColor_Normal = Color();
    Color m_TextColor_Highlight = Color();
};



#endif

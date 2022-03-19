#ifndef _NEXTUI_DIALOG_H_
#define _NEXTUI_DIALOG_H_ 1
#include "nextui_button.h"
#include "nextui_screen.h"
#include "vgui_controls/AnimationController.h"
#include "tier1/utlvector.h"

class CNextUIDialogButton : public CNextUIButton {
    DECLARE_CLASS_SIMPLE(CNextUIDialogButton, CNextUIButton);

public:
    CNextUIDialogButton(vgui::Panel *pParent, const char *pszText);
    CNextUIDialogButton(vgui::Panel *pParent, const wchar_t *pszText);
    virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
    virtual void PaintBackground();
    virtual void Paint();
    virtual void OnThink();
    void SetPosition(int iXPos);
    void SetPositionIdx(int iID, int iCount);

private:
    int m_iXPos = 0;
    float m_flResWidth = 0.0f;
    float m_flResHeight = 0.0f;
    bool m_bLastHighlight = false;

private:
    CPanelAnimationVar(Color, m_BackgroundColor, "BackgroundColor", "0 0 0 0");
    CPanelAnimationVar(Color, m_TextColor, "TextColor", "0 0 0 0");

private:
    vgui::HFont m_hTextFont = 0;
    float m_flBaseX = 0.0f;
    float m_flBaseY = 0.0f;
    float m_flWidth = 0.0f;
    Color m_BackgroundColor_Normal = Color();
    Color m_BackgroundColor_Highlight = Color();
    Color m_TextColor_Normal = Color();
    Color m_TextColor_Highlight = Color();
};

class CNextUIDialog : public CNextUIScreen {
    DECLARE_CLASS_SIMPLE(CNextUIDialog, CNextUIScreen);

public:
    CNextUIDialog(vgui::Panel *pParent, const char *pszPanelName);
    virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
    virtual void PaintBackground();
    virtual void Paint();
    virtual void OnActivate();
    void AddButton(CNextUIDialogButton *pButton);
    void SetText(const char *pszText);
    void SetText(const wchar_t *pszText);

private:
    wchar_t m_szText[2048];
    float m_flGradientHeightPercentInv = 0.0f;
    CUtlVector<CNextUIDialogButton *> m_vecButtons;

private:
    CPanelAnimationVar(Color, m_FadeColor, "FadeColor", "0 0 0 0");

private:
    vgui::HFont m_hTextFont = 0;
    float m_flFadeTime = 0.0f;
    float m_flGradientHeightPercent = 0.0f;
    float m_flTextX = 0.0f;
    float m_flTextY = 0.0f;
    Color m_TargetFadeColor = Color();
    Color m_TextColor = Color();
};

class CNextUIDialogButton_CloseParent : public CNextUIDialogButton {
    DECLARE_CLASS_SIMPLE(CNextUIDialogButton_CloseParent, CNextUIDialogButton);

public:
    CNextUIDialogButton_CloseParent(CNextUIDialog *pParent, const char *pszText);
    CNextUIDialogButton_CloseParent(CNextUIDialog *pParent, const wchar_t *pszText);
    virtual void SetParent(CNextUIDialog *pParent);
    virtual void OnClick();

private:
    CNextUIDialog *m_pParent;
};

#endif

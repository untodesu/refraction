#include "cbase.h"
#include "nextui_int.h"
#include "nextui_popup.h"
#include "vgui/ISurface.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CNextUIPopupButton::CNextUIPopupButton(vgui::Panel *pParent, const char *pszText)
    : BaseClass(pParent, "", pszText)
{
    SetPaintBackgroundEnabled(true);
    SetPaintBorderEnabled(false);
    SetPaintEnabled(true);
}

CNextUIPopupButton::CNextUIPopupButton(vgui::Panel *pParent, const wchar_t *pszText)
    : BaseClass(pParent, "", pszText), m_BackgroundColor(), m_TextColor()
{
    SetPaintBackgroundEnabled(true);
    SetPaintBorderEnabled(false);
    SetPaintEnabled(true);
}

void CNextUIPopupButton::ApplySchemeSettings(vgui::IScheme *pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);

    m_hTextFont = pScheme->GetFont("PopupButton", true);

    m_flBaseX = atof(pScheme->GetResourceString("Popup.Button.BaseX"));
    m_flBaseY = atof(pScheme->GetResourceString("Popup.Button.BaseY"));
    m_flWidth = atof(pScheme->GetResourceString("Popup.Button.Width"));

    m_BackgroundColor_Normal = GetSchemeColor("Popup.Button.BackgroundColor.Normal", pScheme);
    m_BackgroundColor_Highlight = GetSchemeColor("Popup.Button.BackgroundColor.Highlight", pScheme);
    m_TextColor_Normal = GetSchemeColor("Popup.Button.TextColor.Normal", pScheme);
    m_TextColor_Highlight = GetSchemeColor("Popup.Button.TextColor.Highlight", pScheme);

    m_BackgroundColor = m_BackgroundColor_Normal;
    m_TextColor = m_TextColor_Normal;

    m_flResWidth = XRES(m_flWidth);
    m_flResHeight = XRES(atof(pScheme->GetResourceString("Popup.Button.Height")));
    SetSize(m_flResWidth, m_flResHeight);
    SetPosition(m_iXPos);

    // After scheme reloading we should re-animate the panel
    // so the animation controller re-calculates the correct
    // values for whatever stuff that has been stuck after
    // video mode change (eg. proportional offsets).
    m_bLastHighlight = !m_bHighlight;
}

void CNextUIPopupButton::PaintBackground()
{
    int iWide, iTall;
    GetSize(iWide, iTall);
    g_pVGuiSurface->DrawSetColor(m_BackgroundColor);
    g_pVGuiSurface->DrawFilledRect(0, 0, iWide, iTall);
    g_pVGuiSurface->DrawFilledRectFade(0, 0, iWide, iTall / 2 - 1, 255, 0, false);
}

void CNextUIPopupButton::Paint()
{
    const wchar_t *pszText = GetText();
    if(pszText[0]) {
        int iTextW, iTextH;
        g_pVGuiSurface->GetTextSize(m_hTextFont, pszText, iTextW, iTextH);
        g_pVGuiSurface->DrawSetTextColor(m_TextColor);
        g_pVGuiSurface->DrawSetTextFont(m_hTextFont);
        g_pVGuiSurface->DrawSetTextPos((m_flResWidth / 2.0f) - (iTextW / 2.0f), (m_flResHeight / 2.0f) - (iTextH / 2.0f));
        g_pVGuiSurface->DrawPrintText(pszText, Q_wcslen(pszText));
    }
}

void CNextUIPopupButton::OnThink()
{
    BaseClass::OnThink();

    if(m_bLastHighlight != m_bHighlight) {
        vgui::AnimationController *pAC = CNextUI::GetInstance()->GetAnimationController();
        m_bLastHighlight = m_bHighlight;
        if(m_bLastHighlight) {
            pAC->RunAnimationCommand(this, "BackgroundColor", m_BackgroundColor_Highlight, 0.0f, 0.25f, pAC->INTERPOLATOR_LINEAR);
            pAC->RunAnimationCommand(this, "TextColor", m_TextColor_Highlight, 0.0f, 0.25f, pAC->INTERPOLATOR_LINEAR);
        }
        else {
            pAC->RunAnimationCommand(this, "BackgroundColor", m_BackgroundColor_Normal, 0.0f, 0.25f, pAC->INTERPOLATOR_LINEAR);
            pAC->RunAnimationCommand(this, "TextColor", m_TextColor_Normal, 0.0f, 0.25f, pAC->INTERPOLATOR_LINEAR);
        }
    }
}

void CNextUIPopupButton::SetPosition(int iXPos)
{
    int iWide, iTall;
    GetSize(iWide, iTall);

    int iWidth, iHeight;
    g_pVGuiSurface->GetScreenSize(iWidth, iHeight);

    m_iXPos = iXPos;

    int iNewXPos = XRES(m_iXPos);
    int iNewYPos = iHeight - iTall - YRES(m_flBaseY);
    SetPos(iNewXPos, iNewYPos);
}

void CNextUIPopupButton::SetPositionIdx(int iID, int iCount)
{
    SetPosition(m_flBaseX + (640.f - m_flWidth - m_flBaseX * 2.0f) * (float)iID / (float)iCount);
}

CNextUIPopup::CNextUIPopup(vgui::Panel *pParent, const char *pszPanelName)
    : BaseClass(pParent, pszPanelName), m_szText()
{
    // HACK: we should set this from the outside
    SetScheme(CNextUI::GetInstance()->GetScheme());

    SetPaintBackgroundEnabled(true);
    SetPaintBorderEnabled(false);
    SetPaintEnabled(true);
}

void CNextUIPopup::ApplySchemeSettings(vgui::IScheme *pScheme)
{
    m_hTextFont = pScheme->GetFont("PopupText", true);

    m_flFadeTime = atof(pScheme->GetResourceString("Popup.Background.FadeTime"));
    m_flGradientHeightPercent = atof(pScheme->GetResourceString("Popup.Background.GradientHeightPercent")) / 100.0f;
    m_flTextX = XRES(atof(pScheme->GetResourceString("Popup.Text.PositionX")));
    m_flTextY = YRES(atof(pScheme->GetResourceString("Popup.Text.PositionY")));

    m_flGradientHeightPercentInv = Max(0.0f, 1.0f - m_flGradientHeightPercent);

    m_TargetFadeColor = GetSchemeColor("Popup.Background.FadeColor", pScheme);
    m_TextColor = GetSchemeColor("Popup.Text.Color", pScheme);

    int iButtonCount = m_vecButtons.Count() - 1;
    if(iButtonCount >= 0) {
        for(int i = 0; i <= iButtonCount; i++) {
            m_vecButtons[i]->SetPositionIdx(i, iButtonCount);
        }
    }

    CNextUIScreen::ApplySchemeSettings(pScheme);
}

void CNextUIPopup::PaintBackground()
{
    int iWide, iTall;
    GetSize(iWide, iTall);
    g_pVGuiSurface->DrawSetColor(m_FadeColor);
    g_pVGuiSurface->DrawFilledRect(0, 0, iWide, iTall);
    g_pVGuiSurface->DrawFilledRectFade(0, 0, iWide, iTall * m_flGradientHeightPercent, 0xFF, 0x00, false);
    g_pVGuiSurface->DrawFilledRectFade(0, iTall * m_flGradientHeightPercentInv, iWide, iTall, 0x00, 0xFF, false);
}

void CNextUIPopup::Paint()
{
    CNextUIScreen::Paint();

    if(m_szText[0]) {
        g_pVGuiSurface->DrawSetTextColor(m_TextColor);
        g_pVGuiSurface->DrawSetTextFont(m_hTextFont);
        g_pVGuiSurface->DrawSetTextPos(m_flTextX, m_flTextY);
        g_pVGuiSurface->DrawPrintText(m_szText, Q_wcslen(m_szText));
    }
}

void CNextUIPopup::OnActivate()
{
    m_FadeColor = Color(0, 0, 0, 0);
    vgui::AnimationController *pAC = CNextUI::GetInstance()->GetAnimationController();
    pAC->RunAnimationCommand(this, "FadeColor", m_TargetFadeColor, 0.0, m_flFadeTime, pAC->INTERPOLATOR_DEACCEL);
}

void CNextUIPopup::AddButton(CNextUIPopupButton *pButton)
{
    int iCount = m_vecButtons.Count();
    for(int i = 0; i < iCount; i++) {
        if(m_vecButtons[i] != pButton)
            continue;
        return;
    }

    m_vecButtons.AddToTail(pButton);
}

void CNextUIPopup::SetText(const char *pszText)
{
    CNextUI::GetInstance()->GetLocalizedString(pszText, m_szText, sizeof(m_szText) / sizeof(wchar_t));
}

void CNextUIPopup::SetText(const wchar_t *pszText)
{
    Q_wcsncpy(m_szText, pszText, sizeof(m_szText));
}

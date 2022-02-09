#include "cbase.h"
#include "gamemenu_button.h"
#include "nextui/nextui_int.h"
#include "vgui/ISurface.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CGameMenuButton::CGameMenuButton(vgui::Panel *pParent, const char *pszCommand)
    : BaseClass(pParent, "", ""), m_flHighlightWidth(), m_flTextOffsetX(), m_flTextOffsetY(), m_BackgroundColor(), m_HighlightColor(), m_TextColor()
{
    // HACK: we should set this from the outside
    SetScheme(CNextUI::GetInstance()->GetScheme());

    SetPaintEnabled(true);
    SetPaintBackgroundEnabled(true);
    SetPaintBorderEnabled(false);

    SetCommand(pszCommand);
}

void CGameMenuButton::ApplySchemeSettings(vgui::IScheme *pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);

    m_hFont = pScheme->GetFont("GameMenuButton", true);

    SetClickSound(pScheme->GetResourceString("GameMenu.Button.ClickSound"));
    SetRolloverSound(pScheme->GetResourceString("GameMenu.Button.RolloverSound"));

    m_flWidth = atof(pScheme->GetResourceString("GameMenu.Button.Width"));
    m_flHeight = atof(pScheme->GetResourceString("GameMenu.Button.Height"));
    m_flHighlightWidth_Normal = atof(pScheme->GetResourceString("GameMenu.Button.HighlightWidth.Normal"));
    m_flHighlightWidth_Highlight = atof(pScheme->GetResourceString("GameMenu.Button.HighlightWidth.Highlight"));
    m_flTextOffsetX_Normal = atof(pScheme->GetResourceString("GameMenu.Button.TextOffsetX.Normal"));
    m_flTextOffsetX_Highlight = atof(pScheme->GetResourceString("GameMenu.Button.TextOffsetX.Highlight"));
    m_flTextOffsetY_Normal = atof(pScheme->GetResourceString("GameMenu.Button.TextOffsetY.Normal"));
    m_flTextOffsetY_Highlight = atof(pScheme->GetResourceString("GameMenu.Button.TextOffsetY.Highlight"));

    m_BackgroundColor_Normal = GetSchemeColor("GameMenu.Button.BackgroundColor.Normal", pScheme);
    m_BackgroundColor_Highlight = GetSchemeColor("GameMenu.Button.BackgroundColor.Highlight", pScheme);
    m_HighlightColor_Normal = GetSchemeColor("GameMenu.Button.HighlightColor.Normal", pScheme);
    m_HighlightColor_Highlight = GetSchemeColor("GameMenu.Button.HighlightColor.Highlight", pScheme);
    m_TextColor_Normal = GetSchemeColor("GameMenu.Button.TextColor.Normal", pScheme);
    m_TextColor_Highlight = GetSchemeColor("GameMenu.Button.TextColor.Highlight", pScheme);

    m_flHighlightWidth = m_flHighlightWidth_Normal;
    m_flTextOffsetX = m_flTextOffsetX_Normal;
    m_flTextOffsetY = m_flTextOffsetY_Normal;
    m_BackgroundColor = m_BackgroundColor_Normal;
    m_HighlightColor = m_HighlightColor_Normal;
    m_TextColor = m_TextColor_Normal;

    m_flResWidth = XRES(m_flWidth);
    m_flResHeight = YRES(m_flHeight);
    SetSize(m_flResWidth, m_flResHeight);

    // After scheme reloading we should re-animate the panel
    // so the animation controller re-calculates the correct
    // values for whatever stuff that has been stuck after
    // video mode change (eg. proportional offsets).
    m_bLastHighlight = !m_bHighlight;
}

void CGameMenuButton::OnThink()
{
    BaseClass::OnThink();

    if(m_bLastHighlight != m_bHighlight) {
        vgui::AnimationController *pAC = CNextUI::GetInstance()->GetAnimationController();
        m_bLastHighlight = m_bHighlight;
        if(m_bLastHighlight) {
            pAC->RunAnimationCommand(this, "HighlightWidth", m_flHighlightWidth_Highlight, 0.0f, 0.25f, pAC->INTERPOLATOR_LINEAR);
            pAC->RunAnimationCommand(this, "TextOffsetX", m_flTextOffsetX_Highlight, 0.0f, 0.25f, pAC->INTERPOLATOR_LINEAR);
            pAC->RunAnimationCommand(this, "TextOffsetY", m_flTextOffsetY_Highlight, 0.0f, 0.25f, pAC->INTERPOLATOR_LINEAR);
            pAC->RunAnimationCommand(this, "BackgroundColor", m_BackgroundColor_Highlight, 0.0f, 0.25f, pAC->INTERPOLATOR_LINEAR);
            pAC->RunAnimationCommand(this, "HighlightColor", m_HighlightColor_Highlight, 0.0f, 0.25f, pAC->INTERPOLATOR_LINEAR);
            pAC->RunAnimationCommand(this, "TextColor", m_TextColor_Highlight, 0.0f, 0.25f, pAC->INTERPOLATOR_LINEAR);
        }
        else {
            pAC->RunAnimationCommand(this, "HighlightWidth", m_flHighlightWidth_Normal, 0.0f, 0.25f, pAC->INTERPOLATOR_LINEAR);
            pAC->RunAnimationCommand(this, "TextOffsetX", m_flTextOffsetX_Normal, 0.0f, 0.25f, pAC->INTERPOLATOR_LINEAR);
            pAC->RunAnimationCommand(this, "TextOffsetY", m_flTextOffsetY_Normal, 0.0f, 0.25f, pAC->INTERPOLATOR_LINEAR);
            pAC->RunAnimationCommand(this, "BackgroundColor", m_BackgroundColor_Normal, 0.0f, 0.25f, pAC->INTERPOLATOR_LINEAR);
            pAC->RunAnimationCommand(this, "HighlightColor", m_HighlightColor_Normal, 0.0f, 0.25f, pAC->INTERPOLATOR_LINEAR);
            pAC->RunAnimationCommand(this, "TextColor", m_TextColor_Normal, 0.0f, 0.25f, pAC->INTERPOLATOR_LINEAR);
        }
    }
}

void CGameMenuButton::PaintBackground()
{
    // Draw background
    g_pVGuiSurface->DrawSetColor(m_BackgroundColor);
    g_pVGuiSurface->DrawFilledRect(0, 0, m_flResWidth, m_flResHeight);
}

void CGameMenuButton::Paint()
{
    // Draw highlight
    if(m_bHighlight) {
        g_pVGuiSurface->DrawSetColor(m_HighlightColor);
        g_pVGuiSurface->DrawFilledRect(0, 0, m_flHighlightWidth, m_flResHeight);
    }

    // Draw text
    const wchar_t *pszText = GetText();
    if(pszText[0]) {
        int iTextW, iTextH;
        g_pVGuiSurface->GetTextSize(m_hFont, pszText, iTextW, iTextH);
        g_pVGuiSurface->DrawSetTextColor(m_TextColor);
        g_pVGuiSurface->DrawSetTextFont(m_hFont);
        g_pVGuiSurface->DrawSetTextPos(m_flTextOffsetX, (m_flResHeight / 2.0f) - (iTextH / 2.0f) + m_flTextOffsetY);
        g_pVGuiSurface->DrawPrintText(pszText, Q_wcslen(pszText));
    }
}

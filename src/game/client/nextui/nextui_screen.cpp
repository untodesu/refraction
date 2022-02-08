#include "cbase.h"
#include "nextui_int.h"
#include "nextui_screen.h"
#include "vgui/ISurface.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CNextUIScreen::CNextUIScreen(vgui::Panel *pParent, const char *pszPanelName)
    : BaseClass(pParent, pszPanelName), m_szTitle(), m_bSchemeLoaded(false), m_bQueuedActivate(false)
{
}

void CNextUIScreen::ApplySchemeSettings(vgui::IScheme *pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);

    m_hTitleFont = pScheme->GetFont("ScreenTitle", true);
    m_TitleColor = GetSchemeColor("Screen.Title.Color", pScheme);
    m_flTitleX = XRES(atof(pScheme->GetResourceString("Screen.Title.PositionX")));
    m_flTitleY = YRES(atof(pScheme->GetResourceString("Screen.Title.PositionY")));

    // Resize ourselves so we always go fullscreen
    int iWide, iTall;
    g_pVGuiSurface->GetScreenSize(iWide, iTall);
    SetBounds(0, 0, iWide, iTall);

    if(m_bQueuedActivate) {
        m_bQueuedActivate = false;
        OnActivate();
    }

    m_bSchemeLoaded = true;
}

void CNextUIScreen::OnActivate()
{
}

void CNextUIScreen::Paint()
{
    if(m_szTitle[0]) {
        g_pVGuiSurface->DrawSetTextColor(m_TitleColor);
        g_pVGuiSurface->DrawSetTextFont(m_hTitleFont);
        g_pVGuiSurface->DrawSetTextPos(m_flTitleX, m_flTitleY);
        g_pVGuiSurface->DrawPrintText(m_szTitle, Q_wcslen(m_szTitle));
    }
}

void CNextUIScreen::SetTitle(const char *pszTitle)
{
    CNextUI::GetInstance()->GetLocalizedString(pszTitle, m_szTitle, sizeof(m_szTitle) / sizeof(wchar_t));
}

void CNextUIScreen::SetTitle(const wchar_t *pszTitle)
{
    Q_wcsncpy(m_szTitle, pszTitle, sizeof(m_szTitle));
}

void CNextUIScreen::Activate()
{
    if(!m_bSchemeLoaded) {
        m_bQueuedActivate = true;
        return;
    }

    OnActivate();
}

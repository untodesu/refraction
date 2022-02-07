#include "cbase.h"
#include "nextui_screen.h"
#include "vgui/ISurface.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CNextUIScreen::CNextUIScreen(vgui::Panel *pParent, const char *pszPanelName)
    : BaseClass(pParent, pszPanelName)
{
}

void CNextUIScreen::ApplySchemeSettings(vgui::IScheme *pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);

    // Resize ourselves so we always go fullscreen
    int iWide, iTall;
    g_pVGuiSurface->GetScreenSize(iWide, iTall);
    SetBounds(0, 0, iWide, iTall);
}

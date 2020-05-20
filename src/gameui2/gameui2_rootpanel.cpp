#include "vgui/ISurface.h"
#include "gameui2_rootpanel.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CGameUI2RootPanel::CGameUI2RootPanel( vgui::VPANEL parent ) : BaseClass( NULL, "GameUI2_RootPanel" )
{
    SetParent( parent );
}

void CGameUI2RootPanel::ApplySchemeSettings( vgui::IScheme *pScheme )
{
    BaseClass::ApplySchemeSettings( pScheme );

    // Resize the panel to the screen size
    // Otherwise, it'll just be in a little corner
    int wide, tall;
    g_pVGuiSurface->GetScreenSize( wide, tall );
    SetSize( wide, tall );
}

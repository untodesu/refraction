#include "cbase.h"
#include "nextui_rootpanel.h"
#include "vgui/ISurface.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CNextUIRootPanel::CNextUIRootPanel(vgui::VPANEL hParent)
    : BaseClass(NULL, "NextUI")
{
    SetParent(hParent);
    SetPaintEnabled(false);
    SetPaintBackgroundEnabled(false);
    SetPaintBorderEnabled(false);
    SetProportional(false);
    SetPostChildPaintEnabled(true);
    SetVisible(false);
}

#include "vgui/ILocalize.h"
#include "gameui2_api.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

void GameUI2_GetLocalizedString( const char *pszSource, wchar_t *szDestination, int dstChars )
{
    const wchar_t *pszValue = g_pVGuiLocalize->Find( pszSource );
    if( pszValue ) {
        Q_wcsncpy( szDestination, pszValue, sizeof( wchar_t ) * dstChars );
        return;
    }

    mbstowcs( szDestination, pszSource, dstChars );
}

CBaseUIElement::CBaseUIElement( const char *pszElementName ) : BaseClass( NULL, pszElementName )
{

}

void CBaseUIElement::Initialize()
{

}

void CBaseUIElement::Shutdown()
{

}

void CBaseUIElement::SetPropBounds( int xpos, int ypos, int wide, int tall )
{
    xpos = g_pVGuiSchemeManager->GetProportionalScaledValue( xpos );
    ypos = g_pVGuiSchemeManager->GetProportionalScaledValue( ypos );
    wide = g_pVGuiSchemeManager->GetProportionalScaledValue( wide );
    tall = g_pVGuiSchemeManager->GetProportionalScaledValue( tall );
    SetBounds( xpos, ypos, wide, tall );
}

void CBaseUIElement::SetPropSize( int wide, int tall )
{
    wide = g_pVGuiSchemeManager->GetProportionalScaledValue( wide );
    tall = g_pVGuiSchemeManager->GetProportionalScaledValue( tall );
    SetSize( wide, tall );
}

void CBaseUIElement::SetPropPos( int xpos, int ypos )
{
    xpos = g_pVGuiSchemeManager->GetProportionalScaledValue( xpos );
    ypos = g_pVGuiSchemeManager->GetProportionalScaledValue( ypos );
    SetPos( xpos, ypos );
}

CUIElementReg * CUIElementReg::s_pElements = NULL;

CUIElementReg::CUIElementReg( CreateElementFn pCreateFn, DeleteElementFn pDeleteFn ) : m_pCreateFn( pCreateFn ), m_pDeleteFn( pDeleteFn )
{
    m_pNext = s_pElements;
    s_pElements = this;
}

void CUIElementReg::Create()
{
    if( !m_pElement ) {
        m_pCreateFn( &m_pElement );
    }
}

void CUIElementReg::Delete()
{
    if( m_pElement ) {
        m_pElement->SetParent( ( vgui::VPANEL )0 );
        m_pDeleteFn( &m_pElement );
    }
}


#include "cbase.h"
#include "engine/IEngineSound.h"
#include "vgui/ISurface.h"
#include "nextui_button.h"
#include "nextui_int.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


CNextUIButton::CNextUIButton(vgui::Panel *pParent, const char *pszPanelName, const char *pszText)
    : BaseClass(pParent, pszPanelName), m_szCommand(), m_szRolloverSound(), m_szClickSound(), m_szText()
{
    SetText(pszText);
}

CNextUIButton::CNextUIButton(vgui::Panel *pParent, const char *pszPanelName, const wchar_t *pszText)
    : BaseClass(pParent, pszPanelName), m_szCommand(), m_szRolloverSound(), m_szClickSound(), m_szText()
{
    SetText(pszText);
}

void CNextUIButton::OnCursorEntered()
{
    BaseClass::OnCursorEntered();
    g_pVGuiSurface->PlaySound(m_szRolloverSound);
    m_bHighlight = true;
}

void CNextUIButton::OnCursorExited()
{
    BaseClass::OnCursorExited();
    m_bHighlight = false;
}

void CNextUIButton::OnMousePressed(vgui::MouseCode eCode)
{
    if(eCode == MOUSE_LEFT && m_bHighlight) {
        g_pVGuiSurface->PlaySound(m_szClickSound);
        CNextUI::GetGameUI()->SendMainMenuCommand(m_szCommand);
    }
}

void CNextUIButton::OnThink()
{
    BaseClass::OnThink();

    if(HasFocus()) {
        if(!IsCursorOver()) {
            m_bHighlight = false;
        }
        else if(!m_bHighlight) {
            m_bHighlight = true;
        }
    }
    else if(!IsCursorOver()) {
        m_bHighlight = false;
    }
}

void CNextUIButton::SetCommand(const char *pszCommand)
{
    Q_strncpy(m_szCommand, pszCommand, sizeof(m_szCommand));
}

void CNextUIButton::SetRolloverSound(const char *pszSound)
{
    if(pszSound && pszSound[0]) {
        Q_strncpy(m_szRolloverSound, pszSound, sizeof(m_szRolloverSound));
        enginesound->PrecacheSound(m_szRolloverSound, true, true);
    }
}

void CNextUIButton::SetClickSound(const char *pszSound)
{
    if(pszSound && pszSound[0]) {
        Q_strncpy(m_szClickSound, pszSound, sizeof(m_szClickSound));
        enginesound->PrecacheSound(m_szClickSound, true, true);
    }
}

void CNextUIButton::SetText(const wchar_t *pszText)
{
    Q_wcsncpy(m_szText, pszText, sizeof(m_szText));
}

void CNextUIButton::SetText(const char *pszText)
{
    CNextUI::GetInstance()->GetLocalizedString(pszText, m_szText, sizeof(m_szText) / sizeof(wchar_t));
}

const wchar_t *CNextUIButton::GetText() const
{
    return m_szText;
}

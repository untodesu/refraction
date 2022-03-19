#ifndef _NEXTUI_BUTTON_H_
#define _NEXTUI_BUTTON_H_ 1
#pragma once
#include "vgui_controls/Panel.h"

#define NEXTUI_BUTTON_NORMAL            0
#define NEXTUI_BUTTON_HIGHLIGHT         1
#define NEXTUI_BUTTON_HIGHLIGHT_STICKY  2

class CNextUIButton : public vgui::Panel {
    DECLARE_CLASS_SIMPLE(CNextUIButton, vgui::Panel);

public:
    CNextUIButton(vgui::Panel *pParent, const char *pszPanelName, const char *pszText);
    CNextUIButton(vgui::Panel *pParent, const char *pszPanelName, const wchar_t *pszText);
    virtual void OnCursorEntered();
    virtual void OnCursorExited();
    virtual void OnMousePressed(vgui::MouseCode eCode);
    virtual void OnMouseDoublePressed(vgui::MouseCode eCode);
    virtual void OnStickyHighlight();
    virtual void OnThink();
    virtual void OnClick();
    void SetSticky(bool bSticky);
    void SetCommand(const char *pszCommand);
    void SetRolloverSound(const char *pszSound);
    void SetClickSound(const char *pszSound);
    void SetText(const wchar_t *pszText);
    void SetText(const char *pszText);
    const wchar_t *GetText() const;

public:
    bool m_bHighlight = true;

private:
    bool m_bSticky = false;
    char m_szCommand[512];
    char m_szRolloverSound[1024];
    char m_szClickSound[1024];
    wchar_t m_szText[512];
};

#endif

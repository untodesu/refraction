#ifndef GAMEUI2_API_H
#define GAMEUI2_API_H
#ifdef _WIN32
#pragma once
#endif

#include "vgui_controls/Panel.h"
#include "gameui2_interface.h"

void GameUI2_GetLocalizedString( const char *pszSource, wchar_t *szDestination, int dstChars );

// Base UI element.
class CBaseUIElement : public vgui::Panel {
    DECLARE_CLASS_SIMPLE( CBaseUIElement, vgui::Panel );

public:
    CBaseUIElement(const char *pszElementName);
    virtual void Initialize();
    virtual void Shutdown();

    void SetPropBounds( int xpos, int ypos, int wide, int tall );
    void SetPropSize( int wide, int tall );
    void SetPropPos( int xpos, int ypos );
};

// Used to internally register UI elements.
class CUIElementReg {
public:
    typedef void( *CreateElementFn )( CBaseUIElement ** );
    typedef void( *DeleteElementFn )( CBaseUIElement ** );

public:
    static CUIElementReg *s_pElements;
    CUIElementReg *m_pNext;

    CreateElementFn m_pCreateFn;
    DeleteElementFn m_pDeleteFn;
    CBaseUIElement *m_pElement = NULL;

public:
    CUIElementReg( CreateElementFn pCreateFn, DeleteElementFn pDeleteFn );
    void Create();
    void Delete();
};

// Declares UI element...
#define REGISTER_UI_ELEMENT(className)                          \
    static void className##_Create(CBaseUIElement **pElement)   \
    {                                                           \
        *pElement = (CBaseUIElement *)(new className);          \
    }                                                           \
    static void className##_Delete(CBaseUIElement **pElement)   \
    {                                                           \
        delete *pElement;                                       \
        *pElement = nullptr;                                    \
    }                                                           \
    static CUIElementReg s_##className##_reg(className##_Create, className##_Delete)

#endif

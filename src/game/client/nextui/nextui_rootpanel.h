#ifndef _NEXTUI_ROOTPANEL_H_
#define _NEXTUI_ROOTPANEL_H_ 1
#pragma once
#include "nextui_screen.h"

class CNextUIRootPanel : public CNextUIScreen {
    DECLARE_CLASS_SIMPLE(CNextUIRootPanel, CNextUIScreen);

public:
    CNextUIRootPanel(vgui::VPANEL hParent);
};

#endif

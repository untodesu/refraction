#ifndef _NEXTUI_INT_H_
#define _NEXTUI_INT_H_ 1
#pragma once
#include "GameUI/IGameUI.h"
#include "INextUI.h"
#include "nextui_rootpanel.h"
#include "vgui_controls/AnimationController.h"

class CNextUI : public CBaseGameSystemPerFrame {
public:
    const char *Name() override;
    bool Init() override;
    void PostInit() override;
    void Shutdown() override;
    void VidInit() override;
    void Update(float frametime) override;
    vgui::HScheme GetScheme() const;
    vgui::AnimationController *GetAnimationController() const;
    void GetLocalizedString(const char *pszSource, wchar_t *pszDest, int iDestChars) const;

private:
    vgui::HScheme m_hScheme = 0;
    CNextUIRootPanel *m_pRootPanel = NULL;
    vgui::AnimationController *m_pAnimationController = NULL;

public:
    static IGameUI *GetGameUI();
    static CNextUI *GetInstance();
};

#endif

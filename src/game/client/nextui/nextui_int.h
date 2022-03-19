#ifndef _NEXTUI_INT_H_
#define _NEXTUI_INT_H_ 1
#pragma once
#include "GameUI/IGameUI.h"
#include "INextUI.h"
#include "elements/gamemenu.h"
#include "nextui_rootpanel.h"
#include "nextui_screen.h"
#include "vgui_controls/AnimationController.h"

struct SaveFileInfo {
    char m_szShortName[64];
    char m_szFilename[64];
    char m_szMapName[32];
    char m_szFileTime[32];
    char m_szComment[80];
    unsigned int m_uTimestamp;

    static bool ParseFile(const char *pszFilename, const char *pszShortName, SaveFileInfo &save);
    static void FindSaveSlot(char *pszBuffer, int iBufferSize);
    static int SortFunc(const void *p1, const void *p2);
};

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
    CNextUIRootPanel *GetRootPanel() const;
    void GetLocalizedString(const char *pszSource, wchar_t *pszDest, int iDestChars) const;
    void SetCurrentScreen(CNextUIScreen *pScreen);
    void UnsetCurrentScreen(CNextUIScreen *pScreen);
    CNextUIScreen *GetCurrentScreen();

private:
    vgui::HScheme m_hScheme = 0;
    CNextUIRootPanel *m_pRootPanel = NULL;
    vgui::AnimationController *m_pAnimationController = NULL;
    CNextUIScreen *m_pCurrentScreen = NULL;
    CNextUIScreen *m_pPreviousScreen = NULL;
    CGameMenu *m_pGameMenuScreen = NULL;

public:
    static IGameUI *GetGameUI();
    static CNextUI *GetInstance();
};

#endif

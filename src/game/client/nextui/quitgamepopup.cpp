#include "cbase.h"
#include "filesystem.h"
#include "nextui_int.h"
#include "nextui_popup.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// This is literally what GameUI.dll does lmao
static void GameUI_FindSaveSlot(char *pszBuffer, int iBufferSize)
{
    static char szFilename[1024] = { 0 };

    pszBuffer[0] = 0;
    for(int i = 0; i < 1000; i++) {
        Q_snprintf(szFilename, sizeof(szFilename), "save/half-life-%03d.sav", i);

        FileHandle_t fp = g_pFullFileSystem->Open(szFilename, "rb");
        if(!fp) {
            Q_strncpy(pszBuffer, szFilename + 5, iBufferSize);
            char *pszExtension = Q_strstr(pszBuffer, ".sav");
            if(pszExtension)
                pszExtension[0] = 0;
            return;
        }

        g_pFullFileSystem->Close(fp);
    }

    ConColorMsg(LOG_COLOR_B_RED, "NextUI: unable to generate a new save file");
    Assert(0);
}

class CNextUIPopupButton_Quicksave : public CNextUIPopupButton {
    DECLARE_CLASS_SIMPLE(CNextUIPopupButton_Quicksave, CNextUIPopupButton);

public:
    CNextUIPopupButton_Quicksave(CNextUIPopup *pParent, const char *pszTitle)
        : BaseClass(pParent, pszTitle)
    {

    }

    void OnClick() override
    {
        static char szSaveName[32] = { 0 }, szCommand[128] = { 0 };;
        GameUI_FindSaveSlot(szSaveName, sizeof(szSaveName));
        Q_snprintf(szCommand, sizeof(szCommand), "save %s", szSaveName);
        engine->ClientCmd_Unrestricted(szCommand);
        BaseClass::OnClick();
    }
};

class CNextUIPopupButton_UnsetParentScreen : public CNextUIPopupButton {
    DECLARE_CLASS_SIMPLE(CNextUIPopupButton_UnsetParentScreen, CNextUIPopupButton);

public:
    CNextUIPopupButton_UnsetParentScreen(CNextUIPopup *pParent, const char *pszTitle)
        : BaseClass(pParent, pszTitle), m_pParent(pParent)
    {

    }

    void OnClick() override
    {
        CNextUI::GetInstance()->UnsetCurrentScreen(m_pParent);
        BaseClass::OnClick();
    }

private:
    CNextUIPopup *m_pParent;
};

static CNextUIPopup *s_pQuitGamePopup = NULL;
static CNextUIPopup *s_pQuitGamePopupInGame = NULL;

CON_COMMAND(nextui_quitgame, "QuitGameDialog[InGame]")
{
    CNextUI *pUI = CNextUI::GetInstance();

    if(engine->IsInGame() && !engine->IsLevelMainMenuBackground()) {
        if(!s_pQuitGamePopupInGame) {
            s_pQuitGamePopupInGame = new CNextUIPopup(pUI->GetRootPanel(), "QuitGameDialogInGame");
            s_pQuitGamePopupInGame->SetText("#GameUI_SaveAndQuitQuery_Info");

            // Save and quit
            CNextUIPopupButton_Quicksave *pButton_1 = new CNextUIPopupButton_Quicksave(s_pQuitGamePopupInGame, "#GameUI_SaveAndQuit");
            pButton_1->SetCommand("QuitNoConfirm");
            s_pQuitGamePopupInGame->AddButton(pButton_1);

            // Don't save and quit
            CNextUIPopupButton *pButton_2 = new CNextUIPopupButton(s_pQuitGamePopupInGame, "#GameUI_DontSaveAndQuit");
            pButton_2->SetCommand("QuitNoConfirm");
            s_pQuitGamePopupInGame->AddButton(pButton_2);

            // Cancel
            CNextUIPopupButton_UnsetParentScreen *pButton_3 = new CNextUIPopupButton_UnsetParentScreen(s_pQuitGamePopupInGame, "#GameUI_Cancel");
            s_pQuitGamePopupInGame->AddButton(pButton_3);
        }
        
        pUI->SetCurrentScreen(s_pQuitGamePopupInGame);
    }
    else {
        if(!s_pQuitGamePopup) {
            s_pQuitGamePopup = new CNextUIPopup(pUI->GetRootPanel(), "QuitGameDialog");
            s_pQuitGamePopup->SetText("#GameUI_QuitConfirmationText");

            // Quit game
            CNextUIPopupButton *pButton_1 = new CNextUIPopupButton(s_pQuitGamePopup, "#GameUI_Quit");
            pButton_1->SetCommand("QuitNoConfirm");
            s_pQuitGamePopup->AddButton(pButton_1);

            // Cancel
            CNextUIPopupButton_UnsetParentScreen *pButton_2 = new CNextUIPopupButton_UnsetParentScreen(s_pQuitGamePopup, "#GameUI_Cancel");
            s_pQuitGamePopup->AddButton(pButton_2);
        }

        pUI->SetCurrentScreen(s_pQuitGamePopup);
    }
}

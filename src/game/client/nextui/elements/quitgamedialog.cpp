#include "cbase.h"
#include "filesystem.h"
//#include "saveloaddialog.h"
#include "nextui/nextui_dialog.h"
#include "nextui/nextui_int.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CNextUIDialogButton_Quicksave : public CNextUIDialogButton {
    DECLARE_CLASS_SIMPLE(CNextUIDialogButton_Quicksave, CNextUIDialogButton);

public:
    CNextUIDialogButton_Quicksave(CNextUIDialog *pParent, const char *pszTitle)
        : BaseClass(pParent, pszTitle)
    {

    }

    void OnClick() override
    {
        static char szSaveName[32] = { 0 }, szCommand[128] = { 0 };;
        SaveFileInfo::FindSaveSlot(szSaveName, sizeof(szSaveName));
        Q_snprintf(szCommand, sizeof(szCommand), "save %s", szSaveName);
        engine->ClientCmd_Unrestricted(szCommand);
        BaseClass::OnClick();
    }
};

static CNextUIDialog *s_pQuitGamePopup = NULL;
static CNextUIDialog *s_pQuitGamePopupInGame = NULL;

CON_COMMAND(nextui_quitgame, "QuitGameDialog[InGame]")
{
    CNextUI *pUI = CNextUI::GetInstance();

    if(engine->IsInGame() && !engine->IsLevelMainMenuBackground()) {
        if(!s_pQuitGamePopupInGame) {
            s_pQuitGamePopupInGame = new CNextUIDialog(pUI->GetRootPanel(), "QuitGameDialogInGame");
            s_pQuitGamePopupInGame->SetText("#GameUI_SaveAndQuitQuery_Info");
            s_pQuitGamePopupInGame->SetTitle("#GameUI_QuitConfirmationTitle");

            // Save and quit
            CNextUIDialogButton_Quicksave *pButton_1 = new CNextUIDialogButton_Quicksave(s_pQuitGamePopupInGame, "#GameUI_SaveAndQuit");
            pButton_1->SetCommand("QuitNoConfirm");
            s_pQuitGamePopupInGame->AddButton(pButton_1);

            // Don't save and quit
            CNextUIDialogButton *pButton_2 = new CNextUIDialogButton(s_pQuitGamePopupInGame, "#GameUI_DontSaveAndQuit");
            pButton_2->SetCommand("QuitNoConfirm");
            s_pQuitGamePopupInGame->AddButton(pButton_2);

            // Cancel
            CNextUIDialogButton_CloseParent *pButton_3 = new CNextUIDialogButton_CloseParent(s_pQuitGamePopupInGame, "#GameUI_Cancel");
            s_pQuitGamePopupInGame->AddButton(pButton_3);
        }
        
        pUI->SetCurrentScreen(s_pQuitGamePopupInGame);
    }
    else {
        if(!s_pQuitGamePopup) {
            s_pQuitGamePopup = new CNextUIDialog(pUI->GetRootPanel(), "QuitGameDialog");
            s_pQuitGamePopup->SetText("#GameUI_QuitConfirmationText");
            s_pQuitGamePopup->SetTitle("#GameUI_QuitConfirmationTitle");

            // Quit game
            CNextUIDialogButton *pButton_1 = new CNextUIDialogButton(s_pQuitGamePopup, "#GameUI_Quit");
            pButton_1->SetCommand("QuitNoConfirm");
            s_pQuitGamePopup->AddButton(pButton_1);

            // Cancel
            CNextUIDialogButton_CloseParent *pButton_2 = new CNextUIDialogButton_CloseParent(s_pQuitGamePopup, "#GameUI_Cancel");
            s_pQuitGamePopup->AddButton(pButton_2);
        }

        pUI->SetCurrentScreen(s_pQuitGamePopup);
    }
}

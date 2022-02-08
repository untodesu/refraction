MenuLayout {
    #GameUI_GameMenu_ResumeGame {
        command "ResumeGame"
        in_game "1"
        in_menu "0"
    }

    #GameUI_GameMenu_Disconnect {
        command "Disconnect"
        in_game "1"
        in_menu "0"
    }

    blank {
        blank   "1"
        in_game "1"
        in_menu "0"
    }

    #GameUI_GameMenu_NewGame {
        command "OpenNewGameDialog"
        in_game "1"
        in_menu "1"
    }

    #GameUI_GameMenu_LoadGame {
        command "OpenLoadGameDialog"
        in_game "1"
        in_menu "1"
    }

    #GameUI_GameMenu_SaveGame {
        command "OpenSaveGameDialog"
        in_game "1"
        in_menu "1"
    }

    blank {
        blank   "1"
        in_game "1"
        in_menu "1"
    }
    
    #GameUI_GameMenu_Options {
        command "OpenOptionsDialog"
        in_game "1"
        in_menu "1"
    }

    #GameUI_GameMenu_Quit {
        command "engine nextui_quitgame"
        in_game "1"
        in_menu "1"
    }
}

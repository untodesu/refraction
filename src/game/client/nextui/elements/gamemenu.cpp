#include "cbase.h"
#include "vgui/ISurface.h"
#include "gamemenu.h"
#include "nextui/nextui_int.h"
#include "tier1/KeyValues.h"
#include "filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CGameMenu::CGameMenu(vgui::Panel *pParent)
    : BaseClass(pParent, "GameMenu"), m_szTitleText()
{
    // HACK: we should set this from the outside
    SetScheme(CNextUI::GetInstance()->GetScheme());

    SetPaintBackgroundEnabled(true);
    SetPaintBorderEnabled(false);
    SetPaintEnabled(true);

    m_bWasInGame = engine->IsInGame();
}

void CGameMenu::ApplySchemeSettings(vgui::IScheme *pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);

    m_hTitleFont = pScheme->GetFont("GameMenuTitle", true);

    m_FadeColor = GetSchemeColor("GameMenu.Background.FadeColor", pScheme);
    m_TitleColor = GetSchemeColor("GameMenu.Title.Color", pScheme);

    m_flGradientWidthPercent = atof(pScheme->GetResourceString("GameMenu.Background.GradientWidthPercent")) / 100.0f;
    m_flButtonStackX = XRES(atof(pScheme->GetResourceString("GameMenu.ButtonStack.PositionX")));
    m_flButtonStackY = YRES(atof(pScheme->GetResourceString("GameMenu.ButtonStack.PositionY")));
    m_flTitleX = XRES(atof(pScheme->GetResourceString("GameMenu.Title.PositionX")));
    m_flTitleY = YRES(atof(pScheme->GetResourceString("GameMenu.Title.PositionY")));

    m_iButtonHeight = YRES(atof(pScheme->GetResourceString("GameMenu.Button.Height")));

    g_pVGuiSurface->GetScreenSize(m_iScreenW, m_iScreenH);

    KeyValues *pKV = new KeyValues("GameInfo");
    pKV->LoadFromFile(filesystem, "gameinfo.txt", "GAME");
    Q_wcsncpy(m_szTitleText, pKV->GetWString("nextui_title", L"NextUI"), sizeof(m_szTitleText));
    pKV->deleteThis();

    Rearrange();
}

void CGameMenu::Paint()
{
    g_pVGuiSurface->DrawSetTextColor(m_TitleColor);
    g_pVGuiSurface->DrawSetTextFont(m_hTitleFont);
    g_pVGuiSurface->DrawSetTextPos(m_flTitleX, m_flTitleY);
    g_pVGuiSurface->DrawPrintText(m_szTitleText, Q_wcslen(m_szTitleText));
}

void CGameMenu::PaintBackground()
{
    g_pVGuiSurface->DrawSetColor(m_FadeColor);
    g_pVGuiSurface->DrawFilledRectFade(0, 0, m_iScreenW * m_flGradientWidthPercent, m_iScreenH, 0xFF, 0x00, true);
}

void CGameMenu::OnThink()
{
    const bool bInGame = engine->IsInGame() && !engine->IsLevelMainMenuBackground();
    if(bInGame != m_bWasInGame) {
        m_bWasInGame = bInGame;
        Rearrange();
    }
}

void CGameMenu::Rearrange()
{
    RemoveAll();

    int iButtonYPos = m_flButtonStackY;

    KeyValues *pKV = new KeyValues("GameMenu");
    pKV->LoadFromFile(filesystem, "resource/gamemenulayout.res", "GAME");
    for(KeyValues *pSub = pKV->GetFirstSubKey(); pSub; pSub = pSub->GetNextTrueSubKey()) {
        const char *pszButtonTitle = pSub->GetName();
        const char *pszButtonCommand = pSub->GetString("command", "");
        bool bIsBlank = pSub->GetBool("blank", false);
        bool bShowInGame = pSub->GetBool("in_game", true);
        bool bShowInMenu = pSub->GetBool("in_menu", true);

        if(m_bWasInGame && !bShowInGame) {
            continue;
        }

        if(!m_bWasInGame && !bShowInMenu) {
            continue;
        }
        
        if(bIsBlank) {
            iButtonYPos += m_iButtonHeight / 2 * Max(1, pSub->GetInt("gap", 1));
            continue;
        }

        CGameMenuButton *pButton = new CGameMenuButton(this, pszButtonCommand);
        pButton->SetText(pszButtonTitle);
        pButton->SetPos(m_flButtonStackX, iButtonYPos);

        iButtonYPos += m_iButtonHeight;
        m_vecButtons.AddToTail(pButton);
    }

    pKV->deleteThis();
}

void CGameMenu::RemoveAll()
{
    for(int i = 0; i < m_vecButtons.Count(); i++) {
        if(m_vecButtons[i]) {
            m_vecButtons[i]->SetParent((vgui::VPANEL)0);
            delete m_vecButtons[i];
        }
    }

    m_vecButtons.RemoveAll();
}

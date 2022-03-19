#include "cbase.h"
#include "vgui/ISurface.h"
#include "gamemenu.h"
#include "nextui/nextui_button.h"
#include "nextui/nextui_int.h"
#include "tier1/KeyValues.h"
#include "filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CGameMenuButton : public CNextUIButton {
    DECLARE_CLASS_SIMPLE(CGameMenuButton, CNextUIButton);

public:
    CGameMenuButton(vgui::Panel *pParent, const char *pszCommand = NULL)
        : BaseClass(pParent, "", ""), m_flHighlightWidth(), m_flTextOffsetX(), m_flTextOffsetY(), m_BackgroundColor(), m_HighlightColor(), m_TextColor()
    {
        // HACK: we should set this from the outside
        SetScheme(CNextUI::GetInstance()->GetScheme());

        SetPaintEnabled(true);
        SetPaintBackgroundEnabled(true);
        SetPaintBorderEnabled(false);

        SetCommand(pszCommand);
    }

    void ApplySchemeSettings(vgui::IScheme *pScheme) override
    {
        BaseClass::ApplySchemeSettings(pScheme);

        m_hFont = pScheme->GetFont("GameMenuButton", true);

        SetClickSound(pScheme->GetResourceString("GameMenu.Button.ClickSound"));
        SetRolloverSound(pScheme->GetResourceString("GameMenu.Button.RolloverSound"));

        m_flWidth = atof(pScheme->GetResourceString("GameMenu.Button.Width"));
        m_flHeight = atof(pScheme->GetResourceString("GameMenu.Button.Height"));
        m_flHighlightWidth_Normal = atof(pScheme->GetResourceString("GameMenu.Button.HighlightWidth.Normal"));
        m_flHighlightWidth_Highlight = atof(pScheme->GetResourceString("GameMenu.Button.HighlightWidth.Highlight"));
        m_flTextOffsetX_Normal = atof(pScheme->GetResourceString("GameMenu.Button.TextOffsetX.Normal"));
        m_flTextOffsetX_Highlight = atof(pScheme->GetResourceString("GameMenu.Button.TextOffsetX.Highlight"));
        m_flTextOffsetY_Normal = atof(pScheme->GetResourceString("GameMenu.Button.TextOffsetY.Normal"));
        m_flTextOffsetY_Highlight = atof(pScheme->GetResourceString("GameMenu.Button.TextOffsetY.Highlight"));

        m_BackgroundColor_Normal = GetSchemeColor("GameMenu.Button.BackgroundColor.Normal", pScheme);
        m_BackgroundColor_Highlight = GetSchemeColor("GameMenu.Button.BackgroundColor.Highlight", pScheme);
        m_HighlightColor_Normal = GetSchemeColor("GameMenu.Button.HighlightColor.Normal", pScheme);
        m_HighlightColor_Highlight = GetSchemeColor("GameMenu.Button.HighlightColor.Highlight", pScheme);
        m_TextColor_Normal = GetSchemeColor("GameMenu.Button.TextColor.Normal", pScheme);
        m_TextColor_Highlight = GetSchemeColor("GameMenu.Button.TextColor.Highlight", pScheme);

        m_flHighlightWidth = m_flHighlightWidth_Normal;
        m_flTextOffsetX = m_flTextOffsetX_Normal;
        m_flTextOffsetY = m_flTextOffsetY_Normal;
        m_BackgroundColor = m_BackgroundColor_Normal;
        m_HighlightColor = m_HighlightColor_Normal;
        m_TextColor = m_TextColor_Normal;

        m_flResWidth = XRES(m_flWidth);
        m_flResHeight = YRES(m_flHeight);
        SetSize(m_flResWidth, m_flResHeight);

        // After scheme reloading we should re-animate the panel
        // so the animation controller re-calculates the correct
        // values for whatever stuff that has been stuck after
        // video mode change (eg. proportional offsets).
        m_bLastHighlight = !m_bHighlight;
    }

    void OnThink() override
    {
        BaseClass::OnThink();

        if(m_bLastHighlight != m_bHighlight) {
            vgui::AnimationController *pAC = CNextUI::GetInstance()->GetAnimationController();
            m_bLastHighlight = m_bHighlight;
            if(m_bLastHighlight) {
                pAC->RunAnimationCommand(this, "HighlightWidth", m_flHighlightWidth_Highlight, 0.0f, 0.25f, pAC->INTERPOLATOR_LINEAR);
                pAC->RunAnimationCommand(this, "TextOffsetX", m_flTextOffsetX_Highlight, 0.0f, 0.25f, pAC->INTERPOLATOR_LINEAR);
                pAC->RunAnimationCommand(this, "TextOffsetY", m_flTextOffsetY_Highlight, 0.0f, 0.25f, pAC->INTERPOLATOR_LINEAR);
                pAC->RunAnimationCommand(this, "BackgroundColor", m_BackgroundColor_Highlight, 0.0f, 0.25f, pAC->INTERPOLATOR_LINEAR);
                pAC->RunAnimationCommand(this, "HighlightColor", m_HighlightColor_Highlight, 0.0f, 0.25f, pAC->INTERPOLATOR_LINEAR);
                pAC->RunAnimationCommand(this, "TextColor", m_TextColor_Highlight, 0.0f, 0.25f, pAC->INTERPOLATOR_LINEAR);
            }
            else {
                pAC->RunAnimationCommand(this, "HighlightWidth", m_flHighlightWidth_Normal, 0.0f, 0.25f, pAC->INTERPOLATOR_LINEAR);
                pAC->RunAnimationCommand(this, "TextOffsetX", m_flTextOffsetX_Normal, 0.0f, 0.25f, pAC->INTERPOLATOR_LINEAR);
                pAC->RunAnimationCommand(this, "TextOffsetY", m_flTextOffsetY_Normal, 0.0f, 0.25f, pAC->INTERPOLATOR_LINEAR);
                pAC->RunAnimationCommand(this, "BackgroundColor", m_BackgroundColor_Normal, 0.0f, 0.25f, pAC->INTERPOLATOR_LINEAR);
                pAC->RunAnimationCommand(this, "HighlightColor", m_HighlightColor_Normal, 0.0f, 0.25f, pAC->INTERPOLATOR_LINEAR);
                pAC->RunAnimationCommand(this, "TextColor", m_TextColor_Normal, 0.0f, 0.25f, pAC->INTERPOLATOR_LINEAR);
            }
        }
    }

    void PaintBackground() override
    {
        // Draw background
        g_pVGuiSurface->DrawSetColor(m_BackgroundColor);
        g_pVGuiSurface->DrawFilledRect(0, 0, m_flResWidth, m_flResHeight);
    }

    void Paint() override
    {
        // Draw highlight
        if(m_bHighlight) {
            g_pVGuiSurface->DrawSetColor(m_HighlightColor);
            g_pVGuiSurface->DrawFilledRect(0, 0, m_flHighlightWidth, m_flResHeight);
        }

        // Draw text
        const wchar_t *pszText = GetText();
        if(pszText[0]) {
            int iTextW, iTextH;
            g_pVGuiSurface->GetTextSize(m_hFont, pszText, iTextW, iTextH);
            g_pVGuiSurface->DrawSetTextColor(m_TextColor);
            g_pVGuiSurface->DrawSetTextFont(m_hFont);
            g_pVGuiSurface->DrawSetTextPos(m_flTextOffsetX, (m_flResHeight / 2.0f) - (iTextH / 2.0f) + m_flTextOffsetY);
            g_pVGuiSurface->DrawPrintText(pszText, Q_wcslen(pszText));
        }
    }

private:
    bool m_bLastHighlight = false;
    float m_flResWidth = 0.0f;
    float m_flResHeight = 0.0f;

private:
    CPanelAnimationVarAliasType(float, m_flHighlightWidth, "HighlightWidth", "0", "proportional_float");
    CPanelAnimationVarAliasType(float, m_flTextOffsetX, "TextOffsetX", "0", "proportional_float");
    CPanelAnimationVarAliasType(float, m_flTextOffsetY, "TextOffsetY", "0", "proportional_float");
    CPanelAnimationVar(Color, m_BackgroundColor, "BackgroundColor", "0 0 0 0");
    CPanelAnimationVar(Color, m_HighlightColor, "HighlightColor", "0 0 0 0");
    CPanelAnimationVar(Color, m_TextColor, "TextColor", "0 0 0 0");

private:
    vgui::HFont m_hFont = 0;
    float m_flWidth = 0.0f;
    float m_flHeight = 0.0f;
    float m_flHighlightWidth_Normal = 0.0f;
    float m_flHighlightWidth_Highlight = 0.0f;
    float m_flTextOffsetX_Normal = 0.0f;
    float m_flTextOffsetX_Highlight = 0.0f;
    float m_flTextOffsetY_Normal = 0.0f;
    float m_flTextOffsetY_Highlight = 0.0f;
    Color m_BackgroundColor_Normal = Color();
    Color m_BackgroundColor_Highlight = Color();
    Color m_HighlightColor_Normal = Color();
    Color m_HighlightColor_Highlight = Color();
    Color m_TextColor_Normal = Color();
    Color m_TextColor_Highlight = Color();
};

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

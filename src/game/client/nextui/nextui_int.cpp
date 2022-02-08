#include "cbase.h"
#include "nextui_int.h"
#include "ienginevgui.h"
#include "vgui/ILocalize.h"
#include "vgui_controls/Panel.h"
#include "gamemenu.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static CNextUI s_NextUI;
static IGameUI *s_pGameUI = NULL;
static CDllDemandLoader s_GameUIDll("GameUI");
CBaseGameSystemPerFrame *g_pNextUI = &s_NextUI;

const char *CNextUI::Name()
{
    return "NextUI";
}

bool CNextUI::Init()
{
    CreateInterfaceFn pfnGameUiFactory = s_GameUIDll.GetFactory();
    if(!pfnGameUiFactory) {
        ConColorMsg(LOG_COLOR_B_RED, "NextUI: unable to load GameUI module.\n");
        return false;
    }

    s_pGameUI = (IGameUI *)pfnGameUiFactory(GAMEUI_INTERFACE_VERSION, NULL);
    if(!s_pGameUI) {
        ConColorMsg(LOG_COLOR_B_RED, "NextUI: unable to load IGameUI.\n");
        return false;
    }

    m_hScheme = g_pVGuiSchemeManager->LoadSchemeFromFile("resource/nextuischeme.res", "NextUI");

    m_pRootPanel = new CNextUIRootPanel(enginevgui->GetPanel(PANEL_GAMEUIDLL));
    m_pRootPanel->SetScheme(m_hScheme);

    m_pAnimationController = new vgui::AnimationController(m_pRootPanel);
    m_pAnimationController->SetProportional(false);
    m_pAnimationController->SetScheme(m_hScheme);

    s_pGameUI->SetMainMenuOverride(m_pRootPanel->GetVPanel());

    m_pGameMenuScreen = new CGameMenu(m_pRootPanel);
    m_pCurrentScreen = m_pGameMenuScreen;

    return true;
}

void CNextUI::PostInit()
{
    ConColorMsg(LOG_COLOR_B_GREEN, "NextUI: PostInit\n");
}

void CNextUI::Shutdown()
{
    ConColorMsg(LOG_COLOR_B_GREEN, "NextUI: Shutdown\n");
}

void CNextUI::VidInit()
{
    ConColorMsg(LOG_COLOR_B_GREEN, "NextUI: VidInit\n");
}

void CNextUI::Update(float frametime)
{
    m_pAnimationController->UpdateAnimations(Plat_FloatTime());
}

vgui::HScheme CNextUI::GetScheme() const
{
    return m_hScheme;
}

vgui::AnimationController *CNextUI::GetAnimationController() const
{
    return m_pAnimationController;
}

CNextUIRootPanel *CNextUI::GetRootPanel() const
{
    return m_pRootPanel;
}

void CNextUI::GetLocalizedString(const char *pszSource, wchar_t *pszDest, int iDestChars) const
{
    const wchar_t *pszValue = g_pVGuiLocalize->Find(pszSource);
    if(pszValue) {
        Q_wcsncpy(pszDest, pszValue, sizeof(wchar_t) * iDestChars);
        return;
    }

    g_pVGuiLocalize->ConvertANSIToUnicode(pszSource, pszDest, iDestChars);
}

void CNextUI::SetCurrentScreen(CNextUIScreen *pScreen)
{
    if(m_pCurrentScreen != pScreen) {
        if(m_pCurrentScreen)
            m_pCurrentScreen->SetVisible(false);
        m_pPreviousScreen = m_pCurrentScreen;
        m_pCurrentScreen = pScreen;
        m_pCurrentScreen->SetVisible(true);
        m_pCurrentScreen->Activate();
    }
}

void CNextUI::UnsetCurrentScreen(CNextUIScreen *pScreen)
{
    if(m_pCurrentScreen == pScreen) {
        m_pCurrentScreen->SetVisible(false);
        m_pCurrentScreen = m_pPreviousScreen;
        if(!m_pCurrentScreen)
            m_pCurrentScreen = m_pGameMenuScreen;
        m_pCurrentScreen->SetVisible(true);
        m_pPreviousScreen = NULL;
    }
}

CNextUIScreen *CNextUI::GetCurrentScreen()
{
    return m_pCurrentScreen;
}

IGameUI *CNextUI::GetGameUI()
{
    return s_pGameUI;
}

CNextUI *CNextUI::GetInstance()
{
    return &s_NextUI;
}

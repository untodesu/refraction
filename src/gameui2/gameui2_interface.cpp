#include "gameui2_interface.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static CDllDemandLoader g_gameuiDll( "gameui" );

CGameUI2 g_GameUI2;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CGameUI2, IGameUI2, GAMEUI2_INTERFACE_VERSION, g_GameUI2 );

IVEngineClient *            g_pEngine = NULL;
IEngineSound *              g_pEngineSound = NULL;
IEngineVGui *               g_pEngineVGui = NULL;
ISoundEmitterSystemBase *   g_pSoundEmitterSystem = NULL;
IVRenderView *              g_pRenderView = NULL;
IMaterialSystem *           g_pMaterialSystem = NULL;
IMatSystemSurface *         g_pMatSystemSurface = NULL;
IFileSystem *               g_pFileSystem = NULL;

bool GameUI2_InitInterfaces( CreateInterfaceFn appFactory )
{
    g_pEngine               = (IVEngineClient *)            appFactory( VENGINE_CLIENT_INTERFACE_VERSION, NULL );
    g_pEngineSound          = (IEngineSound*)               appFactory( IENGINESOUND_CLIENT_INTERFACE_VERSION, NULL );
    g_pEngineVGui           = (IEngineVGui*)                appFactory( VENGINE_VGUI_VERSION, NULL );
    g_pSoundEmitterSystem   = (ISoundEmitterSystemBase*)    appFactory( SOUNDEMITTERSYSTEM_INTERFACE_VERSION, NULL );
    g_pRenderView           = (IVRenderView*)               appFactory( VENGINE_RENDERVIEW_INTERFACE_VERSION, NULL );
    g_pMaterialSystem       = (IMaterialSystem *)           appFactory( MATERIAL_SYSTEM_INTERFACE_VERSION, NULL );
    g_pMatSystemSurface     = (IMatSystemSurface*)          appFactory( MAT_SYSTEM_SURFACE_INTERFACE_VERSION, NULL );
    g_pFileSystem           = (IFileSystem *)               appFactory( FILESYSTEM_INTERFACE_VERSION, NULL );

    return ( g_pEngine && g_pEngineSound && g_pEngineVGui && g_pSoundEmitterSystem && g_pRenderView && g_pMaterialSystem && g_pMatSystemSurface && g_pFileSystem );
}

void CGameUI2::Initialize( CreateInterfaceFn appFactory )
{
    MEM_ALLOC_CREDIT();
    ConnectTier1Libraries( &appFactory, 1 );
    ConnectTier2Libraries( &appFactory, 1 );
    ConnectTier3Libraries( &appFactory, 1 );
    ConVar_Register( FCVAR_CLIENTDLL );

    // Initialize engine stuff
    const bool engineStatus = GameUI2_InitInterfaces( appFactory );
    const bool vguiStatus = vgui::VGui_InitInterfacesList( GAMEUI_INTERFACE_VERSION, &appFactory, 1 );

    CreateInterfaceFn factory = g_gameuiDll.GetFactory();
    if( factory ) {
        m_pGameUI1 = (IGameUI *)factory( GAMEUI_INTERFACE_VERSION, NULL );
    }

    if( !engineStatus || !vguiStatus || !m_pGameUI1 ) {
        Error( "CGameUI2::Initialize() failed to get necessary interfaces\n");
        // Uh oh, there's a crash
    }

    // setup panel
    m_pRootPanel = new CGameUI2RootPanel( GetVPanel() );
    m_pGameUI1->SetMainMenuOverride( m_pRootPanel->GetVPanel() );

    // Setup AC
    m_pAnimController = new vgui::AnimationController( m_pRootPanel );
    m_pAnimController->SetScheme( m_pRootPanel->GetScheme() );
    //m_pAnimController->SetScriptFile( m_pRootPanel->GetVPanel(), "scripts/uianimations.txt", true );
}

void CGameUI2::Shutdown()
{
    if( m_pAnimController ) {
        m_pAnimController->SetParent( ( vgui::VPANEL )0 );
        delete m_pAnimController;
    }

    if( m_pGameUI1 ) {
        m_pGameUI1->SetMainMenuOverride( ( vgui::VPANEL )0 );
    }

    if( m_pRootPanel ) {
        m_pRootPanel->SetParent( ( vgui::VPANEL )0 );
        delete m_pRootPanel;
    }

    ConVar_Unregister();
    DisconnectTier3Libraries();
    DisconnectTier2Libraries();
    DisconnectTier1Libraries();
}

void CGameUI2::VidInit()
{
    int updated = 0;
    for( CUIElementReg *pItem = CUIElementReg::s_pElements; pItem; pItem = pItem->m_pNext ) {
        if( pItem->m_pElement ) {
            // Initialize() usually loads data from scheme, so here we are.
            pItem->m_pElement->Initialize();
            updated++;
        }
    }

    GameUI2_Warning( "GameUI2: VidInit() called, %d panels should be updated...\n", updated );
}

void CGameUI2::PostInit()
{
    m_pRootPanel->SetVisible( true );

    vgui::HScheme hScheme = g_pVGuiSchemeManager->LoadSchemeFromFile( "resource/uischeme.res", "UIScheme" );

    // Setup panels
    int found = 0;
    for( CUIElementReg *pItem = CUIElementReg::s_pElements; pItem; pItem = pItem->m_pNext ) {
        pItem->Create();
        pItem->m_pElement->SetParent( m_pRootPanel );
        pItem->m_pElement->SetScheme( hScheme );
        pItem->m_pElement->Initialize();
        found++;
    }

    GameUI2_Message( "GameUI2: Initialized %d panels.\n", found );
}

void CGameUI2::PreShutdown()
{
    // Shutdown panels
    for( CUIElementReg *pItem = CUIElementReg::s_pElements; pItem; pItem = pItem->m_pNext ) {
        pItem->m_pElement->Shutdown();
        pItem->Delete();    // <-- Automatically unparents
    }
}

void CGameUI2::OnUpdate()
{
    if( m_pAnimController ) {
        m_pAnimController->UpdateAnimations( Plat_FloatTime() );
    }
}

void CGameUI2::LevelInitPreEntity( const char *pMapName )
{

}

void CGameUI2::LevelInitPostEntity()
{

}

void CGameUI2::LevelShutdown()
{
    if( m_pAnimController ) {
        m_pAnimController->UpdateAnimations( Plat_FloatTime() );
        m_pAnimController->RunAllAnimationsToCompletion();
    }
}

vgui::VPANEL CGameUI2::GetVPanel()
{
    return g_pEngineVGui->GetPanel( PANEL_GAMEUIDLL );
}

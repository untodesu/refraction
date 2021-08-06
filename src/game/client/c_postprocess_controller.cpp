#include "cbase.h"
#include "c_postprocess_controller.h"
#include "viewpostprocess.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( postprocess_controller, C_PostProcessController );

IMPLEMENT_CLIENTCLASS_DT( C_PostProcessController, DT_PostProcessController, CPostProcessController )
    RecvPropBool( RECVINFO( m_bBloomScaleOverriden ) ),
    RecvPropFloat( RECVINFO( m_flBloomScale ) ),
    RecvPropBool( RECVINFO( m_bMinAutoExposureOverriden ) ),
    RecvPropBool( RECVINFO( m_bMaxAutoExposureOverriden ) ),
    RecvPropFloat( RECVINFO( m_flMinAutoExposure ) ),
    RecvPropFloat( RECVINFO( m_flMaxAutoExposure ) ),
END_RECV_TABLE();
 
C_PostProcessController *C_PostProcessController::s_pController = NULL;

C_PostProcessController::~C_PostProcessController()
{
    if( s_pController == this )
        s_pController = NULL;
    g_pPostProcess->UnholdAllCustomPostEffects();
}

void C_PostProcessController::OnDataChanged( DataUpdateType_t type )
{
    Assert( s_pController == NULL || s_pController == this );
    BaseClass::OnDataChanged( type );
    s_pController = this;
}

void C_PostProcessController::ReceiveMessage( int iClassID, bf_read &msg )
{
    if( iClassID != GetClientClass()->m_ClassID ) {
        BaseClass::ReceiveMessage( iClassID, msg );
        return;
    }

    int iCommand = 0;
    char szEffectName[128] = { 0 };
    msg.ReadString( szEffectName, sizeof( szEffectName ) );
    iCommand = msg.ReadByte();

    if( iCommand == 0xFF ) {
        // Allow ConVars to take control back...
        g_pPostProcess->EnableCustomPostEffect( szEffectName, false, false );
        return;
    }

    g_pPostProcess->EnableCustomPostEffect( szEffectName, iCommand != 0x00, true );
}

bool C_PostProcessController::IsBloomScaleOverriden()
{
    return s_pController && s_pController->m_bBloomScaleOverriden;
}

float C_PostProcessController::GetBloomScale()
{
    return s_pController ? s_pController->m_flBloomScale : 0.0f;
}

bool C_PostProcessController::IsMinAutoExposureOverriden()
{
    return s_pController && s_pController->m_bMinAutoExposureOverriden;
}

bool C_PostProcessController::IsMaxAutoExposureOverriden()
{
    return s_pController && s_pController->m_bMaxAutoExposureOverriden;
}

float C_PostProcessController::GetMinAutoExposure()
{
    return s_pController ? s_pController->m_flMinAutoExposure : 0.0f;
}

float C_PostProcessController::GetMaxAutoExposure()
{
    return s_pController ? s_pController->m_flMaxAutoExposure : 0.0f;
}

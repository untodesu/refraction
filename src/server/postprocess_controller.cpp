#include "cbase.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CPostProcessController : public CBaseEntity {
    DECLARE_CLASS( CPostProcessController, CBaseEntity );
public:
    DECLARE_DATADESC();
    DECLARE_SERVERCLASS();

    void Spawn() override;
    int UpdateTransmitState() override;
    void UpdateTonemapScaleBlend();

    void Input_EnableCustomPostEffect( inputdata_t &data );
    void Input_DisableCustomPostEffect( inputdata_t &data );
    void Input_ResetCustomPostEffect( inputdata_t &data );
    void Input_SetBloomScale( inputdata_t &data );
    void Input_ResetBloomScale( inputdata_t &data );
    void Input_SetTonemapScale( inputdata_t &data );
    void Input_BlendTonemapScale( inputdata_t &data );
    void Input_SetTonemapRate( inputdata_t &data );
    void Input_SetMinAutoExposure( inputdata_t &data );
    void Input_ResetMinAutoExposure( inputdata_t &data );
    void Input_SetMaxAutoExposure( inputdata_t &data );
    void Input_ResetMaxAutoExposure( inputdata_t &data );

private:
    float m_flBlendTonemapStart;
    float m_flBlendTonemapEnd;
    float m_flBlendEndTime;
    float m_flBlendStartTime;

    CNetworkVar( bool, m_bBloomScaleOverriden );
    CNetworkVar( float, m_flBloomScale );
    CNetworkVar( bool, m_bMinAutoExposureOverriden );
    CNetworkVar( bool, m_bMaxAutoExposureOverriden );
    CNetworkVar( float, m_flMinAutoExposure );
    CNetworkVar( float, m_flMaxAutoExposure );
};

LINK_ENTITY_TO_CLASS( postprocess_controller, CPostProcessController );

static ConVarRef mat_hdr_tonemapscale( "mat_hdr_tonemapscale", false );
static ConVarRef mat_hdr_manual_tonemap_rate( "mat_hdr_manual_tonemap_rate", false );

BEGIN_DATADESC( CPostProcessController )
    DEFINE_FIELD( m_flBlendTonemapStart, FIELD_FLOAT ),
    DEFINE_FIELD( m_flBlendTonemapEnd, FIELD_FLOAT),
    DEFINE_FIELD( m_flBlendEndTime, FIELD_TIME ),
    DEFINE_FIELD( m_flBlendStartTime, FIELD_TIME ),
    DEFINE_FIELD( m_bBloomScaleOverriden, FIELD_BOOLEAN ),
    DEFINE_FIELD( m_flBloomScale, FIELD_FLOAT ),
    DEFINE_FIELD( m_bMinAutoExposureOverriden, FIELD_BOOLEAN ),
    DEFINE_FIELD( m_bMaxAutoExposureOverriden, FIELD_BOOLEAN ),
    DEFINE_FIELD( m_flMinAutoExposure, FIELD_FLOAT ),
    DEFINE_FIELD( m_flMaxAutoExposure, FIELD_FLOAT ),

    DEFINE_THINKFUNC( UpdateTonemapScaleBlend ),

    DEFINE_INPUTFUNC( FIELD_STRING, "EnableCustomPostEffect", Input_EnableCustomPostEffect ),
    DEFINE_INPUTFUNC( FIELD_STRING, "DisableCustomPostEffect", Input_DisableCustomPostEffect ),
    DEFINE_INPUTFUNC( FIELD_STRING, "ResetCustomPostEffect", Input_ResetCustomPostEffect ),
    DEFINE_INPUTFUNC( FIELD_FLOAT, "SetBloomScale", Input_SetBloomScale ),
    DEFINE_INPUTFUNC( FIELD_VOID, "ResetBloomScale", Input_ResetBloomScale ),
    DEFINE_INPUTFUNC( FIELD_FLOAT, "SetTonemapScale", Input_SetTonemapScale ),
    DEFINE_INPUTFUNC( FIELD_STRING, "BlendTonemapScale", Input_BlendTonemapScale ),
    DEFINE_INPUTFUNC( FIELD_FLOAT, "SetTonemapRate", Input_SetTonemapRate ),
    DEFINE_INPUTFUNC( FIELD_FLOAT, "SetMinAutoExposure", Input_SetMinAutoExposure ),
    DEFINE_INPUTFUNC( FIELD_VOID, "ResetMinAutoExposure", Input_ResetMinAutoExposure ),
    DEFINE_INPUTFUNC( FIELD_FLOAT, "SetMaxAutoExposure", Input_SetMaxAutoExposure ),
    DEFINE_INPUTFUNC( FIELD_VOID, "ResetMaxAutoExposure", Input_ResetMaxAutoExposure ),
END_DATADESC();

IMPLEMENT_SERVERCLASS_ST( CPostProcessController, DT_PostProcessController )
    SendPropBool( SENDINFO( m_bBloomScaleOverriden ) ),
    SendPropFloat( SENDINFO( m_flBloomScale ) ),
    SendPropBool( SENDINFO( m_bMinAutoExposureOverriden ) ),
    SendPropBool( SENDINFO( m_bMaxAutoExposureOverriden ) ),
    SendPropFloat( SENDINFO( m_flMinAutoExposure ) ),
    SendPropFloat( SENDINFO( m_flMaxAutoExposure ) ),
END_SEND_TABLE();

void CPostProcessController::Spawn()
{
    SetSolid( SOLID_NONE );
    SetMoveType( MOVETYPE_NONE );

    m_bBloomScaleOverriden = false;
    m_flBloomScale = 0.0f;
    m_bMinAutoExposureOverriden = false;
    m_bMaxAutoExposureOverriden = false;
    m_flMinAutoExposure = 0.0f;
    m_flMaxAutoExposure = 0.0f;
}

int CPostProcessController::UpdateTransmitState()
{
    return SetTransmitState( FL_EDICT_ALWAYS );
}

void CPostProcessController::UpdateTonemapScaleBlend()
{
    mat_hdr_tonemapscale.SetValue( RemapValClamped( gpGlobals->curtime, m_flBlendStartTime, m_flBlendEndTime, m_flBlendTonemapStart, m_flBlendTonemapEnd ) );
    if( gpGlobals->curtime >= m_flBlendEndTime )
        return;
    SetNextThink( gpGlobals->curtime + 0.1f );
}

void CPostProcessController::Input_EnableCustomPostEffect( inputdata_t &data )
{
    EntityMessageBegin( this, true );
    WRITE_STRING( data.value.String() );
    WRITE_BYTE( 0x01 );
    MessageEnd();
}

void CPostProcessController::Input_DisableCustomPostEffect( inputdata_t &data )
{
    EntityMessageBegin( this, true );
    WRITE_STRING( data.value.String() );
    WRITE_BYTE( 0x00 );
    MessageEnd();
}

void CPostProcessController::Input_ResetCustomPostEffect( inputdata_t &data )
{
    EntityMessageBegin( this, true );
    WRITE_STRING( data.value.String() );
    WRITE_BYTE( 0xFF );
    MessageEnd();
}

void CPostProcessController::Input_SetBloomScale( inputdata_t &data )
{
    m_flBloomScale = data.value.Float();
    m_bBloomScaleOverriden = true;
}

void CPostProcessController::Input_ResetBloomScale( inputdata_t &data )
{
    m_flBloomScale = 0.0f;
    m_bBloomScaleOverriden = false;
}

void CPostProcessController::Input_SetTonemapScale( inputdata_t &data )
{
    mat_hdr_tonemapscale.SetValue( data.value.Float() );
}

void CPostProcessController::Input_BlendTonemapScale( inputdata_t &data )
{
    char szParseString[256];
    Q_strncpy( szParseString, data.value.String(), sizeof( szParseString ) - 1 );

    // <tonemap_scale> <blend_time>
    float flScale, flBlendTime;
    if( sscanf( szParseString, "%f %f", &flScale, &flBlendTime ) < 2 ) {
        Warning( "%s (%s) received BlendTonemapScale with invalid argument. Syntax: <tonemap_scale> <blend_time>\n", GetClassname(), GetDebugName() );
        return;
    }

    m_flBlendTonemapStart = mat_hdr_tonemapscale.GetFloat();
    m_flBlendTonemapEnd = flScale;
    m_flBlendStartTime = gpGlobals->curtime;
    m_flBlendEndTime = m_flBlendStartTime + flBlendTime;

    // Start thinking
    SetNextThink( gpGlobals->curtime + 0.1 );
    SetThink( &CPostProcessController::UpdateTonemapScaleBlend );
}

void CPostProcessController::Input_SetTonemapRate( inputdata_t &data )
{
    mat_hdr_manual_tonemap_rate.SetValue( data.value.Float() );
}

void CPostProcessController::Input_SetMinAutoExposure( inputdata_t &data )
{
    m_flMinAutoExposure = data.value.Float();
    m_bMinAutoExposureOverriden = true;
}

void CPostProcessController::Input_ResetMinAutoExposure( inputdata_t &data )
{
    m_flMinAutoExposure = 0.0f;
    m_bMinAutoExposureOverriden = false;
}

void CPostProcessController::Input_SetMaxAutoExposure( inputdata_t &data )
{
    m_flMaxAutoExposure = data.value.Float();
    m_bMaxAutoExposureOverriden = true;
}

void CPostProcessController::Input_ResetMaxAutoExposure( inputdata_t &data )
{
    m_flMaxAutoExposure = 0.0f;
    m_bMaxAutoExposureOverriden = false;
}

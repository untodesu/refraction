//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================
#include "cbase.h"

extern bool g_bUseCustomAutoExposureMin;
extern bool g_bUseCustomAutoExposureMax;
extern float g_flCustomAutoExposureMin;
extern float g_flCustomAutoExposureMax;

EHANDLE g_hTonemapControllerInUse = NULL;

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
class C_EnvTonemapController : public C_BaseEntity
{
    DECLARE_CLASS( C_EnvTonemapController, C_BaseEntity );
public:
    DECLARE_CLIENTCLASS();

    C_EnvTonemapController();
    ~C_EnvTonemapController();
    virtual void    OnDataChanged( DataUpdateType_t updateType );

private:
    bool m_bUseCustomAutoExposureMin;
    bool m_bUseCustomAutoExposureMax;
    float m_flCustomAutoExposureMin;
    float m_flCustomAutoExposureMax;
private:
    C_EnvTonemapController( const C_EnvTonemapController & );
};

IMPLEMENT_CLIENTCLASS_DT( C_EnvTonemapController, DT_EnvTonemapController, CEnvTonemapController )
    RecvPropInt( RECVINFO(m_bUseCustomAutoExposureMin) ),
    RecvPropInt( RECVINFO(m_bUseCustomAutoExposureMax) ),
    RecvPropFloat( RECVINFO(m_flCustomAutoExposureMin) ),
    RecvPropFloat( RECVINFO(m_flCustomAutoExposureMax) ),
END_RECV_TABLE()

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
C_EnvTonemapController::C_EnvTonemapController( void )
{
    m_bUseCustomAutoExposureMin = false;
    m_bUseCustomAutoExposureMax = false;
    m_flCustomAutoExposureMin = 0;
    m_flCustomAutoExposureMax = 0;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
C_EnvTonemapController::~C_EnvTonemapController( void )
{
    if ( g_hTonemapControllerInUse == this )
    {
        g_bUseCustomAutoExposureMin = false;
        g_bUseCustomAutoExposureMax = false;
    }
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_EnvTonemapController::OnDataChanged( DataUpdateType_t updateType )
{
    BaseClass::OnDataChanged(updateType);

    g_bUseCustomAutoExposureMin = m_bUseCustomAutoExposureMin;
    g_bUseCustomAutoExposureMax = m_bUseCustomAutoExposureMax;
    g_flCustomAutoExposureMin = m_flCustomAutoExposureMin;
    g_flCustomAutoExposureMax = m_flCustomAutoExposureMax;

    g_hTonemapControllerInUse = this;
}


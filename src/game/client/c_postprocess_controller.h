#ifndef C_POSTPROCESS_CONTROLLER_H
#define C_POSTPROCESS_CONTROLLER_H 1
#include "c_baseentity.h"

class C_PostProcessController : public C_BaseEntity {
    DECLARE_CLASS( C_PostProcessController, C_BaseEntity );
public:
    DECLARE_CLIENTCLASS();

    virtual ~C_PostProcessController();
    void OnDataChanged( DataUpdateType_t type ) override;
    void ReceiveMessage( int iClassID, bf_read &msg ) override;

    static bool IsBloomScaleOverriden();
    static float GetBloomScale();
    static bool IsMinAutoExposureOverriden();
    static bool IsMaxAutoExposureOverriden();
    static float GetMinAutoExposure();
    static float GetMaxAutoExposure();

private:
    bool m_bBloomScaleOverriden = false;
    float m_flBloomScale = 0.0f;
    bool m_bMinAutoExposureOverriden = false;
    bool m_bMaxAutoExposureOverriden = false;
    float m_flMinAutoExposure = 0.0f;
    float m_flMaxAutoExposure = 0.0f;

    static C_PostProcessController *s_pController;
};

#endif

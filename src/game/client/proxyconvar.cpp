#include "cbase.h"
#include "functionproxy.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// forward declarations
void ToolFramework_RecordMaterialParams(IMaterial *pMaterial);

class CConVarProxy : public CResultProxy {
public:
    bool Init(IMaterial *pMaterial, KeyValues *pKeyValues)
    {
        if(!CResultProxy::Init(pMaterial, pKeyValues))
            return false;
        const char *pszConVarName = pKeyValues->GetString("convar", NULL);
        if(!pszConVarName)
            return false;
        m_pConVar = g_pCVar->FindVar(pszConVarName);
        if(!m_pConVar)
            return false;
        return true;
    }

    void OnBind(void *pC_BaseEntity)
    {
        Assert(m_pResult);
        m_pResult->SetValueAutodetectType(m_pConVar->GetString());
    }

private:
    ConVar *m_pConVar;
};
EXPOSE_INTERFACE(CConVarProxy, IMaterialProxy, "ConVar" IMATERIAL_PROXY_INTERFACE_VERSION);

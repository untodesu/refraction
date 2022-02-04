#include "BaseVSShader.h"
#include "sdk_lensdistortion_vs30.inc"
#include "sdk_lensdistortion_ps30.inc"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

BEGIN_SHADER(SDK_LensDistortion, "Help for SDK_LensDistortion")
    BEGIN_SHADER_PARAMS
        SHADER_PARAM(AMOUNT, SHADER_PARAM_TYPE_VEC2, "[-4.0 0.5]", "Distortion amount")
        SHADER_PARAM(HEALTH, SHADER_PARAM_TYPE_FLOAT, "1.0", "Player health")
    END_SHADER_PARAMS;

    SHADER_INIT
    {
        if(!params[AMOUNT]->IsDefined())
            params[AMOUNT]->SetVecValue(-4.0f, 0.5f);
        if(!params[HEALTH]->IsDefined())
            params[HEALTH]->SetFloatValue(1.0f);
    }

    SHADER_FALLBACK
    {
        return 0;
    }

    SHADER_DRAW
    {
        SHADOW_STATE
        {
            pShaderShadow->EnableDepthWrites(false);
            pShaderShadow->EnableTexture(SHADER_SAMPLER0, true);

            pShaderShadow->VertexShaderVertexFormat(VERTEX_POSITION, 1, 0, 0);

            DECLARE_STATIC_VERTEX_SHADER(sdk_lensdistortion_vs30);
            SET_STATIC_VERTEX_SHADER(sdk_lensdistortion_vs30);
            DECLARE_STATIC_PIXEL_SHADER(sdk_lensdistortion_ps30);
            SET_STATIC_PIXEL_SHADER(sdk_lensdistortion_ps30);
        }

        DYNAMIC_STATE
        {
            pShaderAPI->BindStandardTexture(SHADER_SAMPLER0, TEXTURE_FRAME_BUFFER_FULL_TEXTURE_0);

            const float *C0half = params[AMOUNT]->GetVecValue();
            const float C0[4] = { C0half[0], C0half[1], params[HEALTH]->GetFloatValue(), (float)Plat_FloatTime() };
            pShaderAPI->SetPixelShaderConstant(0, C0, 4);

            DECLARE_DYNAMIC_VERTEX_SHADER(sdk_lensdistortion_vs30);
            SET_DYNAMIC_VERTEX_SHADER(sdk_lensdistortion_vs30);
            DECLARE_DYNAMIC_PIXEL_SHADER(sdk_lensdistortion_ps30);
            SET_DYNAMIC_PIXEL_SHADER(sdk_lensdistortion_ps30);
        }

        Draw();
    }
END_SHADER

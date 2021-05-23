//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
// This is what all vs/ps (dx8+) shaders inherit from.
//===========================================================================//

#include "BaseVSShader.h"
#include "mathlib/vmatrix.h"
#include "mathlib/bumpvects.h"
#include "cpp_shader_constant_register_map.h"
#include "convar.h"

#include "re_lightmappedgeneric_flashlight_vs30.inc"
#include "re_flashlight_ps30.inc"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static ConVarRef mat_fullbright( "mat_fullbright", false );

void CBaseVSShader::SetVertexShaderTextureTransform( int vertexReg, int transformVar )
{
    Vector4D transformation[2];
    IMaterialVar* pTransformationVar = s_ppParams[transformVar];
    if( pTransformationVar && ( pTransformationVar->GetType() == MATERIAL_VAR_TYPE_MATRIX ) ) {
        const VMatrix &mat = pTransformationVar->GetMatrixValue();
        transformation[0].Init( mat[0][0], mat[0][1], mat[0][2], mat[0][3] );
        transformation[1].Init( mat[1][0], mat[1][1], mat[1][2], mat[1][3] );
    }
    else {
        transformation[0].Init( 1.0f, 0.0f, 0.0f, 0.0f );
        transformation[1].Init( 0.0f, 1.0f, 0.0f, 0.0f );
    }
    s_pShaderAPI->SetVertexShaderConstant( vertexReg, transformation[0].Base(), 2 );
}

void CBaseVSShader::SetVertexShaderTextureScaledTransform( int vertexReg, int transformVar, int scaleVar )
{
    Vector4D transformation[2];
    IMaterialVar* pTransformationVar = s_ppParams[transformVar];
    if( pTransformationVar && ( pTransformationVar->GetType() == MATERIAL_VAR_TYPE_MATRIX ) ) {
        const VMatrix &mat = pTransformationVar->GetMatrixValue();
        transformation[0].Init( mat[0][0], mat[0][1], mat[0][2], mat[0][3] );
        transformation[1].Init( mat[1][0], mat[1][1], mat[1][2], mat[1][3] );
    }
    else {
        transformation[0].Init( 1.0f, 0.0f, 0.0f, 0.0f );
        transformation[1].Init( 0.0f, 1.0f, 0.0f, 0.0f );
    }

    Vector2D scale( 1, 1 );
    IMaterialVar* pScaleVar = s_ppParams[scaleVar];
    if( pScaleVar ) {
        if( pScaleVar->GetType() == MATERIAL_VAR_TYPE_VECTOR )
            pScaleVar->GetVecValue( scale.Base(), 2 );
        else if( pScaleVar->IsDefined() )
            scale[0] = scale[1] = pScaleVar->GetFloatValue();
    }

    // Apply the scaling
    transformation[0][0] *= scale[0];
    transformation[0][1] *= scale[1];
    transformation[1][0] *= scale[0];
    transformation[1][1] *= scale[1];
    transformation[0][3] *= scale[0];
    transformation[1][3] *= scale[1];
    s_pShaderAPI->SetVertexShaderConstant( vertexReg, transformation[0].Base(), 2 );
}

//-----------------------------------------------------------------------------
// GR - translucency query
//-----------------------------------------------------------------------------
BlendType_t CBaseVSShader::EvaluateBlendRequirements( int textureVar, bool isBaseTexture,
                                                      int detailTextureVar )
{
    // Either we've got a constant modulation
    bool isTranslucent = IsAlphaModulating();

    // Or we've got a vertex alpha
    isTranslucent = isTranslucent || ( CurrentMaterialVarFlags() & MATERIAL_VAR_VERTEXALPHA );

    // Or we've got a texture alpha (for blending or alpha test)
    isTranslucent = isTranslucent || ( TextureIsTranslucent( textureVar, isBaseTexture ) && !( CurrentMaterialVarFlags() & MATERIAL_VAR_ALPHATEST ) );

    if( ( detailTextureVar != -1 ) && ( !isTranslucent ) ) {
        isTranslucent = TextureIsTranslucent( detailTextureVar, isBaseTexture );
    }

    if( CurrentMaterialVarFlags() & MATERIAL_VAR_ADDITIVE ) {
        return isTranslucent ? BT_BLENDADD : BT_ADD; // Additive
    }
    else {
        return isTranslucent ? BT_BLEND : BT_NONE; // Normal blending
    }
}

void CBaseVSShader::SetFlashlightVertexShaderConstants( bool bBump, int bumpTransformVar, bool bDetail, int detailScaleVar, bool bSetTextureTransforms )
{
    Assert( !IsSnapshotting() );

    VMatrix worldToTexture;
    const FlashlightState_t &flashlightState = s_pShaderAPI->GetFlashlightState( worldToTexture );

    // Set the flashlight origin
    float pos[4];
    pos[0] = flashlightState.m_vecLightOrigin[0];
    pos[1] = flashlightState.m_vecLightOrigin[1];
    pos[2] = flashlightState.m_vecLightOrigin[2];
    pos[3] = 1.0f / ( ( 0.6f * flashlightState.m_FarZ ) - flashlightState.m_FarZ );     // DX8 needs this

    s_pShaderAPI->SetVertexShaderConstant( VERTEX_SHADER_SHADER_SPECIFIC_CONST_0, pos, 1 );

    s_pShaderAPI->SetVertexShaderConstant( VERTEX_SHADER_SHADER_SPECIFIC_CONST_1, worldToTexture.Base(), 4 );

    // Set the flashlight attenuation factors
    float atten[4];
    atten[0] = flashlightState.m_fConstantAtten;
    atten[1] = flashlightState.m_fLinearAtten;
    atten[2] = flashlightState.m_fQuadraticAtten;
    atten[3] = flashlightState.m_FarZ;
    s_pShaderAPI->SetVertexShaderConstant( VERTEX_SHADER_SHADER_SPECIFIC_CONST_5, atten, 1 );

    if( bDetail ) {
        SetVertexShaderTextureScaledTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_8, BASETEXTURETRANSFORM, detailScaleVar );
    }

    if( bSetTextureTransforms ) {
        SetVertexShaderTextureTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_6, BASETEXTURETRANSFORM );
        if( !bDetail && bBump && bumpTransformVar != -1 ) {
            SetVertexShaderTextureTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_8, bumpTransformVar ); // aliased on top of detail transform
        }
    }
}

void CBaseVSShader::DrawFlashlight( IMaterialVar **params, IShaderDynamicAPI *pShaderAPI, IShaderShadow *pShaderShadow, DrawFlashlight_Vars_t &vars )
{
    // FLASHLIGHTFIXME: hack . . need to fix the vertex shader so that it can deal with and without bumps for vertexlitgeneric
    if( !vars.m_bLightmappedGeneric ) {
        vars.m_bBump = false;
    }
    bool bBump2 = vars.m_bWorldVertexTransition && vars.m_bBump && vars.m_nBumpmap2Var != -1 && params[vars.m_nBumpmap2Var]->IsTexture();
    bool bSeamless = vars.m_fSeamlessScale != 0.0;
    bool bDetail = vars.m_bLightmappedGeneric && ( vars.m_nDetailVar != -1 ) && params[vars.m_nDetailVar]->IsDefined() && ( vars.m_nDetailScale != -1 );

    int nDetailBlendMode = 0;
    if( bDetail ) {
        nDetailBlendMode = GetIntParam( vars.m_nDetailTextureCombineMode, params );
        nDetailBlendMode = nDetailBlendMode > 1 ? 1 : nDetailBlendMode;
    }

    if( pShaderShadow ) {
        SetInitialShadowState();
        pShaderShadow->EnableDepthWrites( false );
        pShaderShadow->EnableAlphaWrites( false );

        // Alpha blend
        SetAdditiveBlendingShadowState( BASETEXTURE, true );

        // Alpha test
        pShaderShadow->EnableAlphaTest( IS_FLAG_SET( MATERIAL_VAR_ALPHATEST ) );
        if( vars.m_nAlphaTestReference != -1 && params[vars.m_nAlphaTestReference]->GetFloatValue() > 0.0f ) {
            pShaderShadow->AlphaFunc( SHADER_ALPHAFUNC_GEQUAL, params[vars.m_nAlphaTestReference]->GetFloatValue() );
        }

        // Spot sampler
        pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );
        pShaderShadow->EnableSRGBRead( SHADER_SAMPLER0, true );

        // Base sampler
        pShaderShadow->EnableTexture( SHADER_SAMPLER1, true );
        pShaderShadow->EnableSRGBRead( SHADER_SAMPLER1, true );

        // Normalizing cubemap sampler
        pShaderShadow->EnableTexture( SHADER_SAMPLER2, true );

        // Normalizing cubemap sampler2 or normal map sampler
        pShaderShadow->EnableTexture( SHADER_SAMPLER3, true );

        // RandomRotation sampler
        pShaderShadow->EnableTexture( SHADER_SAMPLER5, true );

        // Flashlight depth sampler
        pShaderShadow->EnableTexture( SHADER_SAMPLER7, true );
        pShaderShadow->SetShadowDepthFiltering( SHADER_SAMPLER7 );

        if( vars.m_bWorldVertexTransition ) {
            // $basetexture2
            pShaderShadow->EnableTexture( SHADER_SAMPLER4, true );
            pShaderShadow->EnableSRGBRead( SHADER_SAMPLER4, true );
        }
        if( bBump2 ) {
            // Normalmap2 sampler
            pShaderShadow->EnableTexture( SHADER_SAMPLER6, true );
        }
        if( bDetail ) {
            pShaderShadow->EnableTexture( SHADER_SAMPLER8, true ); // detail sampler
            if( nDetailBlendMode != 0 ) //Not Mod2X
                pShaderShadow->EnableSRGBRead( SHADER_SAMPLER8, true );
        }

        pShaderShadow->EnableSRGBWrite( true );

        if( vars.m_bLightmappedGeneric ) {
            re_lightmappedgeneric_flashlight_vs30_Static_Index vshIndex( pShaderShadow, params );
            vshIndex.SetWORLDVERTEXTRANSITION( vars.m_bWorldVertexTransition );
            vshIndex.SetNORMALMAP( vars.m_bBump );
            vshIndex.SetSEAMLESS( bSeamless );
            vshIndex.SetDETAIL( bDetail );
            pShaderShadow->SetVertexShader( "re_lightmappedgeneric_flashlight_vs30", vshIndex.GetIndex() );

            unsigned int flags = VERTEX_POSITION | VERTEX_NORMAL;
            if( vars.m_bBump ) {
                flags |= VERTEX_TANGENT_S | VERTEX_TANGENT_T;
            }
            int numTexCoords = 1;
            if( vars.m_bWorldVertexTransition ) {
                flags |= VERTEX_COLOR;
                numTexCoords = 2; // need lightmap texcoords to get alpha.
            }
            pShaderShadow->VertexShaderVertexFormat( flags, numTexCoords, 0, 0 );
        }
#if 0
        else
        {
            vertexlitgeneric_flashlight_vs11_Static_Index vshIndex;
            vshIndex.SetTEETH( vars.m_bTeeth );
            pShaderShadow->SetVertexShader( "vertexlitgeneric_flashlight_vs11", vshIndex.GetIndex() );

            unsigned int flags = VERTEX_POSITION | VERTEX_NORMAL;
            int numTexCoords = 1;
            pShaderShadow->VertexShaderVertexFormat( flags, numTexCoords, 0, vars.m_bBump ? 4 : 0 );
        }
#endif

        int nBumpMapVariant = 0;
        if( vars.m_bBump ) {
            nBumpMapVariant = ( vars.m_bSSBump ) ? 2 : 1;
        }
        if( g_pHardwareConfig->SupportsPixelShaders_2_b() ) {
            int nShadowFilterMode = g_pHardwareConfig->GetShadowFilterMode();

            re_flashlight_ps30_Static_Index	pshIndex( pShaderShadow, params );
            pshIndex.SetNORMALMAP( nBumpMapVariant );
            pshIndex.SetNORMALMAP2( bBump2 );
            pshIndex.SetWORLDVERTEXTRANSITION( vars.m_bWorldVertexTransition );
            pshIndex.SetSEAMLESS( bSeamless );
            pshIndex.SetDETAILTEXTURE( bDetail );
            pshIndex.SetDETAIL_BLEND_MODE( nDetailBlendMode );
            pshIndex.SetFLASHLIGHTDEPTHFILTERMODE( nShadowFilterMode );
            pShaderShadow->SetPixelShader( "re_flashlight_ps30", pshIndex.GetIndex() );
        }
#if 0
        else
        {
            flashlight_ps20_Static_Index	pshIndex;
            pshIndex.SetNORMALMAP( nBumpMapVariant );
            pshIndex.SetNORMALMAP2( bBump2 );
            pshIndex.SetWORLDVERTEXTRANSITION( vars.m_bWorldVertexTransition );
            pshIndex.SetSEAMLESS( bSeamless );
            pshIndex.SetDETAILTEXTURE( bDetail );
            pshIndex.SetDETAIL_BLEND_MODE( nDetailBlendMode );
            pShaderShadow->SetPixelShader( "flashlight_ps20", pshIndex.GetIndex() );
        }
#endif
        FogToBlack();
    }
    else {
        VMatrix worldToTexture;
        ITexture *pFlashlightDepthTexture;
        FlashlightState_t flashlightState = pShaderAPI->GetFlashlightStateEx( worldToTexture, &pFlashlightDepthTexture );

        SetFlashLightColorFromState( flashlightState, pShaderAPI );

        BindTexture( SHADER_SAMPLER0, flashlightState.m_pSpotlightTexture, flashlightState.m_nSpotlightTextureFrame );

        if( pFlashlightDepthTexture && g_pConfig->ShadowDepthTexture() && flashlightState.m_bEnableShadows ) {
            BindTexture( SHADER_SAMPLER7, pFlashlightDepthTexture, 0 );
            pShaderAPI->BindStandardTexture( SHADER_SAMPLER5, TEXTURE_SHADOW_NOISE_2D );

            // Tweaks associated with a given flashlight
            float tweaks[4];
            tweaks[0] = ShadowFilterFromState( flashlightState );
            tweaks[1] = ShadowAttenFromState( flashlightState );
            HashShadow2DJitter( flashlightState.m_flShadowJitterSeed, &tweaks[2], &tweaks[3] );
            pShaderAPI->SetPixelShaderConstant( PSREG_ENVMAP_TINT__SHADOW_TWEAKS, tweaks, 1 );

            // Dimensions of screen, used for screen-space noise map sampling
            float vScreenScale[4] = { 1280.0f / 32.0f, 720.0f / 32.0f, 0, 0 };
            int nWidth, nHeight;
            pShaderAPI->GetBackBufferDimensions( nWidth, nHeight );
            vScreenScale[0] = (float)nWidth / 32.0f;
            vScreenScale[1] = (float)nHeight / 32.0f;
            pShaderAPI->SetPixelShaderConstant( PSREG_FLASHLIGHT_SCREEN_SCALE, vScreenScale, 1 );
        }

        if( params[BASETEXTURE]->IsTexture() && mat_fullbright.GetInt() != 2 ) {
            BindTexture( SHADER_SAMPLER1, BASETEXTURE, FRAME );
        }
        else {
            pShaderAPI->BindStandardTexture( SHADER_SAMPLER1, TEXTURE_GREY );
        }
        if( vars.m_bWorldVertexTransition ) {
            Assert( vars.m_nBaseTexture2Var >= 0 && vars.m_nBaseTexture2FrameVar >= 0 );
            BindTexture( SHADER_SAMPLER4, vars.m_nBaseTexture2Var, vars.m_nBaseTexture2FrameVar );
        }
        pShaderAPI->BindStandardTexture( SHADER_SAMPLER2, TEXTURE_NORMALIZATION_CUBEMAP );
        if( vars.m_bBump ) {
            BindTexture( SHADER_SAMPLER3, vars.m_nBumpmapVar, vars.m_nBumpmapFrame );
        }
        else {
            pShaderAPI->BindStandardTexture( SHADER_SAMPLER3, TEXTURE_NORMALIZATION_CUBEMAP );
        }

        if( bDetail ) {
            BindTexture( SHADER_SAMPLER8, vars.m_nDetailVar );
        }

        if( vars.m_bWorldVertexTransition ) {
            if( bBump2 ) {
                BindTexture( SHADER_SAMPLER6, vars.m_nBumpmap2Var, vars.m_nBumpmap2Frame );
            }
        }

        if( vars.m_bLightmappedGeneric ) {
            DECLARE_DYNAMIC_VERTEX_SHADER( re_lightmappedgeneric_flashlight_vs30 );
            SET_DYNAMIC_VERTEX_SHADER_COMBO( DOWATERFOG, pShaderAPI->GetSceneFogMode() == MATERIAL_FOG_LINEAR_BELOW_FOG_Z );
            SET_DYNAMIC_VERTEX_SHADER( re_lightmappedgeneric_flashlight_vs30 );
            if( bSeamless ) {
                float const0[4] = { vars.m_fSeamlessScale, 0, 0, 0 };
                pShaderAPI->SetVertexShaderConstant( VERTEX_SHADER_SHADER_SPECIFIC_CONST_6, const0 );
            }

            if( bDetail ) {
                float vDetailConstants[4] = { 1, 1, 1, 1 };

                if( vars.m_nDetailTint != -1 ) {
                    params[vars.m_nDetailTint]->GetVecValue( vDetailConstants, 3 );
                }

                if( vars.m_nDetailTextureBlendFactor != -1 ) {
                    vDetailConstants[3] = params[vars.m_nDetailTextureBlendFactor]->GetFloatValue();
                }

                pShaderAPI->SetPixelShaderConstant( 0, vDetailConstants, 1 );
            }
        }
#if 0
        else
        {
            vertexlitgeneric_flashlight_vs11_Dynamic_Index vshIndex;
            vshIndex.SetDOWATERFOG( pShaderAPI->GetSceneFogMode() == MATERIAL_FOG_LINEAR_BELOW_FOG_Z );
            vshIndex.SetSKINNING( pShaderAPI->GetCurrentNumBones() > 0 );
            pShaderAPI->SetVertexShaderIndex( vshIndex.GetIndex() );

            if( vars.m_bTeeth )
            {
                Assert( vars.m_nTeethForwardVar >= 0 );
                Assert( vars.m_nTeethIllumFactorVar >= 0 );
                Vector4D lighting;
                params[vars.m_nTeethForwardVar]->GetVecValue( lighting.Base(), 3 );
                lighting[3] = params[vars.m_nTeethIllumFactorVar]->GetFloatValue();
                pShaderAPI->SetVertexShaderConstant( VERTEX_SHADER_SHADER_SPECIFIC_CONST_0, lighting.Base() );
            }
        }
#endif

        pShaderAPI->SetPixelShaderFogParams( PSREG_FOG_PARAMS );

        float vEyePos_SpecExponent[4];
        pShaderAPI->GetWorldSpaceCameraPosition( vEyePos_SpecExponent );
        vEyePos_SpecExponent[3] = 0.0f;
        pShaderAPI->SetPixelShaderConstant( PSREG_EYEPOS_SPEC_EXPONENT, vEyePos_SpecExponent, 1 );

        if( g_pHardwareConfig->SupportsPixelShaders_2_b() ) {
            DECLARE_DYNAMIC_PIXEL_SHADER( re_flashlight_ps30 );
            SET_DYNAMIC_PIXEL_SHADER_COMBO( PIXELFOGTYPE, pShaderAPI->GetPixelFogCombo() );
            SET_DYNAMIC_PIXEL_SHADER_COMBO( FLASHLIGHTSHADOWS, flashlightState.m_bEnableShadows && ( pFlashlightDepthTexture != NULL ) );
            SET_DYNAMIC_PIXEL_SHADER( re_flashlight_ps30 );
        }
#if 0
        else
        {
            DECLARE_DYNAMIC_PIXEL_SHADER( flashlight_ps20 );
            SET_DYNAMIC_PIXEL_SHADER_COMBO( PIXELFOGTYPE,  pShaderAPI->GetPixelFogCombo() );
            SET_DYNAMIC_PIXEL_SHADER( flashlight_ps20 );
        }
#endif

        float atten[4];										// Set the flashlight attenuation factors
        atten[0] = flashlightState.m_fConstantAtten;
        atten[1] = flashlightState.m_fLinearAtten;
        atten[2] = flashlightState.m_fQuadraticAtten;
        atten[3] = flashlightState.m_FarZ;
        s_pShaderAPI->SetPixelShaderConstant( PSREG_FLASHLIGHT_ATTENUATION, atten, 1 );

        SetFlashlightVertexShaderConstants( vars.m_bBump, vars.m_nBumpTransform, bDetail, vars.m_nDetailScale, bSeamless ? false : true );
    }
    Draw();
}

// Take 0..1 seed and map to (u, v) coordinate to be used in shadow filter jittering...
void CBaseVSShader::HashShadow2DJitter( const float fJitterSeed, float *fU, float* fV )
{
    const int nTexRes = 32;
    int nSeed = fmod( fJitterSeed, 1.0f ) * nTexRes * nTexRes;

    int nRow = nSeed / nTexRes;
    int nCol = nSeed % nTexRes;

    // Div and mod to get an individual texel in the fTexRes x fTexRes grid
    *fU = nRow / (float)nTexRes;   // Row
    *fV = nCol / (float)nTexRes;   // Column
}


void CBaseVSShader::DrawEqualDepthToDestAlpha( void )
{
    bool bMakeActualDrawCall = false;
    if( s_pShaderShadow ) {
        s_pShaderShadow->EnableColorWrites( false );
        s_pShaderShadow->EnableAlphaWrites( true );
        s_pShaderShadow->EnableDepthWrites( false );
        s_pShaderShadow->EnableAlphaTest( false );
        s_pShaderShadow->EnableBlending( false );

        s_pShaderShadow->DepthFunc( SHADER_DEPTHFUNC_EQUAL );

        s_pShaderShadow->SetVertexShader( "depthtodestalpha_vs20", 0 );
        s_pShaderShadow->SetPixelShader( "depthtodestalpha_ps20b", 0 );
    }
    if( s_pShaderAPI ) {
        s_pShaderAPI->SetVertexShaderIndex( 0 );
        s_pShaderAPI->SetPixelShaderIndex( 0 );

        bMakeActualDrawCall = s_pShaderAPI->ShouldWriteDepthToDestAlpha();
    }
    Draw( bMakeActualDrawCall );
}

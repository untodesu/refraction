// PBR shader.
// Originates from https://github.com/thexa4/source-pbr
// Adopted for Refraction. Things done so far:
// 1. Fixed SP branch crash (See PSREG_FREE).
// 2. Removed all the verbosity in the comments
//    leaving only stuff that explains obfuscated stuff.

#include "BaseVSShader.h"
#include "cpp_shader_constant_register_map.h"
#include "pbr_vs30.inc"
#include "pbr_ps30.inc"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

const Sampler_t SAMPLER_BASETEXTURE     = SHADER_SAMPLER0;
const Sampler_t SAMPLER_NORMAL          = SHADER_SAMPLER1;
const Sampler_t SAMPLER_ENVMAP          = SHADER_SAMPLER2;
const Sampler_t SAMPLER_SHADOWDEPTH     = SHADER_SAMPLER4;
const Sampler_t SAMPLER_RANDOMROTATION  = SHADER_SAMPLER5;
const Sampler_t SAMPLER_FLASHLIGHT      = SHADER_SAMPLER6;
const Sampler_t SAMPLER_LIGHTMAP        = SHADER_SAMPLER7;
const Sampler_t SAMPLER_MRAO            = SHADER_SAMPLER10;
const Sampler_t SAMPLER_EMISSIVE        = SHADER_SAMPLER11;
const Sampler_t SAMPLER_SPECULAR        = SHADER_SAMPLER12;

static ConVarRef mat_fullbright( "mat_fullbright", false );
static ConVarRef mat_specular( "mat_specular", false );
static ConVar mat_pbr_parallaxmap( "mat_pbr_parallaxmap", "1" );

struct PBR_Vars_t {
    PBR_Vars_t()
    {
        memset( this, 0xFF, sizeof( *this ) );
    }

    int baseTexture;
    int baseColor;
    int bumpMap;
    int envMap;
    int baseTextureFrame;
    int baseTextureTransform;
    int useParallax;
    int parallaxDepth;
    int parallaxCenter;
    int alphaTestReference;
    int flashlightTexture;
    int flashlightTextureFrame;
    int emissionTexture;
    int mraoTexture;
    int useEnvAmbient;
    int specularTexture;
};

BEGIN_VS_SHADER( PBR, "Help for PBR shader" )
    BEGIN_SHADER_PARAMS;
        SHADER_PARAM( ALPHATESTREFERENCE,   SHADER_PARAM_TYPE_FLOAT,    "0",        "" )
        SHADER_PARAM( ENVMAP,               SHADER_PARAM_TYPE_ENVMAP,   "",         "" )
        SHADER_PARAM( MRAOTEXTURE,          SHADER_PARAM_TYPE_TEXTURE,  "",         "" )
        SHADER_PARAM( EMISSIONTEXTURE,      SHADER_PARAM_TYPE_TEXTURE,  "",         "" )
        SHADER_PARAM( BUMPMAP,              SHADER_PARAM_TYPE_TEXTURE,  "",         "" )
        SHADER_PARAM( USEENVAMBIENT,        SHADER_PARAM_TYPE_BOOL,     "0",        "" )
        SHADER_PARAM( SPECULARTEXTURE,      SHADER_PARAM_TYPE_TEXTURE,  "",         "" )
        SHADER_PARAM( PARALLAX,             SHADER_PARAM_TYPE_BOOL,     "0",        "" )
        SHADER_PARAM( PARALLAXDEPTH,        SHADER_PARAM_TYPE_FLOAT,    "0.0030",   "" )
        SHADER_PARAM( PARALLAXCENTER,       SHADER_PARAM_TYPE_FLOAT,    "0.5",      "" )
    END_SHADER_PARAMS;

    void SetupVars( PBR_Vars_t &info )
    {
        info.baseTexture = BASETEXTURE;
        info.baseColor = COLOR;
        info.bumpMap = BUMPMAP;
        info.baseTextureFrame = FRAME;
        info.baseTextureTransform = BASETEXTURETRANSFORM;
        info.alphaTestReference = ALPHATESTREFERENCE;
        info.flashlightTexture = FLASHLIGHTTEXTURE;
        info.flashlightTextureFrame = FLASHLIGHTTEXTUREFRAME;
        info.envMap = ENVMAP;
        info.emissionTexture = EMISSIONTEXTURE;
        info.mraoTexture = MRAOTEXTURE;
        info.useEnvAmbient = USEENVAMBIENT;
        info.specularTexture = SPECULARTEXTURE;
        info.useParallax = PARALLAX;
        info.parallaxDepth = PARALLAXDEPTH;
        info.parallaxCenter = PARALLAXCENTER;
    }

    SHADER_FALLBACK
    {
        return 0;
    }

    SHADER_INIT
    {
        if( !params[BUMPMAP]->IsDefined() )
            params[BUMPMAP]->SetStringValue( "dev/flat_normal" );

        if( !params[MRAOTEXTURE]->IsDefined() )
            params[MRAOTEXTURE]->SetStringValue( "dev/pbr_mraotexture" );

        if( !params[ENVMAP]->IsDefined() )
            params[ENVMAP]->SetStringValue( "env_cubemap" );

        if( g_pHardwareConfig->SupportsBorderColor() )
            params[FLASHLIGHTTEXTURE]->SetStringValue( "effects/flashlight_border" );
        else
            params[FLASHLIGHTTEXTURE]->SetStringValue( "effects/flashlight001" );

        PBR_Vars_t info;
        SetupVars( info );

        LoadTexture( info.flashlightTexture, TEXTUREFLAGS_SRGB );
        LoadBumpMap( info.bumpMap );
        LoadCubeMap( info.envMap, ( g_pHardwareConfig->GetHDRType() == HDR_TYPE_NONE ? TEXTUREFLAGS_SRGB : 0 ) | TEXTUREFLAGS_ALL_MIPS );
        LoadTexture( info.mraoTexture, 0 );

        if( info.emissionTexture >= 0 && params[EMISSIONTEXTURE]->IsDefined() )
            LoadTexture( info.emissionTexture, TEXTUREFLAGS_SRGB );
        if( params[info.baseTexture]->IsDefined() )
            LoadTexture( info.baseTexture, TEXTUREFLAGS_SRGB );
        if( params[info.specularTexture]->IsDefined() )
            LoadTexture( info.specularTexture, TEXTUREFLAGS_SRGB );

        if( IS_FLAG_SET( MATERIAL_VAR_MODEL ) ) {
            SET_FLAGS2( MATERIAL_VAR2_SUPPORTS_HW_SKINNING );           // Required for skinning
            SET_FLAGS2( MATERIAL_VAR2_DIFFUSE_BUMPMAPPED_MODEL );       // Required for dynamic lighting
            SET_FLAGS2( MATERIAL_VAR2_NEEDS_TANGENT_SPACES );           // Required for dynamic lighting
            SET_FLAGS2( MATERIAL_VAR2_LIGHTING_VERTEX_LIT );            // Required for dynamic lighting
            SET_FLAGS2( MATERIAL_VAR2_NEEDS_BAKED_LIGHTING_SNAPSHOTS ); // Required for ambient cube
            SET_FLAGS2( MATERIAL_VAR2_SUPPORTS_FLASHLIGHT );            // Required for flashlight
            SET_FLAGS2( MATERIAL_VAR2_USE_FLASHLIGHT );                 // Required for flashlight
        }
        else {
            SET_FLAGS2( MATERIAL_VAR2_LIGHTING_LIGHTMAP );              // Required for lightmaps
            SET_FLAGS2( MATERIAL_VAR2_LIGHTING_BUMPED_LIGHTMAP );       // Required for lightmaps
            SET_FLAGS2( MATERIAL_VAR2_SUPPORTS_FLASHLIGHT );            // Required for flashlight
            SET_FLAGS2( MATERIAL_VAR2_USE_FLASHLIGHT );                 // Required for flashlight
        }
    }

    SHADER_DRAW
    {
        PBR_Vars_t info;
        SetupVars( info );

        const bool bAlphaTested = IS_FLAG_SET( MATERIAL_VAR_ALPHATEST );
        const bool bLightmapped = !IS_FLAG_SET( MATERIAL_VAR_MODEL );
        const bool bHasBaseTexture = params[info.baseTexture]->IsTexture();
        const bool bHasBumpTexture = params[info.bumpMap]->IsTexture();
        const bool bHasMRAOTexture = params[info.mraoTexture]->IsTexture();
        const bool bHasEmissionTexture = params[info.emissionTexture]->IsTexture();
        const bool bHasSpecularTexture = params[info.specularTexture]->IsTexture();
        const bool bHasEnvMapTexture = params[info.envMap]->IsTexture();
        const bool bUsingFlashlight = UsingFlashlight( params );
        const bool bHasColor = params[info.baseColor]->IsDefined() && info.baseColor != -1;
        const bool bUseEnvAmbient = !!params[info.useEnvAmbient]->GetIntValue();

        BlendType_t nBlendType = EvaluateBlendRequirements( info.baseTexture, true );
        bool bFullyOpaque = ( nBlendType != BT_BLENDADD ) && ( nBlendType != BT_BLEND ) && !bAlphaTested;

        SHADOW_STATE
        {
            pShaderShadow->EnableAlphaTest( bAlphaTested );

            if( info.alphaTestReference != -1 && params[info.alphaTestReference]->GetFloatValue() > 0.0f ) {
                pShaderShadow->AlphaFunc( SHADER_ALPHAFUNC_GEQUAL, params[info.alphaTestReference]->GetFloatValue() );
            }

            if( bUsingFlashlight ) {
                // Additive blending
                pShaderShadow->EnableBlending( true );
                pShaderShadow->BlendFunc( SHADER_BLEND_ONE, SHADER_BLEND_ONE );
            }
            else
                SetDefaultBlendingShadowState( info.baseTexture, true );

            int nShadowFilterMode = bUsingFlashlight ? g_pHardwareConfig->GetShadowFilterMode() : 0;

            pShaderShadow->EnableTexture( SAMPLER_BASETEXTURE, true );
            pShaderShadow->EnableSRGBRead( SAMPLER_BASETEXTURE, true );
            pShaderShadow->EnableTexture( SAMPLER_EMISSIVE, true );
            pShaderShadow->EnableSRGBRead( SAMPLER_EMISSIVE, true );
            pShaderShadow->EnableTexture( SAMPLER_LIGHTMAP, true );
            pShaderShadow->EnableSRGBRead( SAMPLER_LIGHTMAP, false );
            pShaderShadow->EnableTexture( SAMPLER_MRAO, true );
            pShaderShadow->EnableSRGBRead( SAMPLER_MRAO, false );
            pShaderShadow->EnableTexture( SAMPLER_NORMAL, true ); 
            pShaderShadow->EnableSRGBRead( SAMPLER_NORMAL, false );
            pShaderShadow->EnableTexture( SAMPLER_SPECULAR, true );
            pShaderShadow->EnableSRGBRead( SAMPLER_SPECULAR, true );

            if( bUsingFlashlight ) {
                pShaderShadow->EnableTexture( SAMPLER_SHADOWDEPTH, true );
                pShaderShadow->SetShadowDepthFiltering( SAMPLER_SHADOWDEPTH );
                pShaderShadow->EnableSRGBRead( SAMPLER_SHADOWDEPTH, false );
                pShaderShadow->EnableTexture( SAMPLER_RANDOMROTATION, true );
                pShaderShadow->EnableTexture( SAMPLER_FLASHLIGHT, true );
                pShaderShadow->EnableSRGBRead( SAMPLER_FLASHLIGHT, true );
            }

            if( bHasEnvMapTexture ) {
                pShaderShadow->EnableTexture( SAMPLER_ENVMAP, true );
                if( g_pHardwareConfig->GetHDRType() == HDR_TYPE_NONE )
                    pShaderShadow->EnableSRGBRead( SAMPLER_ENVMAP, true );
            }

            // PS2b shaders and up write sRGB (See common_ps_fxc.h line 349)
            // und: is it true?
            pShaderShadow->EnableSRGBWrite( true );

            if( IS_FLAG_SET( MATERIAL_VAR_MODEL ) ) {
                unsigned int flags = VERTEX_POSITION | VERTEX_NORMAL | VERTEX_FORMAT_COMPRESSED;
                pShaderShadow->VertexShaderVertexFormat( flags, 1, 0, 0 );
            }
            else {
                unsigned int flags = VERTEX_POSITION | VERTEX_NORMAL;
                pShaderShadow->VertexShaderVertexFormat( flags, 3, 0, 0 );
            }

            int useParallax = params[info.useParallax]->GetIntValue();
            if( !mat_pbr_parallaxmap.GetBool() ) {
                useParallax = 0;
            }

            DECLARE_STATIC_VERTEX_SHADER( pbr_vs30 );
            SET_STATIC_VERTEX_SHADER( pbr_vs30 );

            DECLARE_STATIC_PIXEL_SHADER( pbr_ps30 );
            SET_STATIC_PIXEL_SHADER_COMBO( FLASHLIGHT, bUsingFlashlight );
            SET_STATIC_PIXEL_SHADER_COMBO( FLASHLIGHTDEPTHFILTERMODE, nShadowFilterMode );
            SET_STATIC_PIXEL_SHADER_COMBO( LIGHTMAPPED, bLightmapped );
            SET_STATIC_PIXEL_SHADER_COMBO( USEENVAMBIENT, bUseEnvAmbient );
            SET_STATIC_PIXEL_SHADER_COMBO( EMISSIVE, bHasEmissionTexture );
            SET_STATIC_PIXEL_SHADER_COMBO( SPECULAR, bHasSpecularTexture );
            SET_STATIC_PIXEL_SHADER_COMBO( PARALLAXOCCLUSION, useParallax );
            SET_STATIC_PIXEL_SHADER( pbr_ps30 );

            // I think this is correct
            DefaultFog();

            pShaderShadow->EnableAlphaWrites( bFullyOpaque );
        }
        
        DYNAMIC_STATE
        {
            bool bLightingOnly = mat_fullbright.GetInt() == 2 && !IS_FLAG_SET( MATERIAL_VAR_NO_DEBUG_OVERRIDE );

            if( bHasBaseTexture )
                BindTexture( SAMPLER_BASETEXTURE, info.baseTexture, info.baseTextureFrame );
            else
                pShaderAPI->BindStandardTexture( SAMPLER_BASETEXTURE, TEXTURE_GREY );

            Vector color;
            if( bHasColor )
                params[info.baseColor]->GetVecValue( color.Base(), 3 );
            else
                color = Vector{ 1.f, 1.f, 1.f };
            pShaderAPI->SetPixelShaderConstant( PSREG_SELFILLUMTINT, color.Base() );

            if( bHasEnvMapTexture )
                BindTexture( SAMPLER_ENVMAP, info.envMap, 0 );
            else
                pShaderAPI->BindStandardTexture( SAMPLER_ENVMAP, TEXTURE_BLACK );

            if( bHasEmissionTexture )
                BindTexture( SAMPLER_EMISSIVE, info.emissionTexture, 0 );
            else
                pShaderAPI->BindStandardTexture( SAMPLER_EMISSIVE, TEXTURE_BLACK );

            if( bHasBumpTexture )
                BindTexture( SAMPLER_NORMAL, info.bumpMap, 0 );
            else
                pShaderAPI->BindStandardTexture( SAMPLER_NORMAL, TEXTURE_NORMALMAP_FLAT );

            if( bHasMRAOTexture )
                BindTexture( SAMPLER_MRAO, info.mraoTexture, 0 );
            else
                pShaderAPI->BindStandardTexture( SAMPLER_MRAO, TEXTURE_WHITE );

            if( bHasSpecularTexture )
                BindTexture( SAMPLER_SPECULAR, info.specularTexture, 0 );
            else
                pShaderAPI->BindStandardTexture( SAMPLER_SPECULAR, TEXTURE_BLACK );

            LightState_t lightState;
            pShaderAPI->GetDX9LightState( &lightState );

            if( !IS_FLAG_SET( MATERIAL_VAR_MODEL ) ) {
                lightState.m_bAmbientLight = false;
                lightState.m_nNumLights = 0;
            }

            bool bFlashlightShadows = false;
            if( bUsingFlashlight ) {
                BindTexture( SAMPLER_FLASHLIGHT, info.flashlightTexture, info.flashlightTextureFrame );

                VMatrix worldToTexture;
                ITexture *pFlashlightDepthTexture;
                FlashlightState_t state = pShaderAPI->GetFlashlightStateEx( worldToTexture, &pFlashlightDepthTexture );
                bFlashlightShadows = state.m_bEnableShadows && pFlashlightDepthTexture;

                SetFlashLightColorFromState( state, pShaderAPI, PSREG_FLASHLIGHT_COLOR );

                if( pFlashlightDepthTexture && g_pConfig->ShadowDepthTexture() && state.m_bEnableShadows ) {
                    BindTexture( SAMPLER_SHADOWDEPTH, pFlashlightDepthTexture, 0 );
                    pShaderAPI->BindStandardTexture( SAMPLER_RANDOMROTATION, TEXTURE_SHADOW_NOISE_2D );
                }
            }

            MaterialFogMode_t fogType = pShaderAPI->GetSceneFogMode();
            int fogIndex = ( fogType == MATERIAL_FOG_LINEAR_BELOW_FOG_Z ) ? 1 : 0;

            int numBones = pShaderAPI->GetCurrentNumBones();

            bool bWriteDepthToAlpha = false;
            bool bWriteWaterFogToAlpha = false;
            if( bFullyOpaque ) {
                bWriteDepthToAlpha = pShaderAPI->ShouldWriteDepthToDestAlpha();
                bWriteWaterFogToAlpha = ( fogType == MATERIAL_FOG_LINEAR_BELOW_FOG_Z );
            }

            float vEyePos_SpecExponent[4];
            pShaderAPI->GetWorldSpaceCameraPosition( vEyePos_SpecExponent );

            int iEnvMapLOD = 6;
            auto envTexture = params[info.envMap]->GetTextureValue();
            if( envTexture ) {
                int width = envTexture->GetMappingWidth();
                int mips = 0;
                while( width >>= 1 ) mips++;
                iEnvMapLOD = mips;
            }

            iEnvMapLOD = Clamp( iEnvMapLOD, 4, 12 );

            // vEyePos_SpecExponent has some spare space
            vEyePos_SpecExponent[3] = iEnvMapLOD;
            pShaderAPI->SetPixelShaderConstant( PSREG_EYEPOS_SPEC_EXPONENT, vEyePos_SpecExponent, 1 );

            s_pShaderAPI->BindStandardTexture( SAMPLER_LIGHTMAP, TEXTURE_LIGHTMAP_BUMPED );

            DECLARE_DYNAMIC_VERTEX_SHADER( pbr_vs30 );
            SET_DYNAMIC_VERTEX_SHADER_COMBO( DOWATERFOG, fogIndex );
            SET_DYNAMIC_VERTEX_SHADER_COMBO( SKINNING, numBones > 0 );
            SET_DYNAMIC_VERTEX_SHADER_COMBO( LIGHTING_PREVIEW, pShaderAPI->GetIntRenderingParameter( INT_RENDERPARM_ENABLE_FIXED_LIGHTING ) != 0 );
            SET_DYNAMIC_VERTEX_SHADER_COMBO( COMPRESSED_VERTS, (int)vertexCompression );
            SET_DYNAMIC_VERTEX_SHADER_COMBO( NUM_LIGHTS, lightState.m_nNumLights );
            SET_DYNAMIC_VERTEX_SHADER( pbr_vs30 );

            DECLARE_DYNAMIC_PIXEL_SHADER( pbr_ps30 );
            SET_DYNAMIC_PIXEL_SHADER_COMBO( NUM_LIGHTS, lightState.m_nNumLights );
            SET_DYNAMIC_PIXEL_SHADER_COMBO( WRITEWATERFOGTODESTALPHA, bWriteWaterFogToAlpha );
            SET_DYNAMIC_PIXEL_SHADER_COMBO( WRITE_DEPTH_TO_DESTALPHA, bWriteDepthToAlpha );
            SET_DYNAMIC_PIXEL_SHADER_COMBO( PIXELFOGTYPE, pShaderAPI->GetPixelFogCombo() );
            SET_DYNAMIC_PIXEL_SHADER_COMBO( FLASHLIGHTSHADOWS, bFlashlightShadows );
            SET_DYNAMIC_PIXEL_SHADER( pbr_ps30 );

            SetVertexShaderTextureTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_0, info.baseTextureTransform );

            // This is probably important
            SetModulationPixelShaderDynamicState_LinearColorSpace( 1 );

            pShaderAPI->SetPixelShaderStateAmbientLightCube( PSREG_AMBIENT_CUBE, !lightState.m_bAmbientLight );

            pShaderAPI->CommitPixelShaderLighting( PSREG_LIGHT_INFO_ARRAY );

            // Handle mat_fullbright 2 (diffuse lighting only)
            if( bLightingOnly )
                pShaderAPI->BindStandardTexture( SAMPLER_BASETEXTURE, TEXTURE_GREY );

            // Handle mat_specular 0 (no envmap reflections)
            if( !mat_specular.GetBool() )
                pShaderAPI->BindStandardTexture( SAMPLER_ENVMAP, TEXTURE_BLACK );

            pShaderAPI->SetPixelShaderFogParams( PSREG_FOG_PARAMS );

            float modulationColor[4] = { 1.0, 1.0, 1.0, 1.0 };
            ComputeModulationColor( modulationColor );

            float flLScale = pShaderAPI->GetLightMapScaleFactor();

            modulationColor[0] *= flLScale;
            modulationColor[1] *= flLScale;
            modulationColor[2] *= flLScale;

            pShaderAPI->SetPixelShaderConstant( PSREG_DIFFUSE_MODULATION, modulationColor );

            if( bUsingFlashlight ) {
                VMatrix worldToTexture;
                float atten[4], pos[4], tweaks[4];

                const FlashlightState_t &flashlightState = pShaderAPI->GetFlashlightState( worldToTexture );
                SetFlashLightColorFromState( flashlightState, pShaderAPI, PSREG_FLASHLIGHT_COLOR );

                BindTexture( SAMPLER_FLASHLIGHT, flashlightState.m_pSpotlightTexture, flashlightState.m_nSpotlightTextureFrame );

                atten[0] = flashlightState.m_fConstantAtten;
                atten[1] = flashlightState.m_fLinearAtten;
                atten[2] = flashlightState.m_fQuadraticAtten;
                atten[3] = flashlightState.m_FarZ;

                pShaderAPI->SetPixelShaderConstant( PSREG_FLASHLIGHT_ATTENUATION, atten, 1 );

                pos[0] = flashlightState.m_vecLightOrigin[0];
                pos[1] = flashlightState.m_vecLightOrigin[1];
                pos[2] = flashlightState.m_vecLightOrigin[2];

                pShaderAPI->SetPixelShaderConstant( PSREG_FLASHLIGHT_POSITION_RIM_BOOST, pos, 1 );

                pShaderAPI->SetPixelShaderConstant( PSREG_FLASHLIGHT_TO_WORLD_TEXTURE, worldToTexture.Base(), 4 );

                tweaks[0] = ShadowFilterFromState( flashlightState );
                tweaks[1] = ShadowAttenFromState( flashlightState );

                HashShadow2DJitter( flashlightState.m_flShadowJitterSeed, &tweaks[2], &tweaks[3] );

                pShaderAPI->SetPixelShaderConstant( PSREG_ENVMAP_TINT__SHADOW_TWEAKS, tweaks, 1 );
            }

            float flParams[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

            // Parallax Depth (the strength of the effect)
            flParams[0] = GetFloatParam( info.parallaxDepth, params, 3.0f );

            // Parallax Center (the height at which it's not moved)
            flParams[1] = GetFloatParam( info.parallaxCenter, params, 3.0f );

            // und: used PSREG_FREE (c27) instead of hardcoded c40.
            // SP somehow does not support more than 32 (c31) constants. Too bad!
            pShaderAPI->SetPixelShaderConstant( PSREG_FREE, flParams, 1 );
        }

        Draw();
    }
END_SHADER

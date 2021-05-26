//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Common pixel shader code specific to flashlights
//
// $NoKeywords: $
//
//=============================================================================//
#ifndef COMMON_FLASHLIGHT_FXC_H_
#define COMMON_FLASHLIGHT_FXC_H_
#include "common_ps_fxc.h"

//
//  1   4   7   4   1
//  4   20  33  20  4
//  7   33  55  33  7
//  4   20  33  20  4
//  1   4   7   4   1
//
float DoShadowNvidiaPCF5x5Gaussian( sampler DepthSampler, const float3 shadowMapPos, const float2 vShadowTweaks )
{
    const float fEpsilonX       = vShadowTweaks.x;
    const float fTwoEpsilonX    = 2.0 * fEpsilonX;
    const float fEpsilonY       = vShadowTweaks.x;
    const float fTwoEpsilonY    = 2.0 * fEpsilonY;

    float3 shadowMapCenter_objDepth = shadowMapPos;         // Do both projections at once

    float2 shadowMapCenter = shadowMapCenter_objDepth.xy;   // Center of shadow filter
    float objDepth = shadowMapCenter_objDepth.z;            // Object depth in shadow space

    float4 vOneTaps;
    vOneTaps.x = tex2Dproj( DepthSampler, float4( shadowMapCenter + float2(  fTwoEpsilonX,  fTwoEpsilonY ), objDepth, 1 ) ).x;
    vOneTaps.y = tex2Dproj( DepthSampler, float4( shadowMapCenter + float2( -fTwoEpsilonX,  fTwoEpsilonY ), objDepth, 1 ) ).x;
    vOneTaps.z = tex2Dproj( DepthSampler, float4( shadowMapCenter + float2(  fTwoEpsilonX, -fTwoEpsilonY ), objDepth, 1 ) ).x;
    vOneTaps.w = tex2Dproj( DepthSampler, float4( shadowMapCenter + float2( -fTwoEpsilonX, -fTwoEpsilonY ), objDepth, 1 ) ).x;
    float flOneTaps = dot( vOneTaps, float4(1.0 / 331.0, 1.0 / 331.0, 1.0 / 331.0, 1.0 / 331.0));

    float4 vSevenTaps;
    vSevenTaps.x = tex2Dproj( DepthSampler, float4( shadowMapCenter + float2(  fTwoEpsilonX,  0 ), objDepth, 1 ) ).x;
    vSevenTaps.y = tex2Dproj( DepthSampler, float4( shadowMapCenter + float2( -fTwoEpsilonX,  0 ), objDepth, 1 ) ).x;
    vSevenTaps.z = tex2Dproj( DepthSampler, float4( shadowMapCenter + float2(  0, fTwoEpsilonY ), objDepth, 1 ) ).x;
    vSevenTaps.w = tex2Dproj( DepthSampler, float4( shadowMapCenter + float2(  0, -fTwoEpsilonY ), objDepth, 1 ) ).x;
    float flSevenTaps = dot( vSevenTaps, float4( 7.0 / 331.0, 7.0 / 331.0, 7.0 / 331.0, 7.0 / 331.0 ) );

    float4 vFourTapsA, vFourTapsB;
    vFourTapsA.x = tex2Dproj( DepthSampler, float4( shadowMapCenter + float2(  fTwoEpsilonX,  fEpsilonY    ), objDepth, 1 ) ).x;
    vFourTapsA.y = tex2Dproj( DepthSampler, float4( shadowMapCenter + float2(  fEpsilonX,     fTwoEpsilonY ), objDepth, 1 ) ).x;
    vFourTapsA.z = tex2Dproj( DepthSampler, float4( shadowMapCenter + float2( -fEpsilonX,     fTwoEpsilonY ), objDepth, 1 ) ).x;
    vFourTapsA.w = tex2Dproj( DepthSampler, float4( shadowMapCenter + float2( -fTwoEpsilonX,  fEpsilonY    ), objDepth, 1 ) ).x;
    vFourTapsB.x = tex2Dproj( DepthSampler, float4( shadowMapCenter + float2( -fTwoEpsilonX, -fEpsilonY    ), objDepth, 1 ) ).x;
    vFourTapsB.y = tex2Dproj( DepthSampler, float4( shadowMapCenter + float2( -fEpsilonX,    -fTwoEpsilonY ), objDepth, 1 ) ).x;
    vFourTapsB.z = tex2Dproj( DepthSampler, float4( shadowMapCenter + float2(  fEpsilonX,    -fTwoEpsilonY ), objDepth, 1 ) ).x;
    vFourTapsB.w = tex2Dproj( DepthSampler, float4( shadowMapCenter + float2(  fTwoEpsilonX, -fEpsilonY    ), objDepth, 1 ) ).x;
    float flFourTapsA = dot( vFourTapsA, float4( 4.0 / 331.0, 4.0 / 331.0, 4.0 / 331.0, 4.0 / 331.0 ) );
    float flFourTapsB = dot( vFourTapsB, float4( 4.0 / 331.0, 4.0 / 331.0, 4.0 / 331.0, 4.0 / 331.0 ) );

    float4 v20Taps;
    v20Taps.x = tex2Dproj( DepthSampler, float4( shadowMapCenter + float2(  fEpsilonX,  fEpsilonY ), objDepth, 1 ) ).x;
    v20Taps.y = tex2Dproj( DepthSampler, float4( shadowMapCenter + float2( -fEpsilonX,  fEpsilonY ), objDepth, 1 ) ).x;
    v20Taps.z = tex2Dproj( DepthSampler, float4( shadowMapCenter + float2(  fEpsilonX, -fEpsilonY ), objDepth, 1 ) ).x;
    v20Taps.w = tex2Dproj( DepthSampler, float4( shadowMapCenter + float2( -fEpsilonX, -fEpsilonY ), objDepth, 1 ) ).x;
    float fl20Taps = dot( v20Taps, float4(20.0 / 331.0, 20.0 / 331.0, 20.0 / 331.0, 20.0 / 331.0));

    float4 v33Taps;
    v33Taps.x = tex2Dproj( DepthSampler, float4( shadowMapCenter + float2(  fEpsilonX,  0 ), objDepth, 1 ) ).x;
    v33Taps.y = tex2Dproj( DepthSampler, float4( shadowMapCenter + float2( -fEpsilonX,  0 ), objDepth, 1 ) ).x;
    v33Taps.z = tex2Dproj( DepthSampler, float4( shadowMapCenter + float2(  0, fEpsilonY ), objDepth, 1 ) ).x;
    v33Taps.w = tex2Dproj( DepthSampler, float4( shadowMapCenter + float2(  0, -fEpsilonY ), objDepth, 1 ) ).x;
    float fl33Taps = dot( v33Taps, float4(33.0 / 331.0, 33.0 / 331.0, 33.0 / 331.0, 33.0 / 331.0));

    float flCenterTap = tex2Dproj( DepthSampler, float4( shadowMapCenter, objDepth, 1 ) ).x * (55.0 / 331.0);

    // Sum all 25 Taps
    return flOneTaps + flSevenTaps + flFourTapsA + flFourTapsB + fl20Taps + fl33Taps + flCenterTap;
}

float DoFlashlightShadow( sampler DepthSampler, sampler RandomRotationSampler, float3 vProjCoords, float2 vScreenPos, int nShadowLevel, float4 vShadowTweaks, bool bAllowHighQuality )
{
    //if( nShadowLevel == NVIDIA_PCF_POISSON )
    //    return DoShadowNvidiaPCF5x5Gaussian( DepthSampler, vProjCoords, float2( 1.0 / 2048.0, 1.0 / 2048.0 ) );
    //if( nShadowLevel == ATI_NOPCF )
    //    return DoShadowPoisson16Sample( DepthSampler, RandomRotationSampler, vProjCoords, vScreenPos, vShadowTweaks, false, false );
    //if( nShadowLevel == ATI_NO_PCF_FETCH4 )
    //    return DoShadowPoisson16Sample( DepthSampler, RandomRotationSampler, vProjCoords, vScreenPos, vShadowTweaks, false, true );
    //return 1.0;
    return DoShadowNvidiaPCF5x5Gaussian( DepthSampler, vProjCoords, float2( 1.0 / 2048.0, 1.0 / 2048.0 ) );
}

#endif //#ifndef COMMON_FLASHLIGHT_FXC_H_

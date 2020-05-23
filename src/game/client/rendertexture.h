//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// Define local cl_dll hooks to the renderable textures in material system
//
//=============================================================================//

#ifndef RENDERTARGETS_H
#define RENDERTARGETS_H

ITexture * GetPowerOfTwoFrameBufferTexture();
ITexture * GetFullFrameFrameBufferTexture( int textureIndex );
ITexture * GetWaterReflectionTexture();
ITexture * GetWaterRefractionTexture();
ITexture * GetFullscreenTexture();
ITexture * GetCameraTexture();
ITexture * GetFullFrameDepthTexture();

// SmallBufferHDRx=r16g16b16a16 quarter-sized texture
ITexture * GetSmallBufferHDR0();
ITexture * GetSmallBufferHDR1();

ITexture * GetSmallBuffer0();   // quarter-sized texture, same fmt as screen
ITexture * GetSmallBuffer1();   // quarter-sized texture, same fmt as screen

#define MAX_TEENY_TEXTURES 3

ITexture * GetTeenyTexture(int which);                       // tiny 32x32 texture, always 8888

void ReleaseRenderTargets( void );

#endif // RENDERTARGETS_H

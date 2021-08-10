// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// !$BUMPMAP && $DIFFUSEBUMPMAP
// $BUMPMASK && $SEAMLESS
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH

#ifndef SDK_LIGHTMAPPEDGENERIC_VS30_H
#define SDK_LIGHTMAPPEDGENERIC_VS30_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class sdk_lightmappedgeneric_vs30_Static_Index
{
	unsigned int m_nBUMPMAP : 2;
	unsigned int m_nBUMPMASK : 2;
	unsigned int m_nDIFFUSEBUMPMAP : 2;
	unsigned int m_nENVMAP_MASK : 2;
	unsigned int m_nSEAMLESS : 2;
	unsigned int m_nTANGENTSPACE : 2;
	unsigned int m_nVERTEXALPHATEXBLENDFACTOR : 2;
	unsigned int m_nVERTEXCOLOR : 2;
#ifdef _DEBUG
	bool m_bBUMPMAP : 1;
	bool m_bBUMPMASK : 1;
	bool m_bDIFFUSEBUMPMAP : 1;
	bool m_bENVMAP_MASK : 1;
	bool m_bSEAMLESS : 1;
	bool m_bTANGENTSPACE : 1;
	bool m_bVERTEXALPHATEXBLENDFACTOR : 1;
	bool m_bVERTEXCOLOR : 1;
#endif	// _DEBUG
public:
	void SetBUMPMAP( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nBUMPMAP = i;
#ifdef _DEBUG
		m_bBUMPMAP = true;
#endif	// _DEBUG
	}

	void SetBUMPMASK( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nBUMPMASK = i;
#ifdef _DEBUG
		m_bBUMPMASK = true;
#endif	// _DEBUG
	}

	void SetDIFFUSEBUMPMAP( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDIFFUSEBUMPMAP = i;
#ifdef _DEBUG
		m_bDIFFUSEBUMPMAP = true;
#endif	// _DEBUG
	}

	void SetENVMAP_MASK( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nENVMAP_MASK = i;
#ifdef _DEBUG
		m_bENVMAP_MASK = true;
#endif	// _DEBUG
	}

	void SetSEAMLESS( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nSEAMLESS = i;
#ifdef _DEBUG
		m_bSEAMLESS = true;
#endif	// _DEBUG
	}

	void SetTANGENTSPACE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nTANGENTSPACE = i;
#ifdef _DEBUG
		m_bTANGENTSPACE = true;
#endif	// _DEBUG
	}

	void SetVERTEXALPHATEXBLENDFACTOR( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nVERTEXALPHATEXBLENDFACTOR = i;
#ifdef _DEBUG
		m_bVERTEXALPHATEXBLENDFACTOR = true;
#endif	// _DEBUG
	}

	void SetVERTEXCOLOR( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nVERTEXCOLOR = i;
#ifdef _DEBUG
		m_bVERTEXCOLOR = true;
#endif	// _DEBUG
	}

	sdk_lightmappedgeneric_vs30_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nBUMPMAP = 0;
		m_nBUMPMASK = 0;
		m_nDIFFUSEBUMPMAP = 0;
		m_nENVMAP_MASK = 0;
		m_nSEAMLESS = 0;
		m_nTANGENTSPACE = 0;
		m_nVERTEXALPHATEXBLENDFACTOR = 0;
		m_nVERTEXCOLOR = 0;
#ifdef _DEBUG
		m_bBUMPMAP = false;
		m_bBUMPMASK = false;
		m_bDIFFUSEBUMPMAP = false;
		m_bENVMAP_MASK = false;
		m_bSEAMLESS = false;
		m_bTANGENTSPACE = false;
		m_bVERTEXALPHATEXBLENDFACTOR = false;
		m_bVERTEXCOLOR = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bBUMPMAP && m_bBUMPMASK && m_bDIFFUSEBUMPMAP && m_bENVMAP_MASK && m_bSEAMLESS && m_bTANGENTSPACE && m_bVERTEXALPHATEXBLENDFACTOR && m_bVERTEXCOLOR );
		return ( 8 * m_nBUMPMAP ) + ( 16 * m_nBUMPMASK ) + ( 32 * m_nDIFFUSEBUMPMAP ) + ( 64 * m_nENVMAP_MASK ) + ( 128 * m_nSEAMLESS ) + ( 256 * m_nTANGENTSPACE ) + ( 512 * m_nVERTEXALPHATEXBLENDFACTOR ) + ( 1024 * m_nVERTEXCOLOR ) + 0;
	}
};

#define shaderStaticTest_sdk_lightmappedgeneric_vs30 vsh_forgot_to_set_static_BUMPMAP + vsh_forgot_to_set_static_BUMPMASK + vsh_forgot_to_set_static_DIFFUSEBUMPMAP + vsh_forgot_to_set_static_ENVMAP_MASK + vsh_forgot_to_set_static_SEAMLESS + vsh_forgot_to_set_static_TANGENTSPACE + vsh_forgot_to_set_static_VERTEXALPHATEXBLENDFACTOR + vsh_forgot_to_set_static_VERTEXCOLOR


class sdk_lightmappedgeneric_vs30_Dynamic_Index
{
	unsigned int m_nDOWATERFOG : 2;
	unsigned int m_nFASTPATH : 2;
	unsigned int m_nLIGHTING_PREVIEW : 2;
#ifdef _DEBUG
	bool m_bDOWATERFOG : 1;
	bool m_bFASTPATH : 1;
	bool m_bLIGHTING_PREVIEW : 1;
#endif	// _DEBUG
public:
	void SetDOWATERFOG( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDOWATERFOG = i;
#ifdef _DEBUG
		m_bDOWATERFOG = true;
#endif	// _DEBUG
	}

	void SetFASTPATH( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nFASTPATH = i;
#ifdef _DEBUG
		m_bFASTPATH = true;
#endif	// _DEBUG
	}

	void SetLIGHTING_PREVIEW( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nLIGHTING_PREVIEW = i;
#ifdef _DEBUG
		m_bLIGHTING_PREVIEW = true;
#endif	// _DEBUG
	}

	sdk_lightmappedgeneric_vs30_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
		m_nDOWATERFOG = 0;
		m_nFASTPATH = 0;
		m_nLIGHTING_PREVIEW = 0;
#ifdef _DEBUG
		m_bDOWATERFOG = false;
		m_bFASTPATH = false;
		m_bLIGHTING_PREVIEW = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bDOWATERFOG && m_bFASTPATH && m_bLIGHTING_PREVIEW );
		return ( 1 * m_nDOWATERFOG ) + ( 2 * m_nFASTPATH ) + ( 4 * m_nLIGHTING_PREVIEW ) + 0;
	}
};

#define shaderDynamicTest_sdk_lightmappedgeneric_vs30 vsh_forgot_to_set_dynamic_DOWATERFOG + vsh_forgot_to_set_dynamic_FASTPATH + vsh_forgot_to_set_dynamic_LIGHTING_PREVIEW


#endif	// SDK_LIGHTMAPPEDGENERIC_VS30_H
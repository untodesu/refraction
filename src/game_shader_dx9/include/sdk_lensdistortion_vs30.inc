// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH

#ifndef SDK_LENSDISTORTION_VS30_H
#define SDK_LENSDISTORTION_VS30_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class sdk_lensdistortion_vs30_Static_Index
{
public:
	sdk_lensdistortion_vs30_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderStaticTest_sdk_lensdistortion_vs30 1


class sdk_lensdistortion_vs30_Dynamic_Index
{
public:
	sdk_lensdistortion_vs30_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderDynamicTest_sdk_lensdistortion_vs30 1


#endif	// SDK_LENSDISTORTION_VS30_H
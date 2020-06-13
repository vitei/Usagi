/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Maths/Vector2f.h"
#include "Engine/PostFX/PostFXSys.h"
#include "Engine/Graphics/GFX.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/Scene/SceneConstantSets.h"
#include "Engine/Graphics/StandardVertDecl.h"
#include "Engine/Core/stl/vector.h"
#include "Engine/Graphics/Device/GFXContext.h"
#include "ASSAO.h"


namespace usg
{

	struct ASSAOConstants
	{
	    Vector2f    ViewportPixelSize;                      // .zw == 1.0 / ViewportSize.xy
	    Vector2f    HalfViewportPixelSize;                  // .zw == 1.0 / ViewportHalfSize.xy

	    Vector2f	DepthUnpackConsts;
	    Vector2f	CameraTanHalfFOV;

	    Vector2f	NDCToViewMul;
	    Vector2f	NDCToViewAdd;

	    Vector2i	PerPassFullResCoordOffset;
	    Vector2f	PerPassFullResUVOffset;

	    Vector2f	Viewport2xPixelSize;
	    Vector2f	Viewport2xPixelSize_x_025;              // Viewport2xPixelSize * 0.25 (for fusing add+mul into mad)

	    float		EffectRadius;                           // world (viewspace) maximum size of the shadow
	    float		EffectShadowStrength;                   // global strength of the effect (0 - 5)
	    float		EffectShadowPow;
	    float		EffectShadowClamp;

	    float		EffectFadeOutMul;                       // effect fade out from distance (ex. 25)
	    float		EffectFadeOutAdd;                       // effect fade out to distance   (ex. 100)
	    float		EffectHorizonAngleThreshold;            // limit errors on slopes and caused by insufficient geometry tessellation (0.05 to 0.5)
	    float		EffectSamplRadiusNearLimitRec;          // if viewspace pixel closer than this, don't enlarge shadow sampling radius anymore (makes no sense to grow beyond some distance, not enough samples to cover everything, so just limit the shadow growth; could be SSAOSettingsFadeOutFrom * 0.1 or less)

	    float		DepthPrecisionOffsetMod;
	    float		NegRecEffectRadius;                     // -1.0 / EffectRadius
	    float		LoadCounterAvgDiv;                      // 1.0 / ( halfDepthMip[SSAO_DEPTH_MIP_LEVELS-1].sizeX * halfDepthMip[SSAO_DEPTH_MIP_LEVELS-1].sizeY )
	    float		AdaptiveSampleCountLimit;

	    float		InvSharpness;
	    int			PassIndex;
	    Vector2f	QuarterResPixelSize;                    // used for importance map only

	    Vector4f	PatternRotScaleMatrices[5];

	    float		NormalsUnpackMul;
	    float		NormalsUnpackAdd;
	    float		DetailAOStrength;
	    float		Dummy0;

#ifdef SSAO_ENABLE_NORMAL_WORLD_TO_VIEW_CONVERSION
    mat4x4                NormalsWorldToViewspaceMatrix;
#endif
	};

	static const ShaderConstantDecl g_assaoConstantDef[] = 
	{
		SHADER_CONSTANT_ELEMENT( ASSAOConstants, ViewportPixelSize,			CT_VECTOR_2, 1 ),
		SHADER_CONSTANT_ELEMENT( ASSAOConstants, HalfViewportPixelSize,		CT_VECTOR_2, 1 ),

		SHADER_CONSTANT_ELEMENT( ASSAOConstants, DepthUnpackConsts,			CT_VECTOR_2, 1 ),
		SHADER_CONSTANT_ELEMENT( ASSAOConstants, CameraTanHalfFOV,			CT_VECTOR_2, 1 ),

		SHADER_CONSTANT_ELEMENT( ASSAOConstants, NDCToViewMul,				CT_VECTOR_2, 1 ),
		SHADER_CONSTANT_ELEMENT( ASSAOConstants, NDCToViewAdd,				CT_VECTOR_2, 1 ),		

		SHADER_CONSTANT_ELEMENT( ASSAOConstants, PerPassFullResCoordOffset,	CT_VECTOR2I, 1 ),
		SHADER_CONSTANT_ELEMENT( ASSAOConstants, PerPassFullResUVOffset,	CT_VECTOR_2, 1 ),	

		SHADER_CONSTANT_ELEMENT( ASSAOConstants, Viewport2xPixelSize,		CT_VECTOR_2, 1 ),
		SHADER_CONSTANT_ELEMENT( ASSAOConstants, Viewport2xPixelSize_x_025,	CT_VECTOR_2, 1 ),

		SHADER_CONSTANT_ELEMENT( ASSAOConstants, EffectRadius,				CT_FLOAT, 1 ),
		SHADER_CONSTANT_ELEMENT( ASSAOConstants, EffectShadowStrength,		CT_FLOAT, 1 ),
		SHADER_CONSTANT_ELEMENT( ASSAOConstants, EffectShadowPow,			CT_FLOAT, 1 ),
		SHADER_CONSTANT_ELEMENT( ASSAOConstants, EffectShadowClamp,			CT_FLOAT, 1 ),

		SHADER_CONSTANT_ELEMENT( ASSAOConstants, EffectFadeOutMul,				CT_FLOAT, 1 ),
		SHADER_CONSTANT_ELEMENT( ASSAOConstants, EffectFadeOutAdd,				CT_FLOAT, 1 ),
		SHADER_CONSTANT_ELEMENT( ASSAOConstants, EffectHorizonAngleThreshold,	CT_FLOAT, 1 ),
		SHADER_CONSTANT_ELEMENT( ASSAOConstants, EffectSamplRadiusNearLimitRec,	CT_FLOAT, 1 ),

		SHADER_CONSTANT_ELEMENT( ASSAOConstants, DepthPrecisionOffsetMod,	CT_FLOAT, 1 ),
		SHADER_CONSTANT_ELEMENT( ASSAOConstants, NegRecEffectRadius,		CT_FLOAT, 1 ),
		SHADER_CONSTANT_ELEMENT( ASSAOConstants, LoadCounterAvgDiv,			CT_FLOAT, 1 ),
		SHADER_CONSTANT_ELEMENT( ASSAOConstants, AdaptiveSampleCountLimit,	CT_FLOAT, 1 ),

		SHADER_CONSTANT_ELEMENT( ASSAOConstants, InvSharpness,				CT_FLOAT, 1 ),
		SHADER_CONSTANT_ELEMENT( ASSAOConstants, PassIndex,					CT_INT, 1 ),
		SHADER_CONSTANT_ELEMENT( ASSAOConstants, QuarterResPixelSize,		CT_VECTOR_2, 1 ),

		SHADER_CONSTANT_ELEMENT( ASSAOConstants, PatternRotScaleMatrices,	CT_VECTOR_4, 5 ),

		SHADER_CONSTANT_ELEMENT( ASSAOConstants, NormalsUnpackMul,			CT_FLOAT, 1 ),
		SHADER_CONSTANT_ELEMENT( ASSAOConstants, NormalsUnpackAdd,			CT_FLOAT, 1 ),
		SHADER_CONSTANT_ELEMENT( ASSAOConstants, DetailAOStrength,			CT_FLOAT, 1 ),
		SHADER_CONSTANT_ELEMENT( ASSAOConstants, Dummy0,					CT_FLOAT, 1 ),
#ifdef SSAO_ENABLE_NORMAL_WORLD_TO_VIEW_CONVERSION
		SHADER_CONSTANT_ELEMENT( ASSAOConstants, NormalsWorldToViewspaceMatrix,			CT_MATRIX_43, 1 ),
#endif		
		SHADER_CONSTANT_END()
	};	

	ASSAO::ASSAO()
	{

	}

	ASSAO::~ASSAO()
	{

	}

	void ASSAO::Init(GFXDevice* pDevice, ResourceMgr* pResource, PostFXSys* pSys, RenderTarget* pDst)
	{
		m_constants.Init(pDevice, g_assaoConstantDef);
	}

	void ASSAO::CleanUp(GFXDevice* pDevice)
	{
		m_constants.CleanUp(pDevice);

		SamplerDecl pointDecl(SF_POINT, SC_CLAMP);
		SamplerDecl linearDecl(SF_LINEAR, SC_CLAMP);

		m_pointSampler = pDevice->GetSampler(pointDecl);
		m_linearSampler = pDevice->GetSampler(linearDecl);
	}

	void ASSAO::SetDestTarget(GFXDevice* pDevice, RenderTarget* pDst)
	{

	}

	void ASSAO::Resize(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight)
	{

	}

	void ASSAO::SetSourceTarget(GFXDevice* pDevice, RenderTarget* pTarget)
	{

	}

	bool ASSAO::Draw(GFXContext* pContext, RenderContext& renderContext)
	{
		return true;
	}


}

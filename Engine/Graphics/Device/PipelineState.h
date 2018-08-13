/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
*****************************************************************************/
#ifndef _USG_GRAPHICS_DEVICE_PIPELINE_STATE_H_
#define _USG_GRAPHICS_DEVICE_PIPELINE_STATE_H_
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Color.h"
#include "Engine/Graphics/RenderConsts.h"
#include "Engine/Graphics/Device/StateEnums.h"
#include "Engine/Graphics/Device/RenderState.h"
#include "Engine/Graphics/Device/GFXHandles.h"
#include API_HEADER(Engine/Graphics/Device, PipelineState_ps.h)

namespace usg {


	struct PipelineInitData
	{

		AlphaStateHndl				alpha;
		RasterizerStateHndl			ras;
		DepthStencilStateHndl		depth;
		PrimitiveType				ePrimType;
		SampleCount					eSampleCount;

		EffectHndl 					pEffect;
		InputBindingHndl			pBinding;
		PipelineLayoutHndl			layout;
		usg::RenderPassHndl			renderPass;

		uint64						uCmpValue;

		void InitCmpValue();
		bool operator==(const PipelineInitData& rhs) const
		{
			return (rhs.uCmpValue == uCmpValue 
				&& renderPass == rhs.renderPass
				&& pEffect == rhs.pEffect);
		}
	};

	class PipelineState
	{
	public:
		PipelineState();
		~PipelineState();

		// Set up the defaults
		void Init(GFXDevice* pDevice, const PipelineInitData& decl, uint32 uID);

		const AlphaStateHndl& GetAlphaHndl() const { return m_alphaState; }
		const DepthStencilStateHndl& GetDepthStencilHndl() const { return m_depthStencilState; }
		const RasterizerStateHndl& GetRasterizerHndl() const { return m_rasterizerState; }
		const PipelineLayoutHndl& GetPipelineLayoutHndl() const { return m_pipelineLayout; }


		bool operator==(const PipelineState& rhs) const { return ((m_uComparison == rhs.m_uComparison)); }
		bool operator!=(const PipelineState& rhs) const { return ((m_uComparison != rhs.m_uComparison)); }
		void GetStateDifferences(const PipelineState& cmp, bool& bAlpha, bool& bDepth, bool& bRas) const
		{
			bAlpha = (m_alphaState != cmp.m_alphaState);
			bDepth = m_depthStencilState != cmp.m_depthStencilState;
			bRas = m_rasterizerState != cmp.m_rasterizerState;
		}

		// For internal use only, optimization
		const AlphaState* GetAlphaStateInt() const { return m_pAlphaState; }
		const RasterizerState* GetRasterizerStateInt() const { return m_pRasterizerState; }
		const DepthStencilState* GetDepthStencilStateInt() const { return m_pDepthStencilState; }
		const InputBinding* GetInputBindingInt() const { return m_pInputBinding.GetContents(); }
		PrimitiveType GetPrimitiveType() const { return m_ePrimType; }
		EffectHndl GetEffect() const { return m_effectHndl; }
		RenderPassHndl GetRenderPass() const { return m_renderPassHndl; }

		PipelineState_ps& GetPlatform() { return m_platform; }
		const PipelineState_ps& GetPlatform() const { return m_platform; }

	private:
		PipelineState_ps		m_platform;

		AlphaStateHndl			m_alphaState;
		DepthStencilStateHndl	m_depthStencilState;
		RasterizerStateHndl		m_rasterizerState;
		PipelineLayoutHndl		m_pipelineLayout;
		PrimitiveType			m_ePrimType;
		// For fast comparisons between state groups
		union
		{
			struct
			{
				uint8 m_alphaCmp;
				uint8 m_depthCmp;
				uint8 m_rasCmp;
			};
			uint32					m_uComparison;
		};
		const AlphaState*			m_pAlphaState;
		const RasterizerState*		m_pRasterizerState;
		const DepthStencilState*	m_pDepthStencilState;
		RenderPassHndl				m_renderPassHndl;
		EffectHndl					m_effectHndl;
		InputBindingHndl			m_pInputBinding;
	};

}


#endif


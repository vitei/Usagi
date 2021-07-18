/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Interface for creating the various render states
//  FIXME: Move all of this into the graphics render context
*****************************************************************************/
#ifndef _USG_GRAPHICS_DEVICE_RENDERSTATE_MGR_H_
#define _USG_GRAPHICS_DEVICE_RENDERSTATE_MGR_H_

#include "Engine/Graphics/Color.h"
#include "Engine/Graphics/RenderConsts.h"
#include "Engine/Graphics/Device/StateEnums.h"
#include "Engine/Graphics/Device/RenderState.h"

namespace usg {


class RenderStateMgr
{
	friend class GFXDevice;
public:
	void					InitDefaults(GFXDevice* pDevice);
	void					Cleanup(GFXDevice* pDevice);

	// If NULL will return the default states
	PipelineStateHndl		GetPipelineState(const RenderPassHndl& hndl, const PipelineStateDecl& decl, GFXDevice* pDevice);
	RenderPassHndl			GetRenderPass(const RenderPassDecl& decl, GFXDevice* pDevice);
	SamplerHndl				GetSamplerState(const SamplerDecl& decl, GFXDevice* pDevice);
	DescriptorSetLayoutHndl GetDescriptorSetLayout(const DescriptorDeclaration* pDecl, GFXDevice* pDevice);
	bool					UsesBlendColor(AlphaStateHndl hndl);

	void GetPipelineStateDeclaration(const PipelineStateHndl pipeline, PipelineStateDecl& out, RenderPassHndl& passOut);
	uint32 GetColorTargetCount(const RenderPassHndl pass);

	void FinishedStaticLoad();
	void ClearDynamicResources(usg::GFXDevice* pDevice);


private:
	AlphaStateHndl			GetAlphaState(const AlphaStateDecl* pDecl, GFXDevice* pDevice);
	RasterizerStateHndl		GetRasterizerState(const RasterizerStateDecl* pDecl, GFXDevice* pDevice);
	DepthStencilStateHndl	GetDepthStencilState(const DepthStencilStateDecl* pDecl, GFXDevice* pDevice);
	PipelineLayoutHndl		GetPipelineLayout(const PipelineLayoutDecl* pDecl, GFXDevice* pDevice);
	void GetAlphaDeclaration(const AlphaStateHndl alpha, AlphaStateDecl& out);
	void GetRaserizerDeclaration(const RasterizerStateHndl raster, RasterizerStateDecl& out);
	void GetDepthStencilDeclaration(const DepthStencilStateHndl ds, DepthStencilStateDecl& out);
	void GetPipelineLayoutDeclaration(const PipelineLayoutHndl layout, PipelineLayoutDecl& out);

	InputBindingHndl GetInputBinding(GFXDevice* pDevice, const PipelineStateDecl* pDecl);
	InputBindingHndl GetInputBindingMultiStream(GFXDevice* pDevice, uint32* puDeclIds, uint32 uBufferCount);

	RenderStateMgr();
	~RenderStateMgr();


	struct PIMPL;
	PIMPL*		m_pImpl;
};

}


#endif


/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Material definition. Materials do not necessarily link to
//  a distinct effect. Rather they are a group of related variables. You may
//  add a decal to a ship, but it will never be asked to behave like water.
//	Consequently water will have it's own overloaded 
*****************************************************************************/
// FIXME: Multiple constant sets, per object and  per effect

#ifndef _USG_MATERIAL_H_
#define _USG_MATERIAL_H_
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Device/PipelineState.h"
#include "Engine/Graphics/Device/DescriptorSet.h"
#include "Engine/Core/String/U8String.h"

namespace usg {

class Texture;
class Effect;
class GFXDevice;
class GFXContext;

class Material
{
public:
	Material();
	Material(GFXDevice* pDevice, PipelineStateHndl pipelineState, const DescriptorSetLayoutHndl& descriptorDecl);
	virtual ~Material(void);

	bool Init(GFXDevice* pDevice, PipelineStateHndl pipelineState, const DescriptorSetLayoutHndl& descriptorDecl);
	bool Init(GFXDevice* pDevice, const Material& copyMat);
	void Cleanup(GFXDevice* pDevice);
	void SetName(const char* szName) { m_name = szName; }

	void SetTexture(uint32 uTex, const TextureHndl& pTex, const SamplerHndl &decl) { m_descriptorSet.SetImageSamplerPairAtBinding(uTex, pTex, decl); }
	void SetConstantSet(uint32 uConstant, const ConstantSet* pSet, uint32 uFlags = (SHADER_FLAG_VERTEX | SHADER_FLAG_GEOMETRY | SHADER_FLAG_PIXEL))
	{
		m_descriptorSet.SetConstantSetAtBinding(uConstant, pSet, 0, uFlags); 
	}

	void UpdateRenderPass(GFXDevice* pDevice, const RenderPassHndl& pass);
	void UpdateDescriptors(GFXDevice* pDevice) { m_descriptorSet.UpdateDescriptors(pDevice); }
	TextureHndl GetTexture(uint32 uBinding) const { return m_descriptorSet.GetTextureAtBinding(uBinding); }
	SamplerHndl GetSampler(uint32 uBinding) const { return m_descriptorSet.GetSamplerAtBinding(uBinding); }

	// Handle the setting of the effect, textures, constant buffers etc
	void Apply(GFXContext* pContext) const;

	void SetPipelineState(PipelineStateHndl pipelineState);
	void SetDescriptorLayout(GFXDevice* pDevice, const DescriptorSetLayoutHndl& descriptorDecl);
	void SetBlendColor(const usg::Color &color) { m_blendColor = color; }

	PipelineStateHndl GetPipelineStateHndl() const { return m_pipelineState; }

	const U8String& GetName() const { return m_name; }
	const usg::Color GetBlendColor() const { return m_blendColor; }


private:
	PRIVATIZE_COPY(Material)

	DescriptorSet			m_descriptorSet;

	Color					m_blendColor;

	PipelineStateHndl		m_pipelineState;

	U8String				m_name;
};


}


#endif

/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
*****************************************************************************/
#pragma once

#ifndef USG_GRAPHICS_DEVICE_DESCRIPTOR_SET_H
#define USG_GRAPHICS_DEVICE_DESCRIPTOR_SET_H

#include "Engine/Graphics/Device/GFXHandles.h"
#include "Engine/Resource/ResourceDecl.h"
#include "Engine/Graphics/Textures/ImageViewDef.h"
#include API_HEADER(Engine/Graphics/Device, DescriptorSet_ps.h)

namespace usg {

class Texture;
class ConstantSet;

class DescriptorSet
{
public:
	DescriptorSet();
	~DescriptorSet();

	// Set up the defaults
	void Init(GFXDevice* pDevice, const DescriptorSetLayoutHndl& layout);
	void Init(GFXDevice* pDevice, const DescriptorSet& copy);
	void CleanUp(GFXDevice* pDevice);

	void SetImageSamplerPair(uint32 uLayoutIndex, const TextureHndl& pTexture, const SamplerHndl& sampler, uint32 uSubIndex = 0, const ImageViewDef& imageView = ImageViewDef::Default());
	void SetImage(uint32 uLayoutIndex, const TextureHndl& pTexture, const ImageViewDef& imageView = ImageViewDef::Default());
	void SetConstantSet(uint32 uLayoutIndex, const ConstantSet* pBuffer, uint32 uSubIndex = 0);

	// Convenience functions so you don't have to recall the descriptor layout
	void SetImageSamplerPairAtBinding(uint32 uBinding, const TextureHndl& pTexture, const SamplerHndl& sampler, uint32 uSubIndex = 0, const ImageViewDef& imageView = ImageViewDef::Default());
	void SetImageAtBinding(uint32 uBinding, const TextureHndl& pTexture, const ImageViewDef& imageView = ImageViewDef::Default());
	void SetConstantSetAtBinding(uint32 uBinding, const ConstantSet* pBuffer, uint32 uSubIndex = 0, uint32 uFlags = SHADER_FLAG_ALL);

	// Not convinced I should allow access, but for now
	TextureHndl GetTextureAtBinding(uint32 uBinding) const;
	SamplerHndl GetSamplerAtBinding(uint32 uBinding) const;

	// Only call when finalizing or upon changing something internal
	void UpdateDescriptors(GFXDevice* pDevice);
	// Confirms update descriptors has been changed since internal data has been updated
	bool IsUptoDate() const;

	bool GetValid() const { return m_bValid; }


	// FIXME: REMOVE THESE ACCESSORS WHEN COMPLETE
	const DescriptorSetLayout*	GetLayoutDesc() const { return m_pLayoutDesc;  }
	const DescriptorSetLayoutHndl& GetLayoutHndl() const { return m_layoutHndl; }
	const DescriptorData*		GetData() const { return m_pData; }


	DescriptorSet_ps&       GetPlatform()       { return m_platform; }
	const DescriptorSet_ps& GetPlatform() const { return m_platform; }

private:
	void UpdateTimeTags();

	PRIVATIZE_COPY(DescriptorSet)

	DescriptorSet_ps			m_platform;
	DescriptorData*				m_pData;
	bool 						m_bValid;
	uint32						m_uDataCount;
	uint32						m_uLastUpdate;

	DescriptorSetLayoutHndl		m_layoutHndl;
	const DescriptorSetLayout*	m_pLayoutDesc;
};

} // namespace usagi


#endif // USG_GRAPHICS_DEVICE_DESCRIPTOR_SET_H


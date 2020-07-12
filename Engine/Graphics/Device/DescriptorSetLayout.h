/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
*****************************************************************************/
#ifndef _USG_GRAPHICS_DEVICE_DESCRIPTOR_SET_LAYOUT_H_
#define _USG_GRAPHICS_DEVICE_DESCRIPTOR_SET_LAYOUT_H_

#include "Engine/Graphics/RenderConsts.h"
#include "Engine/Graphics/Device/GFXHandles.h"
#include API_HEADER(Engine/Graphics/Device, DescriptorSetLayout_ps.h)

namespace usg {

class DescriptorSetLayoutDecl;

class DescriptorSetLayout
{
public:
	DescriptorSetLayout();
	~DescriptorSetLayout();

	// Set up the defaults
	void Init(GFXDevice* pDevice, const DescriptorSetLayoutDecl& declaration, uint32 uID);

	const DescriptorDeclaration* GetDeclaration(uint32 uIndex) const { ASSERT(uIndex < m_uDecls); return &m_pDeclInfo[uIndex].declaration; }
	uint32 GetResourceIndex(uint32 uDecl, uint32 uSubResource) const;
	uint32 GetDeclarationCount() const { return m_uDecls; }
	uint32 GetResourceCount() const { return m_uResources; }
	uint32 GetNumberOfType(DescriptorType eType) const { return m_uTypedResources[eType]; }

	DescriptorSetLayout_ps& GetPlatform() { return m_platform; }

private:
	struct DeclarationInfo
	{
		DescriptorDeclaration	declaration;
		uint32					uResourceOffset;
	};

	DescriptorSetLayout_ps	m_platform;
	DeclarationInfo*		m_pDeclInfo;
	uint32					m_uResources;
	uint32					m_uTypedResources[DESCRIPTOR_TYPE_INVALID];
	uint32					m_uDecls;
};

}


#endif


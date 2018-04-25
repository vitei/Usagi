/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Memory/MemUtil.h"
#include "Engine/Graphics/RenderConsts.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Graphics/Device/DescriptorSetLayoutDecl.h"
#include "Engine/Graphics/Device/DescriptorSetLayout.h"

namespace usg {

DescriptorSetLayout::DescriptorSetLayout()
{
	m_pDeclInfo = NULL;
	m_uResources = 0;
	m_uDecls = 0;
	for (uint32 i = 0; i < DESCRIPTOR_TYPE_CONSTANT_BUFFER; i++)
	{
		m_uTypedResources[i] = 0;
	}
}

DescriptorSetLayout::~DescriptorSetLayout()
{
	if (m_pDeclInfo)
	{
		vdelete[] m_pDeclInfo;
		m_pDeclInfo = NULL;
	}
}

void DescriptorSetLayout::Init(GFXDevice* pDevice, const DescriptorSetLayoutDecl& declaration, uint32 uID)
{
	ASSERT(m_pDeclInfo==NULL);
	const DescriptorDeclaration* pDeclaration = declaration.GetElements();
	while (pDeclaration->eDescriptorType != DESCRIPTOR_TYPE_INVALID)
	{
		m_uResources += pDeclaration->uCount;
		m_uTypedResources[pDeclaration->eDescriptorType] += pDeclaration->uCount;
		pDeclaration++;
	}

	m_uDecls = declaration.GetElementCount();
	pDeclaration = declaration.GetElements();

	uint32 uResourceOffset = 0;
	m_pDeclInfo = vnew(ALLOC_OBJECT) DeclarationInfo[m_uDecls];
	for (uint32 i = 0; i < m_uDecls; i++)
	{
		m_pDeclInfo[i].declaration = pDeclaration[i];
		m_pDeclInfo[i].uResourceOffset = uResourceOffset;
		uResourceOffset += pDeclaration[i].uCount;
	}

	m_platform.Init(pDevice, *this);
}

uint32 DescriptorSetLayout::GetResourceIndex(uint32 uDecl, uint32 uSubResource) const
{
	ASSERT(uDecl < m_uDecls);
	const DeclarationInfo* pDecl = &m_pDeclInfo[uDecl];

	ASSERT(uSubResource < pDecl->declaration.uCount);
	return pDecl->uResourceOffset + uSubResource;
}

}

/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
*****************************************************************************/
#ifndef _USG_GRAPHICS_DEVICE_DESCRIPTOR_SET_LAYOUT_DECL_H_
#define _USG_GRAPHICS_DEVICE_DESCRIPTOR_SET_LAYOUT_DECL_H_
#include "Engine/Common/Common.h"
#include "Engine/Graphics/RenderConsts.h"
#include "Engine/Graphics/Device/GFXHandles.h"

namespace usg {

// We pass in a direct pointer when setting it up to avoid a copy, but the copy constructor allocates the necessary memory
class DescriptorSetLayoutDecl
{
public:
	DescriptorSetLayoutDecl() { m_uElementCount = 0; }
	~DescriptorSetLayoutDecl() {}
	
	void Init(const DescriptorDeclaration* pElement)
	{
		m_uElementCount = 0;
		while (pElement->eDescriptorType != DESCRIPTOR_TYPE_INVALID)
		{
			m_elements[m_uElementCount] = *pElement;
			m_uElementCount++;
			pElement++;
		}

		m_elements[m_uElementCount] = DESCRIPTOR_CAP;
	}
	bool operator ==(const DescriptorSetLayoutDecl& rhs) const
	{
		if (m_uElementCount != rhs.m_uElementCount)
		{
			return false;
		}
		for (uint32 i = 0; i < m_uElementCount; i++)
		{	
			// TODO: A more intelligent system
			if (m_elements[i].eDescriptorType != rhs.m_elements[i].eDescriptorType
				|| m_elements[i].shaderType != rhs.m_elements[i].shaderType
				|| m_elements[i].uBinding != rhs.m_elements[i].uBinding
				|| m_elements[i].uCount != rhs.m_elements[i].uCount)
			{
				return false;
			}
		}
		return true;
	}

	uint32 GetElementCount() const { return m_uElementCount; }
	const DescriptorDeclaration* GetElements() const { return m_elements; }

private:
	enum
	{
		MAX_ELEMENTS = 20	// Enough for all the textures and constant buffers
	};
	DescriptorDeclaration	m_elements[MAX_ELEMENTS];
	uint32					m_uElementCount;
};

}


#endif


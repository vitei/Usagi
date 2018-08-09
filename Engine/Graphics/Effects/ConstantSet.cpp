/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/GFX.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "ConstantSet.h"

namespace usg {


ConstantSet::ConstantSet()
{
	m_bLocked	= false;
	m_uSize		= 0;
	m_pCPUData	= NULL;
	m_bDirty	= false;
	m_uVarCount = 0;
	m_uLastUpdate = USG_INVALID_ID;
}

ConstantSet::~ConstantSet()
{
	if(m_pCPUData)
	{
		mem::Free(MEMTYPE_STANDARD, m_pCPUData);
	}
}


void ConstantSet::AppendDeclaration(const ShaderConstantDecl* pDecl)
{
	while(pDecl->eType != CT_INVALID)
	{
		if(pDecl->eType==CT_STRUCT)
		{
			for(uint32 i=0; i<pDecl->uiCount; i++)
			{
				// Recursive, theoeritcally could have structs within structs
				AppendDeclaration(pDecl->pSubDecl);
			}
		}
		else
		{
			uint32 uAlign = g_uConstantCPUAllignment[pDecl->eType];
			m_uSize = (m_uSize + uAlign - 1) - ((m_uSize + uAlign - 1) % uAlign);
			m_uSize += g_uConstantSize[pDecl->eType]*pDecl->uiCount;
			m_uVarCount++;
		}
		pDecl++;
	}
}

void ConstantSet::Init(GFXDevice* pDevice, const ShaderConstantDecl* pDecl, GPUUsage eUsage, void* pData)
{
	// FIXME: This is only safe as long as it's not a dynamically loaded declaration!
	// We should pass in a flag saying whether or not to copy this data
	m_pDecl		= pDecl;
	m_uVarCount	= 0;

	AppendDeclaration(pDecl);
	
	// Accunt for CPU padding
	m_uSize = (m_uSize + 4 - 1) - ((m_uSize + 4 - 1) % 4);

	if (eUsage == GPU_USAGE_STATIC)
	{
		// Temporarily set data to be this input data
		m_pCPUData = pData;
	}
	else
	{
		m_pCPUData = (uint8*)mem::Alloc(MEMTYPE_STANDARD, ALLOC_SHADER_CONSTANTS, m_uSize, 4);
	}

	m_platform.Init(pDevice, *this, eUsage);

	if (pData)
	{
		UpdateData(pDevice);
	}

	if (eUsage == GPU_USAGE_STATIC)
	{
		m_pCPUData = nullptr;
	}

}


void ConstantSet::CleanUp(GFXDevice* pDevice)
{
	m_platform.CleanUp(pDevice);
}

void* ConstantSet::Lock(uint32 uSize)
{
	if (m_eUsage == GPU_USAGE_STATIC)
	{
		ASSERT(false);
		return nullptr;
	}
	ASSERT(m_uSize==uSize);
	m_bLocked = true;
	return m_pCPUData;
}

bool ConstantSet::UpdateData(GFXDevice* pDevice)
{
	if(m_bDirty)
	{
		bool bDoubleUpdate = false;

		if(m_uLastUpdate == pDevice->GetFrameCount())
		{
			//DEBUG_PRINT("Wastefully updating a constant buffer multiple times\n");
			bDoubleUpdate = true;
		}

		// TODO: Assert this is only performed once per frame
		m_platform.UpdateBuffer(pDevice, bDoubleUpdate);
		m_bDirty = false;
		m_uLastUpdate = pDevice->GetFrameCount();
		return true;
	}
	return false;
}

void  ConstantSet::Unlock()
{
	m_bLocked = false;
	m_bDirty = true;
}

}

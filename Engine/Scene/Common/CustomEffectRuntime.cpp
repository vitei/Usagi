/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Graphics/RenderConsts.h"
#include "Engine/Core/String/String_Util.h"
#include "Engine/Resource/CustomEffectDecl.h"
#include "Engine/Resource/CustomEffectResource.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/Graphics/Effects/ConstantSet.h"
#include "Engine/Scene/Common/CustomEffectRuntime.h"

namespace usg {

inline void SetVariableData(ConstantSet* pSet, const CustomEffectDecl::Constant* pConstant, void* pData, uint32 uSize, uint32 uIndex)
{
	uint32 uOffset = g_uConstantSize[pConstant->eConstantType] * uIndex;
	uint32 uConstSize = (g_uConstantSize[pConstant->eConstantType] * pConstant->uiCount);
	uint32 uWriteEnd = uSize + uOffset;
	if(uWriteEnd <= uConstSize)
	{
		uOffset += pConstant->uiOffset;
		uint8* pLockData = ((uint8*)pSet->Lock(pSet->GetSize()))+uOffset;
		MemCpy(pLockData, pData, uSize);
		pSet->Unlock();
	}
	else
	{
		ASSERT(false);
	}
}

CustomEffectRuntime::CustomEffectRuntime()
{
	m_uConstantSets = 0;
	m_pConstantSets = nullptr;
}


CustomEffectRuntime::~CustomEffectRuntime()
{
	ASSERT(!m_pConstantSets);
}


void CustomEffectRuntime::Init(GFXDevice* pDevice, const char* szName)
{
	m_resource = ResourceMgr::Inst()->GetCustomEffectRes(pDevice, szName);
	m_uConstantSets = m_resource->GetConstantSetCount();

	if(m_uConstantSets)
	{
		m_pConstantSets = vnew(ALLOC_OBJECT) ConstantSet[m_uConstantSets];

		for(uint32 i=0; i<m_resource->GetConstantSetCount(); i++)
		{
			m_pConstantSets[i].Init(pDevice, m_resource->GetConstantDecl(i));

			uint32 uSize = m_pConstantSets[i].GetSize();
			uint8* pLockData = (uint8*)m_pConstantSets[i].Lock(uSize);
			MemCpy(pLockData, m_resource->GetDefaultData(i), uSize);
			m_pConstantSets[i].Unlock();
		}
	}
}

void CustomEffectRuntime::Init(GFXDevice* pDevice, const CustomEffectRuntime* pCopy)
{
	m_resource = pCopy->m_resource;
	m_uConstantSets = pCopy->m_uConstantSets;
	m_pConstantSets = vnew(ALLOC_OBJECT) ConstantSet[m_uConstantSets];

	for (uint32 i = 0; i < m_uConstantSets; i++)
	{
		m_pConstantSets[i].Init(pDevice, pCopy->m_pConstantSets[i].GetDeclaration());
		void* pDest = m_pConstantSets[i].Lock(m_pConstantSets[i].GetSize());
		MemCpy(pDest, pCopy->m_pConstantSets[i].GetCPUData(), m_pConstantSets[i].GetSize());
		m_pConstantSets[i].Unlock();
		m_pConstantSets[i].UpdateData(pDevice);
	}
	
}

void CustomEffectRuntime::CleanUp(GFXDevice* pDevice)
{
	if (m_pConstantSets)
	{
		for (uint32 i = 0; i < m_uConstantSets; i++)
		{
			m_pConstantSets[i].CleanUp(pDevice);
		}
		vdelete[] m_pConstantSets;
		m_pConstantSets = nullptr;
	}
}

void CustomEffectRuntime::SetSetData(uint32 uSet, void* pData, uint32 uSize)
{
	void* pDst = m_pConstantSets[uSet].Lock(uSize);
	usg::MemCpy(pDst, pData, uSize);
	m_pConstantSets[uSet].Unlock();
}

uint32 CustomEffectRuntime::GetVariableCount(const char* szName) const
{
	for (uint32 uSet = 0; uSet < m_resource->GetConstantSetCount(); uSet++)
	{
		uint32 uSubAttribs = m_resource->GetConstantCount(uSet);
		for (uint32 uConstant = 0; uConstant < uSubAttribs; uConstant++)
		{
			const CustomEffectDecl::Constant* pConstant = m_resource->GetConstant(uSet, uConstant);
			if (str::Compare(szName, pConstant->szName))
			{
				return pConstant->uiCount;
			}
		}
	}
	return 0;
}

bool CustomEffectRuntime::SetVariable(const char* szName, void* pData, uint32 uSize, uint32 uStartId)
{
	bool bFound = false;
	for(uint32 uSet=0; uSet<m_resource->GetConstantSetCount(); uSet++)
	{
		uint32 uSubAttribs = m_resource->GetConstantCount(uSet);
		for(uint32 uConstant=0; uConstant<uSubAttribs; uConstant++)
		{
			const CustomEffectDecl::Constant* pConstant = m_resource->GetConstant(uSet, uConstant);
			if( str::Compare(szName, pConstant->szName) )
			{
				SetVariableData(&m_pConstantSets[uSet], pConstant, pData, uSize, uStartId);
				bFound = true;
			}
		}
	}
	return bFound;
}

void CustomEffectRuntime::GPUUpdate(GFXDevice* pDevice)
{
	for(uint32 uSet=0; uSet<m_uConstantSets; uSet++)
	{
		m_pConstantSets[uSet].UpdateData(pDevice);
	}
}

ConstantSet* CustomEffectRuntime::GetConstantSet(uint32 uSet)
{
	return &m_pConstantSets[uSet];
}

const ConstantSet* CustomEffectRuntime::GetConstantSet(uint32 uSet) const
{
	return &m_pConstantSets[uSet];
}

}

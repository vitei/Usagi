/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Memory/MemAllocator.h"

namespace usg {

MemAllocator::MemAllocator()
{
	m_uSize		= 0;
	m_uAlign	= 0;
	m_pData		= NULL;
	m_bGPUData	= false;
}

MemAllocator::~MemAllocator()
{
	// The destructor should not be called whilst the point is still valid
	ASSERT(m_pData==NULL);
}

void MemAllocator::Init(uint32 uSize, uint32 uAlign, bool bGPUData)
{
	m_uSize		= uSize;
	m_uAlign	= uAlign;
	m_bGPUData	= bGPUData;
}

void MemAllocator::Allocated(void* pData)
{
	m_pData = pData;
	AllocatedCallback(pData);
}


void MemAllocator::Released()
{
	ReleasedCallback();
	m_pData = NULL;
}

}


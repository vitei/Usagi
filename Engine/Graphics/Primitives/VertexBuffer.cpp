/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Primitives/VertexBuffer.h"

namespace usg {

void VertexBuffer::CleanUp(GFXDevice* pDevice)
{
	m_platform.CleanUp(pDevice);
}

VertexBuffer::Lock::Lock()
{
	Invalidate();
}

VertexBuffer::Lock::~Lock()
{
	if(m_pOwner)
	{
		// Some idiot forgot to unlock this
		m_pOwner->GetPlatform().UnlockData(m_pDevice, m_pData, m_uSize);
	}
	m_pOwner = nullptr;
}

void VertexBuffer::Lock::Init(GFXDevice* pDevice, VertexBuffer* pOwner, void* const pData, uint32 uSize)
{
	ASSERT(m_pOwner==nullptr);
	m_pOwner = pOwner;
	m_pData = pData;
	m_uSize = uSize;
	m_pDevice = pDevice;
}

void VertexBuffer::Lock::Invalidate()
{
	m_pOwner = nullptr;
	m_uSize = 0;
	m_pData = nullptr;
	m_pDevice = nullptr;
}

void VertexBuffer::LockData(GFXDevice* pDevice, uint32 uVertCount, Lock& lockOut)
{
	uVertCount = uVertCount > 0 ?  uVertCount : m_uVertCount;
	lockOut.Init(pDevice, this, m_platform.LockData(pDevice, uVertCount*m_uVertSize), uVertCount );
}

void VertexBuffer::Unlock(Lock& lock)
{
	if(lock.GetData() && lock.GetOwner() == this)
	{
		m_platform.UnlockData(lock.GetDevice(), lock.GetData(), lock.GetSize());
		lock.Invalidate();
	}
}

}

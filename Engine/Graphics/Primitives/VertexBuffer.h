/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#pragma once

#ifndef USG_GRAPHICS_VERTEXBUFFER_H
#define USG_GRAPHICS_VERTEXBUFFER_H

#include "Engine/Graphics/Primitives/VertexDeclaration.h"
#include API_HEADER(Engine/Graphics/Primitives, VertexBuffer_ps.h)

namespace usg {

class Effect;

class VertexBuffer
{
public:
	VertexBuffer()
		: m_platform()
		, m_uVertCount(0)
		, m_uVertSize(0)
		, m_bStatic(true)
		, m_pszName(nullptr)
	{
	}

	~VertexBuffer() {}

	// It's getting too inefficient to require all the data to orderer correctly (as nice and easy as it was to keep it platform
	// independent, so here's a lock whose destructor will force an update
	class Lock
	{
		friend class VertexBuffer;
	public:
		Lock();
		~Lock();

		void* GetData() { return m_pData; }
		uint32 GetSize() { return m_uSize; }
		
	private:
		void Init(GFXDevice* pDevice, VertexBuffer* pOwner, void* const pData, uint32 uSize);
		void Invalidate();
		VertexBuffer* GetOwner() { return m_pOwner; }
		GFXDevice* GetDevice() { return m_pDevice; }

		GFXDevice*		m_pDevice;
		VertexBuffer*	m_pOwner;
		uint32			m_uSize;
		void*			m_pData;
	};

	void Init(GFXDevice* pDevice, const void* const pVerts, uint32 uVertSize, uint32 uVertCount, const char* pszName, GPUUsage eUsage = GPU_USAGE_STATIC, GPULocation eLocation = GPU_LOCATION_FASTMEM);
	void Cleanup(GFXDevice* pDevice);
	uint32 GetCount() const { return m_uVertCount; }
	uint32 GetVertSize() const { return m_uVertSize; }
	// Specify zero to copy the entire buffer
	void SetContents(GFXDevice* pDevice, const void* const pData, uint32 uVertCount = 0);
	void LockData(GFXDevice* pDevice, uint32 uVertCount, Lock& lockOut);
	void Unlock(Lock& lock);

	const VertexBuffer_ps& GetPlatform() const { return m_platform; }
	VertexBuffer_ps& GetPlatform() { return m_platform; }

	const char*			GetName() const { return m_pszName; }

private:
	VertexBuffer_ps	m_platform;
	uint32			m_uVertCount;
	uint32			m_uVertSize;
	bool			m_bStatic;
	const char*		m_pszName;
};

inline void VertexBuffer::Init(GFXDevice* pDevice, const void* const pVerts, uint32 uVertSize, uint32 uVertCount, const char* pszName, GPUUsage eUsage, GPULocation eLocation)
{ 
	m_uVertCount = uVertCount;
	m_uVertSize = uVertSize;
	m_bStatic = eUsage == GPU_USAGE_STATIC;
	m_pszName = pszName;
	ASSERT( !m_bStatic || pVerts!=NULL );
	m_platform.Init(pDevice, pVerts, uVertSize*uVertCount, pszName, eUsage, eLocation);
}

inline void VertexBuffer::SetContents(GFXDevice* pDevice, const void* const pData, uint32 uVertCount)
{
	ASSERT(m_bStatic == false);
	ASSERT(uVertCount <= m_uVertCount);
	uVertCount = uVertCount > 0 ?  uVertCount : m_uVertCount;
	m_platform.SetContents(pDevice, pData, m_uVertSize*uVertCount);
}

} // namespace usg

#endif // USG_GRAPHICS_VERTEXBUFFER_H

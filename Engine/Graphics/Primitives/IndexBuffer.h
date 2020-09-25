/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#pragma once

#ifndef USG_GRAPHICS_INDEXBUFFER_H
#define USG_GRAPHICS_INDEXBUFFER_H

#include API_HEADER(Engine/Graphics/Primitives, IndexBuffer_ps.h)

namespace usg {

class IndexBuffer
{
public:
	IndexBuffer()
	: m_platform()
	, m_uCount(0)
	, m_bStatic(true)
	{
	}

	~IndexBuffer() {}

	// FIXME: Replace static with GPUUsage
	void Init(GFXDevice* pDevice, const uint8* const pIndices, uint32 uCount, bool bStatic = true, GPULocation eLocation = GPU_LOCATION_FASTMEM);
	void Init(GFXDevice* pDevice, const uint16* const pIndices, uint32 uCount, bool bStatic = true, GPULocation eLocation = GPU_LOCATION_FASTMEM);
	void Init(GFXDevice* pDevice, const uint32* const pIndices, uint32 uCount, bool bStatic = true, GPULocation eLocation = GPU_LOCATION_FASTMEM);

	void InitSize(GFXDevice* pDevice, const void* const pIndices, uint32 uSize, uint32 uCount, bool bStatic = true, GPULocation eLocation = GPU_LOCATION_FASTMEM);
	void Cleanup(GFXDevice* pDevice);

	void SetContents(GFXDevice* pDevice, const void* const pData, uint32 uIndexCount = 0);

	uint32 GetIndexCount() const { return m_uCount; }
	const IndexBuffer_ps& GetPlatform() const { return m_platform; }

private:
	IndexBuffer_ps	m_platform;
	uint32			m_uCount;
	bool            m_bStatic;
};

} // namespace usg

#endif // USG_GRAPHICS_INDEXBUFFER_H

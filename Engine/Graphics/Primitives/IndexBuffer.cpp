#include "Engine/Common/Common.h"
#include "Engine/Graphics/Primitives/IndexBuffer.h"

namespace usg {


void IndexBuffer::Init(GFXDevice* pDevice, const uint8* const pIndices, uint32 uCount, bool bStatic, GPULocation eLocation)
{
	ASSERT(!bStatic || pIndices != nullptr);
	m_uCount = uCount;
	m_bStatic = bStatic;

	m_platform.Init(pDevice, pIndices, uCount, 1, bStatic, eLocation);
}

void IndexBuffer::Init(GFXDevice* pDevice, const uint16* const pIndices, uint32 uCount, bool bStatic, GPULocation eLocation)
{
	ASSERT(!bStatic || pIndices != nullptr);
	m_uCount = uCount;
	m_bStatic = bStatic;

	m_platform.Init(pDevice, pIndices, uCount, 2, bStatic, eLocation);
}

void IndexBuffer::Init(GFXDevice* pDevice, const uint32* const pIndices, uint32 uCount, bool bStatic, GPULocation eLocation)
{
	ASSERT(!bStatic || pIndices != nullptr);
	m_uCount = uCount;
	m_bStatic = bStatic;

	m_platform.Init(pDevice, pIndices, uCount, 4, bStatic, eLocation);
}

void IndexBuffer::SetContents(GFXDevice* pDevice, const void* const pData, uint32 uIndexCount)
{
	ASSERT(m_bStatic == false);
	ASSERT(uIndexCount <= m_uCount);
	uIndexCount = (uIndexCount > 0) ? uIndexCount : m_uCount;
	m_platform.SetContents(pDevice, pData, uIndexCount);
}


void IndexBuffer::InitSize(GFXDevice* pDevice, const void* const pIndices, uint32 uSize, uint32 uCount, bool bStatic, GPULocation eLocation)
{
	switch (uSize)
	{
		// unsigned char
		case sizeof(uint8):
			{
				Init(pDevice, reinterpret_cast<const uint8* const>(pIndices), uCount, true, eLocation);
			}
			break;
		// unsigned short
		case sizeof(uint16):
			{
				Init(pDevice, reinterpret_cast<const uint16* const>(pIndices), uCount, true, eLocation);
			}
			break;
		case sizeof(uint32):
			{
				Init(pDevice, reinterpret_cast<const uint32* const>(pIndices), uCount, true, eLocation);
			}
			break;
		default:
			ASSERT(0);
			break;
	}
}

void IndexBuffer::Cleanup(GFXDevice* pDevice)
{
	m_platform.Cleanup(pDevice);
}


}

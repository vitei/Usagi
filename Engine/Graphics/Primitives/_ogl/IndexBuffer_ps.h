/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#pragma once

#ifndef USG_GRAPHICS_PC_INDEXBUFFER_H
#define USG_GRAPHICS_PC_INDEXBUFFER_H

#include "Engine/Common/Common.h"
#include "Engine/Graphics/RenderConsts.h"
#include OS_HEADER(Engine/Graphics/Device, OpenGLIncludes.h)

namespace usg {

class GFXDevice;

class IndexBuffer_ps
{
	public:
		IndexBuffer_ps();
		~IndexBuffer_ps();

		void Init(GFXDevice* pDevice, const void* pIndices, uint32 uCount, uint32 uIndexSize, bool bStatic, GPULocation eLocation);
		void CleanUp(GFXDevice* pDevice);
		void SetContents(GFXDevice* pDevice, const void* pData, uint32 uIndexCount);

		GLenum GetType     () const { return m_type; }
		uint32 GetCount    () const { return m_uCount; }
		uint32 GetBuffer   () const { return m_IBO[m_uActiveIBO]; }
		uint32 GetIndexSize() const { return m_uIndexSize; }

	private:
		GLuint m_IBO[GFX_NUM_DYN_BUFF];     
		uint32 m_uActiveIBO;
		uint32 m_uBufferCount;
		GLenum m_type;
		uint32 m_uElementCount;
		uint32 m_uCount;
		uint32 m_uIndexSize;
};

} // namespace usg

#endif // USG_GRAPHICS_PC_INDEXBUFFER_H

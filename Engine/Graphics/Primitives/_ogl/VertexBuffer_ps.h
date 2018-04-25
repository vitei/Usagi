/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#pragma once

#ifndef USG_GRAPHICS_PC_VERTEXBUFFER_H
#define USG_GRAPHICS_PC_VERTEXBUFFER_H
#include "Engine/Common/Common.h"
#include OS_HEADER(Engine/Graphics/Device, OpenGLIncludes.h)

namespace usg {

class Effect;
class GFXDevice;

class VertexBuffer_ps
{
public:
	VertexBuffer_ps();
	~VertexBuffer_ps();

	void Init(GFXDevice* pDevice, const void* const pVerts, uint32 uDataSize, GPUUsage eUpdateType, GPULocation eLocation);
	void CleanUp(GFXDevice* pDevice);
	void SetContents(GFXDevice* pDevice, const void* const pData, uint32 uSize);

	GLuint GetBuffer() const { return m_VBO[m_uActiveVBO]; }

	void*	LockData(GFXDevice* pDevice, uint32 uSize);
	void	UnlockData(GFXDevice* pDevice, void* pData, uint32 uSize);
private:
	
	GLuint						m_VBO[GFX_NUM_DYN_BUFF];
	uint32						m_uActiveVBO;
	uint32						m_uBufferCount;
};

} // namespace usg

#endif // USG_GRAPHICS_PC_VERTEXBUFFER_H

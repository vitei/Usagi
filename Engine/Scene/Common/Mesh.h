/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: A basic mesh
*****************************************************************************/
#ifndef _USG_GRAPHICS_SCENE_MESH_H_
#define _USG_GRAPHICS_SCENE_MESH_H_

#include "Engine/Graphics/Materials/Material.h"
#include "Engine/Graphics/Primitives/VertexBuffer.h"
#include "Engine/Graphics/Primitives/IndexBuffer.h"
#include "Engine/Scene/RenderNode.h"

namespace usg{

class GFXDevice;
class Camera;

class Mesh : public RenderNode
{
public:
	Mesh();
	virtual ~Mesh();
    
	virtual bool Draw(GFXContext* pContext, RenderContext& renderContext) override;
	virtual void RenderPassChanged(GFXDevice* pDevice, uint32 uContextId, const RenderPassHndl &renderPass, const SceneRenderPasses& passes) override;
    
	void SetPipeline(const PipelineStateHndl& hndl) { m_pipeline = hndl; }
	DescriptorSet& GetDescriptorSet() { return m_descriptors;  }
    IndexBuffer& GetIndexBuffer() { return m_indexBuffer; }
    VertexBuffer& GetVertexBuffer() { return m_vertexBuffer; }
    void SetName(const char* pszName);
	void SetMaxCount(uint32 uCount) { m_uMaxCount = uCount; }

private:
	const char*				m_pszName;    
	PipelineStateHndl		m_pipeline;
	DescriptorSet			m_descriptors;
	IndexBuffer				m_indexBuffer;
	VertexBuffer			m_vertexBuffer;
	uint32 					m_uMaxCount;
	bool					m_bHasDescriptors;
};

}

#endif


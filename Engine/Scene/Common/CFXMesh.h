/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: A custom effect based mesh.
//	Not the most efficent way to draw something; but convenient
*****************************************************************************/
#pragma once

#include "Engine/Graphics/Materials/Material.h"
#include "Engine/Graphics/Primitives/VertexBuffer.h"
#include "Engine/Graphics/Primitives/IndexBuffer.h"
#include "Engine/Scene/Common/CustomEffectRuntime.h"
#include "Engine/Scene/RenderNode.h"

namespace usg{

class GFXDevice;
class Camera;


class CFXMesh : public RenderNode
{
public:
	CFXMesh();
	virtual ~CFXMesh();
    
	void Init(usg::GFXDevice* pDevice, EffectHndl hndl, RenderPassHndl renderPass, PipelineStateDecl& declInOut);
	virtual bool Draw(GFXContext* pContext, RenderContext& renderContext) override;
	virtual void RenderPassChanged(GFXDevice* pDevice, uint32 uContextId, const RenderPassHndl &renderPass, const SceneRenderPasses& passes) override;
	void GPUUpdate(usg::GFXDevice* pDevice);
	void Cleanup(usg::GFXDevice* pDevice);
    
	CustomEffectRuntime& GetCustomEffect() { return m_effect;  }
    IndexBuffer& GetIndexBuffer() { return m_indexBuffer; }
    VertexBuffer& GetVertexBuffer() { return m_vertexBuffer; }
    void SetName(const char* pszName);

private:
	const char*							m_pszName;    
	PipelineStateHndl					m_pipeline;
	CustomEffectRuntime					m_effect;
	DescriptorSet						m_descriptor;
	IndexBuffer							m_indexBuffer;
	VertexBuffer						m_vertexBuffer;
};

}



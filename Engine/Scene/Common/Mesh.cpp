/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Scene/RenderGroup.h"
#include "Engine/PostFX/PostFXSys.h"
#include "Engine/Graphics/Device/GFXContext.h"
#include "Engine/Scene/Common/Mesh.h"

namespace usg {

Mesh::Mesh()
: RenderNode()
{
	m_pszName = NULL;
	m_uMaxCount = INT_MAX;
}


Mesh::~Mesh()
{

}

void Mesh::SetName(const char* pszName)
{
	m_pszName = pszName;
}

bool Mesh::Draw(GFXContext* pContext, RenderContext& renderContext)
{
	uint32 uIndexCount = m_indexBuffer.GetIndexCount() > m_uMaxCount ? m_uMaxCount : m_indexBuffer.GetIndexCount();
	uint32 uVertexCount = m_vertexBuffer.GetCount();
	if(uIndexCount > 0 || uVertexCount > 0)
	{

		const RenderGroup* pParent = GetParent();
		//const ConstantSet* GetConstantSet();

		if(m_pszName)
			pContext->BeginGPUTag(m_pszName, Color::Blue);

		// FIXME: This has been removed from the render nodes
#if 0
		if(pParent->HasTransform())
		{
			// FIXME: Most things won't need the instance data in the GS
			pContext->SetConstantBuffer( SHADER_CONSTANT_INSTANCE, pParent->GetConstantSet(), SHADER_FLAG_VS_GS );
		}
#endif

		pContext->SetPipelineState(m_pipeline);
		if (m_descriptors.GetValid())
		{
			pContext->SetDescriptorSet(&m_descriptors, 1);
		}
		pContext->SetVertexBuffer(&m_vertexBuffer);

		if(uIndexCount > 0)
		{
			pContext->DrawIndexedEx(&m_indexBuffer, 0, uIndexCount);
		}
		else
		{
			pContext->DrawImmediate(uVertexCount);
		}

		if(m_pszName)
			pContext->EndGPUTag();

		return true;
	}

	return false;
}

void Mesh::RenderPassChanged(GFXDevice* pDevice, uint32 uContextId, const RenderPassHndl &renderPass, const SceneRenderPasses& passes)
{
	pDevice->ChangePipelineStateRenderPass(renderPass, m_pipeline);
}

}

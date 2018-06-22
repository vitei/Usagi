/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Textures/Texture.h"
#include "Engine/Graphics/Device/PipelineState.h"
#include "SceneRenderPasses.h"
#include <EASTL/sort.h>

namespace usg {


SceneRenderPasses::SceneRenderPasses()
{

}

SceneRenderPasses::~SceneRenderPasses()
{

}

void SceneRenderPasses::AddCallback(ChangeCallback callback, void* pUserData)
{
	CallbackData data;
	data.fnCallback = callback;
	data.pUserData = pUserData;
}

void SceneRenderPasses::RemoveCallback(ChangeCallback callback, void* pUserData)
{
	for (auto itr = m_callbacks.begin(); itr != m_callbacks.end(); ++itr)
	{
		if (itr->fnCallback == callback && itr->pUserData == pUserData)
		{
			m_callbacks.erase_unsorted(itr);
			return;
		}
	}
}


void SceneRenderPasses::SetRenderPass(RenderNode::Layer eLayer, uint32 uPriority, const RenderPassHndl& hndl)
{
	
	for (auto itr = m_entries.begin(); itr != m_entries.end(); ++itr)
	{
		if (itr->eLayer == eLayer && itr->uPriority == uPriority)
		{
			itr->hndl = hndl;
			return;
		}
	}

	// Not found, add it
	RenderPassEntry entry;
	entry.eLayer = eLayer;
	entry.uPriority = uPriority;
	entry.hndl = hndl;

	m_entries.push_back(entry);
}

void SceneRenderPasses::RemovePass(RenderNode::Layer eLayer, uint32 uPriority)
{
	for (auto itr = m_entries.begin(); itr != m_entries.end(); ++itr)
	{
		if (itr->eLayer == eLayer && itr->uPriority == uPriority)
		{
			m_entries.erase(itr);
			return;
		}
	}
}

void SceneRenderPasses::ClearAllPasses()
{
	m_entries.clear();
}

void SceneRenderPasses::UpdateEnd()
{
	eastl::sort(m_entries.begin(), m_entries.end());
	for (auto itr = m_callbacks.begin(); itr != m_callbacks.end(); ++itr)
	{
		itr->fnCallback(*this, itr->pUserData);
	}
}

const RenderPassHndl SceneRenderPasses::GetRenderPass(RenderNode::Layer eLayer, uint32 uPriority) const
{
	for (auto itr = m_entries.rbegin(); itr != m_entries.rend(); ++itr)
	{
		if (itr->eLayer > eLayer)
			continue;

		if (itr->eLayer <= eLayer || itr->uPriority <= uPriority)
			return itr->hndl;
	}

	ASSERT(false);
	return RenderPassHndl();
}

const RenderPassHndl SceneRenderPasses::GetRenderPass(const RenderNode& node) const
{
	return GetRenderPass(node.GetLayer(), node.GetPriority());
}


}


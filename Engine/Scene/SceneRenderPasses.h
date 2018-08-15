/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Tracks the render passes used by the current scene
*****************************************************************************/
#ifndef _USG_GRAPHICS_SCENE_RENDER_PASSES_H_
#define _USG_GRAPHICS_SCENE_RENDER_PASSES_H_
#include "Engine/Common/Common.h"
#include "Engine/Maths/Matrix4x4.h"
#include "Engine/Graphics/Device/PipelineState.h"
#include "RenderNode.h"

namespace usg
{


class SceneRenderPasses
{
public:
	typedef void(*ChangeCallback) (SceneRenderPasses& passSet, GFXDevice* pDevice, void* pUserData);

	SceneRenderPasses();
	~SceneRenderPasses();

	void AddCallback(ChangeCallback callback, void* pUserData);
	void RemoveCallback(ChangeCallback callback, void* pUserData);
	void SetRenderPass(RenderNode::Layer eLayer, uint32 uPriority, const RenderPassHndl& hndl);
	void RemovePass(RenderNode::Layer eLayer, uint32 uPriority);
	void ClearAllPasses();
	void ClearPrevPasses();
	void UpdateEnd(GFXDevice* pDevice);
	const RenderPassHndl GetRenderPass(RenderNode::Layer eLayer, uint32 uPriority, bool bPrevSet = false) const;
	const RenderPassHndl GetRenderPass(const RenderNode& node, bool bPrevSet = false) const;
	bool GetRenderPassChanged(const RenderNode& node, RenderPassHndl& hndlOut) const;
	bool RenderPassesUpdated() const { return !m_prevEntries.empty(); }

private:
	PRIVATIZE_COPY(SceneRenderPasses);

	struct RenderPassEntry
	{
		RenderNode::Layer	eLayer;
		uint32				uPriority;
		RenderPassHndl		hndl;

		bool operator<(const RenderPassEntry& rhs) const
		{
			if (eLayer < rhs.eLayer)
				return true;

			if (eLayer > rhs.eLayer)
				return false;

			return uPriority < rhs.uPriority;
		}
	};

	// Callbacks aren't necessary for render nodes connected to the scene, they will be automatically notified of changes
	struct CallbackData
	{
		ChangeCallback	fnCallback;
		void*			pUserData;
	};

	vector<RenderPassEntry>	m_entries;
	vector<RenderPassEntry>	m_prevEntries;
	vector<CallbackData>	m_callbacks;
};

}


#endif


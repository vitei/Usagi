/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: The instance of a renderable scene (may have multiple views
//	for rear view mirrors, reflections, etc
//	FIXME: A total nonsense at the moment, but will be responsible for culling etc
//	For now it's just here to ensure the interface is in place
*****************************************************************************/
#ifndef _USG_GRAPHICS_SCENE_SCENECONTEXT_H_
#define _USG_GRAPHICS_SCENE_SCENECONTEXT_H_
#include "Engine/Common/Common.h"
#include "Engine/Core/Containers/List.h"
#include "Engine/Scene/Octree.h"
#include "Engine/Scene/RenderGroup.h"

namespace usg{

class Scene;
class Camera;
class PostFXSys;
class SceneSearchObject;
class RenderGroup;


class SceneContext
{
public:
	SceneContext();
	~SceneContext();

	virtual void Update(GFXDevice* pDevice) = 0;
	virtual void ClearLists();

	virtual void InitDeviceData(GFXDevice* pDevice) = 0;
	virtual void Cleanup(GFXDevice* pDevice) = 0;
	void SetScene(Scene* pScene);

	void SetRenderMask(uint32 uRenderMask);

	void AddToDrawList(RenderGroup* pGroup);

	uint32 GetRenderMask() const { return m_uRenderMask; }
	bool IsActive() const { return m_bActive; }
	bool IsDeviceDataValid() const { return m_bDeviceDataValid; }
	void SetActive(bool bActive) { m_bActive = bActive; }
	virtual const Camera* GetCamera() const { return nullptr; }
	virtual const Sphere* GetSphere() const { return nullptr; }
	const Scene* GetScene() const { return m_pScene; }
	Scene* GetScene() { return m_pScene; }
	uint32 GetVisiblePVSCount() const { return m_uVisiblePVSCount; }
	uint32 GetVisibleGroupCount() const { return m_visibleGroups.GetSize(); }

	virtual Octree::SearchObject& GetSearchObject() = 0;

protected:
	void SetDeviceDataLoaded() { m_bDeviceDataValid = true; }
	void Init(const Camera* pCamera, uint32 uRenderMask);
	List<RenderGroup>&	GetVisibleGroups() { return m_visibleGroups; }

private:
	uint32					m_uVisiblePVSCount;
	uint32					m_uRenderMask;
	Scene*					m_pScene;
	bool					m_bActive;
	bool					m_bDeviceDataValid;

	// The visible lists for this context
	List<RenderGroup>		m_visibleGroups;
};

inline void SceneContext::AddToDrawList(RenderGroup* pGroup)
{
	#ifndef FINAL_BUILD
	if(pGroup->GetTransform()!=NULL)
		m_uVisiblePVSCount++;
	#endif
	m_visibleGroups.AddToEnd(pGroup);
}

}

#endif


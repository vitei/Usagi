/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: A context for omni directional shadow depth rendering
//	optimised to test a single sphere for visibility and render in a single pass
*****************************************************************************/
#ifndef _USG_GRAPHICS_SCENE_OMNISHADOWCONTEXT_H_
#define _USG_GRAPHICS_SCENE_OMNISHADOWCONTEXT_H_

#include "Engine/Memory/FastPool.h"
#include "Engine/Scene/SceneContext.h"
#include "Engine/Graphics/Device/DescriptorSet.h"
#include "Engine/Scene/SceneSearchObject.h"

namespace usg{

class Scene;
class Camera;
class PostFXSys;

class OmniShadowContext : public SceneContext
{
public:
	typedef SceneContext Inherited;

	OmniShadowContext();
	~OmniShadowContext();

	void Init(const Sphere* Sphere);
	virtual void InitDeviceData(GFXDevice* pDevice);
	virtual void Cleanup(GFXDevice* pDevice) override;
	virtual void Update(GFXDevice* pDevice);
	virtual const Sphere* GetSphere() const override { return m_pSphere; }
	virtual Octree::SearchObject& GetSearchObject() override { return m_searchObject; }
	virtual void ClearLists();

	void DrawScene(GFXContext* pContext);

private:
	SceneSearchSphere		m_searchObject;
	const Sphere*			m_pSphere;
	list<RenderNode*>		m_drawList;
	DescriptorSet			m_descriptorSet;
	ConstantSet				m_globalConstants;
};

}


#endif


/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: A context for shadow depth rendering
*****************************************************************************/
#ifndef _USG_GRAPHICS_SCENE_SHADOWCONTEXT_H_
#define _USG_GRAPHICS_SCENE_SHADOWCONTEXT_H_

#include "Engine/Memory/FastPool.h"
#include "Engine/Scene/SceneContext.h"
#include "Engine/Scene/SceneSearchObject.h"
#include "Engine/Graphics/Device/DescriptorSet.h"

namespace usg{

class Scene;
class Camera;
class PostFXSys;

class ShadowContext : public SceneContext
{
public:
	typedef SceneContext Inherited;

	ShadowContext();
	~ShadowContext();

	virtual void InitDeviceData(GFXDevice* pDevice);
	virtual void Cleanup(GFXDevice* pDevice) override;
	void Init(const Camera* pCamera);
	virtual void Update(GFXDevice* pDevice);
	virtual void ClearLists();
	virtual const Camera* GetCamera() const override { return m_pCamera; }
	virtual Octree::SearchObject& GetSearchObject() override { return m_searchObject; }

	void SetNonShadowFlags(uint32 uFlags);

	void DrawScene(GFXContext* pContext);

private:

	const Camera*			m_pCamera;
	SceneSearchFrustum		m_searchObject;
	list<RenderNode*>		m_drawList;
	DescriptorSet			m_descriptorSet;
	ConstantSet				m_globalConstants;
};

}


#endif


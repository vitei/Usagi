/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: A context for omni directional shadow depth rendering
//	optimised to test a single sphere for visibility and render in a single pass
*****************************************************************************/
#ifndef _USG_GRAPHICS_SCENE_OMNISHADOWCONTEXT_H_
#define _USG_GRAPHICS_SCENE_OMNISHADOWCONTEXT_H_

#include "Engine/Memory/FastPool.h"
#include "Engine/Scene/SceneContext.h"
#include "Engine/Scene/ShadowContextBase.h"
#include "Engine/Graphics/Device/DescriptorSet.h"
#include "Engine/Scene/SceneSearchObject.h"

namespace usg{

class Scene;
class Camera;
class PostFXSys;

class OmniShadowContext : public ShadowContextBase
{
public:
	typedef ShadowContextBase Inherited;

	OmniShadowContext();
	~OmniShadowContext();

	void Init(const Sphere* Sphere);
	virtual void InitDeviceData(GFXDevice* pDevice);
	virtual void Cleanup(GFXDevice* pDevice) override;
	virtual void Update(GFXDevice* pDevice);
	virtual const Sphere* GetSphere() const override { return m_pSphere; }
	virtual Octree::SearchObject& GetSearchObject() override { return m_searchObject; }

	void DrawScene(GFXContext* pContext);

protected:
	usg::Matrix4x4 GetLightMat() const;
private:
	SceneSearchSphere		m_searchObject;
	const Sphere*			m_pSphere = nullptr;

	uint32					m_uPrevDrawCRC = 0;

	DescriptorSet			m_descriptorSet;
	ConstantSet				m_globalConstants;
};

}


#endif


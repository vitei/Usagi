/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: A context for shadow depth rendering
*****************************************************************************/
#ifndef _USG_GRAPHICS_SCENE_SHADOWCONTEXT_H_
#define _USG_GRAPHICS_SCENE_SHADOWCONTEXT_H_

#include "Engine/Memory/FastPool.h"
#include "Engine/Scene/ShadowContextBase.h"
#include "Engine/Scene/SceneSearchObject.h"
#include "Engine/Graphics/Device/DescriptorSet.h"

namespace usg{

class Scene;
class Camera;
class PostFXSys;

class ShadowContext : public ShadowContextBase
{
public:
	typedef ShadowContextBase Inherited;

	ShadowContext();
	~ShadowContext();

	virtual void InitDeviceData(GFXDevice* pDevice);
	virtual void Cleanup(GFXDevice* pDevice) override;
	void Init(const Camera* pCamera);
	virtual void Update(GFXDevice* pDevice);
	virtual const Camera* GetCamera() const override { return m_pCamera; }
	virtual Octree::SearchObject& GetSearchObject() override { return m_searchObject; }

	void SetShadowFlags(uint32 uFlags);
	void SetShadowExcludeFlags(uint32 uFlags);

	void DrawScene(GFXContext* pContext);

protected:
	virtual Matrix4x4 GetLightMat() const override;

private:

	const Camera*			m_pCamera;
	SceneSearchFrustum		m_searchObject;
	DescriptorSet			m_descriptorSet;
	ConstantSet				m_globalConstants;
};

}


#endif


/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: The instance of a renderable scene (may have multiple views
//	for rear view mirrors, reflections, etc
//	FIXME: A total nonsense at the moment, but will be responsible for culling etc
//	For now it's just here to ensure the interface is in place
*****************************************************************************/
#ifndef _USG_GRAPHICS_SCENE_SCENEVIEWCONTEXT_H_
#define _USG_GRAPHICS_SCENE_SCENEVIEWCONTEXT_H_

#include "Engine/Scene/SceneContext.h"


namespace usg{

class SceneRenderPasses;
class LightingContext;
class Fog;
class GFXContext;

class ViewContext : public SceneContext
{
public:
	typedef SceneContext Inherited;

	static const DescriptorDeclaration m_sDescriptorDecl[]; 

	ViewContext();
	~ViewContext();

	virtual void InitDeviceData(GFXDevice* pDevice) override;
	virtual void Cleanup(GFXDevice* pDevice) override;
	void Init(GFXDevice* pDevice, ResourceMgr* pResMgr, PostFXSys* pFXSys, uint32 uHighestLOD = 0, uint32 uRenderMask = RenderMask::RENDER_MASK_ALL);
	virtual void Update(GFXDevice* pDevice);
	virtual void ClearLists();
	virtual const Camera* GetCamera() const override;
	virtual Octree::SearchObject& GetSearchObject() override;
	virtual bool NeedsSort() const override { return true; }

	void SetHighestLOD(uint32 uLOD);
	void SetLODBias(float fBias);
	void SetCamera(const Camera* pCamera);


	LightingContext& GetLightingContext();
	// Each view has it's own fog as the camera may be located above/ below water, or simply
	// have different draw distances
	Fog& GetFog(uint32 uIndex = 0);
	PostFXSys* GetPostFXSys();
	const SceneRenderPasses& GetRenderPasses() const;
	SceneRenderPasses& GetRenderPasses();


	void PreDraw(GFXContext* pContext, ViewType eViewType);
	void DrawScene(GFXContext* pContext);
	void SetShadowColor(usg::Color& color);


private:
	enum
	{
		MAX_FOGS = 1
	};

	struct PIMPL;
	PIMPL*						m_pImpl;

	float					m_fLODBias;
	Vector4f				m_shadowColor;
	uint32					m_uHighestLOD;
	bool					m_bFirstFrame;
};

}

#endif


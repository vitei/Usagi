/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: The instance of a renderable scene (may have multiple views
//	for rear view mirrors, reflections, etc
//	FIXME: A total nonsense at the moment, but will be responsible for culling etc
//	For now it's just here to ensure the interface is in place
*****************************************************************************/
#ifndef _USG_GRAPHICS_SCENE_SCENEVIEWCONTEXT_H_
#define _USG_GRAPHICS_SCENE_SCENEVIEWCONTEXT_H_
#include "Engine/Common/Common.h"
#include "Engine/Core/Containers/List.h"
#include "Engine/Scene/LightingContext.h"
#include "Engine/Graphics/Lights/LightMgr.h"
#include "Engine/Graphics/Fog.h"
#include "Engine/Graphics/Device/DescriptorSet.h"
#include "Engine/Scene/SceneContext.h"


namespace usg{

class SceneRenderPasses;

class ViewContext : public SceneContext
{
public:
	typedef SceneContext Inherited;

	static const DescriptorDeclaration m_sDescriptorDecl[]; 

	ViewContext();
	~ViewContext();

	virtual void InitDeviceData(GFXDevice* pDevice) override;
	virtual void Cleanup(GFXDevice* pDevice) override;
	void Init(GFXDevice* pDevice, const Camera* pCamera, PostFXSys* pFXSys, uint32 uHighestLOD = 0, uint32 uRenderMask = RenderNode::RENDER_MASK_ALL);
	virtual void Update(GFXDevice* pDevice);
	virtual void ClearLists();
	virtual const Camera* GetCamera() const override { return m_pCamera; }
	virtual Octree::SearchObject& GetSearchObject() override { return m_searchObject; }

	void SetHighestLOD(uint32 uLOD);
	void SetLODBias(float fBias);
	void SetCamera(const Camera* pCamera) { m_pCamera = pCamera; m_searchObject.SetFrustum(&pCamera->GetFrustum()); }


	LightingContext& GetLightingContext() { return m_LightingContext; }
	// Each view has it's own fog as the camera may be located above/ below water, or simply
	// have different draw distances
	Fog& GetFog(uint32 uIndex=0) { ASSERT(uIndex<MAX_FOGS); return m_fog[uIndex]; }
	PostFXSys* GetPostFXSys() { return m_pPostFXSys; }
	const SceneRenderPasses& GetRenderPasses() const;
	SceneRenderPasses& GetRenderPasses();


	void PreDraw(GFXContext* pContext, ViewType eViewType);
	void DrawScene(GFXContext* pContext);
	void SetShadowColor(usg::Color& color);


private:
	enum
	{
		MAX_NODES_PER_LAYER = 500,
		MAX_FOGS = 1
	};
	DescriptorSet			m_globalDescriptors[VIEW_COUNT];
	DescriptorSet			m_globalDescriptorsWithDepth[VIEW_COUNT];
	ConstantSet				m_globalConstants[VIEW_COUNT];
	
	const Camera*			m_pCamera;
	SceneSearchFrustum		m_searchObject;
	uint32					m_uHighestLOD;
	float					m_fLODBias;
	PostFXSys*				m_pPostFXSys;

	// Arbitrarily assigning fog to the scene context
	Fog						m_fog[MAX_FOGS];
	Vector4f				m_shadowColor;
	LightingContext			m_LightingContext;


	RenderNode*				m_pVisibleNodes[RenderNode::LAYER_COUNT][MAX_NODES_PER_LAYER];
	uint32					m_uVisibleNodes[RenderNode::LAYER_COUNT];

};

}

#endif


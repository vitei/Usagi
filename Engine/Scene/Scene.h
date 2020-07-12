/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: The renderable scene, quadtree, lights and all contained objects
*****************************************************************************/
#pragma once
#ifndef USG_GRAPHICS_SCENE_SCENE_H
#define USG_GRAPHICS_SCENE_SCENE_H

#include "Engine/Maths/Vector3f.h"
#include "Engine/Maths/Vector4f.h"
#include "Engine/Maths/Matrix4x4.h"
#include "Engine/Graphics/Device/GFXHandles.h"
#include "Engine/Debug/SceneDebugStats.h"
#include "SceneRenderPasses.h"

namespace usg {

class GFXDevice;
class AABB;
class ParticleSet;
class TransformNode;
class RenderGroup;
class ViewContext;
class ShadowContext;
class OmniShadowContext;
class OffscreenRenderNode;
class Camera;
class LightMgr;
class ParticleMgr;
template<typename T>
class List;
class SceneContext;
class ParticleEffectHndl;
class RenderGroup;

class Scene
{
public:
	Scene();
	~Scene();

	void			Init(GFXDevice* pDevice, ResourceMgr* pResMgr, const AABB& worldBounds, ParticleSet* pSet = nullptr);
	void			Cleanup(GFXDevice* pDevice);
	void			Reset();

	TransformNode*	CreateTransformNode();
	void			DeleteTransformNode(const TransformNode* pNode);
	RenderGroup*	CreateRenderGroup(const TransformNode* pNode);
	void			DeleteRenderGroup(RenderGroup* pRemove);

	ViewContext*	CreateViewContext(GFXDevice* pDevice);
	void			DeleteViewContext(ViewContext* pRemove);

	ViewContext*	GetViewContext(uint32 uId);
	uint32			GetViewContextCount() const;
	SceneRenderPasses& GetRenderPasses(uint32 uViewContext);

	
	ShadowContext*	CreateShadowContext(GFXDevice* pDevice);
	void			DeleteShadowContext(ShadowContext* pRemove);

	OmniShadowContext*	CreateOmniShadowContext(GFXDevice* pDevice);
	void			DeleteOmniShadowContext(OmniShadowContext* pRemove);

	LightMgr&		GetLightMgr();
	const LightMgr&	GetLightMgr() const;

	// TODO: Multiple views, move this code out into a view system
	void			SetSceneOffset(Vector4f vOffset) { m_vTransformOffset = vOffset; }
	void			SetSceneOffset(Vector3f vOffset) { m_vTransformOffset.Assign(vOffset, 0.0f); }
	const Vector4f& GetSceneOffset() const { return m_vTransformOffset; }

	void			ShowBounds(bool bShow) { m_bShowBounds = bShow; }
	uint32			GetPVSCount() { return m_uPVSCount; }
	void			RegisterTimers();

	void			PreUpdate();
	void			TransformUpdate(float fElapsed);
	void			PreDraw(GFXContext* pContext);
	void			Update(GFXDevice* pDevice);

	void			RegisterOffscreenRenderNode(OffscreenRenderNode* pNode);
	void			DeregisterOffscreenRenderNode(OffscreenRenderNode* pNode);

	const Camera*	GetSceneCamera(uint32 uIndex) const;

	ParticleMgr&	GetParticleMgr();
	const RenderPassHndl& GetShadowRenderPass() const;
#ifndef FINAL_BUILD
	List<SceneContext>& GetSceneContexts();
#endif

	const AABB&		GetWorldBounds() const;
	void			UpdateObject(RenderGroup* pComponent);
	void 			UpdateMask(const TransformNode* pTransform, uint32 uMask);

	void CreateEffect(const Matrix4x4& mMat, const char* szName, const Vector3f &vVelocity = Vector3f(0.0f, 0.0f, 0.f));
	void CreateScriptedEffect(const Matrix4x4& mMat, const char* szName, const Vector3f &vVelocity = Vector3f(0.0f, 0.0f, 0.f), float fScale = 1.0f);
	void CreateEffect(ParticleEffectHndl& hndlOut, const Matrix4x4& mMat, const char* szName, const Vector3f &vVelocity = Vector3f(0.0f, 0.0f, 0.f));
	void CreateScriptedEffect(ParticleEffectHndl& hndlOut, const Matrix4x4& mMat, const char* szName, const Vector3f &vVelocity = Vector3f(0.0f, 0.0f, 0.f), float fScale = 1.0f);

	enum
	{
		TIMER_VISIBILITY = 0,
		TIMER_SCENE,
		TIMER_PARTICLES,
		TIMER_PRE_CULL,
		TIMER_COUNT
	};

	uint32 GetFrame() const { return m_uFrame; }

	void SetActiveCamera(uint32 uCameraId, uint32 uViewContext);
	void AddCamera(const Camera* pCamera);
	void RemoveCamera(const Camera* pCamera);

private:
	void			PerformVisibilityTesting(GFXDevice* pDevice);
	void			UpdateSceneContexts(GFXDevice* pDevice);

	struct PIMPL;
	PIMPL*						m_pImpl;
	
	GFXDevice*					m_pDevice;
	SceneDebugStats				m_debugStats;
	uint32						m_uFrame;
	uint32						m_uPVSCount;
	bool						m_bShowBounds;
	Vector4f					m_vTransformOffset;

	Matrix4x4					m_activeView;
};

} // namespace usagi

#endif // USG_GRAPHICS_SCENE_SCENE_H
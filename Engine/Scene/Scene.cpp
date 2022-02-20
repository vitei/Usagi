/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Graphics/Lights/LightMgr.h"
#include "Engine/Memory/FastPool.h"
#include "Engine/Scene/RenderGroup.h"
#include "Engine/PostFX/PostFXSys.h"
#include "Engine/Scene/ViewContext.h"
#include "Engine/Scene/ShadowContext.h"
#include "Engine/Scene/OffscreenRenderNode.h"
#include "Engine/Scene/OmniShadowContext.h"
#include "Engine/Scene/SceneConstantSets.h"
#include "Engine/Scene/Octree.h"
#include "Engine/Particles/ParticleMgr.h"
#include "Engine/Core/Timer/ProfilingTimer.h"
#include "Engine/Scene/Camera/Camera.h"
#include "Engine/Scene/RenderNode.h"
#include "Engine/Debug/Rendering/DebugRender.h"
#include "Engine/Debug/DebugStats.h"
#include "Engine/Scene/Scene.h"
#include <future>
#include <array>

const int RENDER_GROUP_POOL_SIZE = 1000;
const int TRANSFORM_NODE_POOL_SIZE = 1000;

const int DEFAULT_VIEW_CONTEXTS = 2;
const int DEFAULT_OMNI_SHADOW_CONTEXTS = 5;
const int DEFAULT_SHADOW_CONTEXTS = 5;


namespace usg {


// We don't want the transform node being initialised from elsewhere, or children being added
// randomly
class ScTransformNode : public TransformNode
{
public:
	ScTransformNode() {}
	~ScTransformNode() {}

	virtual uint32 GetId() { return SC_TRANS_NODE_ID; }

	void PreCullUpdate() { UpdateWorldSphere(); }

	enum
	{
		SC_TRANS_NODE_ID = 0x56549549
	};

};


struct Scene::PIMPL
{
	PIMPL() :
		viewContexts(DEFAULT_VIEW_CONTEXTS, true),
		shadowContexts(DEFAULT_SHADOW_CONTEXTS, true),
		omniShadowContexts(DEFAULT_OMNI_SHADOW_CONTEXTS, true),
		sceneComponents(RENDER_GROUP_POOL_SIZE, true),
		transformNodes(TRANSFORM_NODE_POOL_SIZE, true, false)
	{

	}
	FastPool<ViewContext>		viewContexts;
	FastPool<ShadowContext>		shadowContexts;
	FastPool<OmniShadowContext>	omniShadowContexts;
	Octree						octree;
	ParticleMgr					particleSystem;
	LightMgr					lightMgr;
	FastPool<RenderGroup>		sceneComponents;
	FastPool<ScTransformNode>	transformNodes;
	list<SceneContext*>			sceneContexts;
	list<RenderGroup*>			staticComponents;
	usg::list<const Camera*>	cameras;
	usg::list<OffscreenRenderNode*> offscreenNodes;
	Debug3D						debug3D;
	ProfilingTimer				profileTimers[TIMER_COUNT];
};


static const char* g_szTimerNames[Scene::TIMER_COUNT] =
{
	"Visibility update",
	"Node update",
	"Static update",
	"Scene update",
	"Particle update",
	"Timer Pre-Cull"
};


Scene::Scene()
{
	m_uFrame = 0;
	m_bShowBounds = false;
	m_uPVSCount = 0;
	m_vTransformOffset.Assign(0.0f, 0.0f, 0.0f, 0.0f);
	m_pImpl = vnew(ALLOC_OBJECT) PIMPL;
}


Scene::~Scene()
{
	vdelete m_pImpl;
}

void Scene::Init(GFXDevice* pDevice, ResourceMgr* pResMgr, const AABB& worldBounds, ParticleSet* pSet)
{
	for(FastPool<ViewContext>::Iterator it = m_pImpl->viewContexts.EmptyBegin(); !it.IsEnd(); ++it)
	{
		(*it)->InitDeviceData(pDevice);
	}

	for (FastPool<ShadowContext>::Iterator it = m_pImpl->shadowContexts.EmptyBegin(); !it.IsEnd(); ++it)
	{
		(*it)->InitDeviceData(pDevice);
	}

	for (FastPool<OmniShadowContext>::Iterator it = m_pImpl->omniShadowContexts.EmptyBegin(); !it.IsEnd(); ++it)
	{
		(*it)->InitDeviceData(pDevice);
	}

	m_pImpl->octree.Init(worldBounds, 20, 1.2f);
	m_pImpl->lightMgr.Init(pDevice, this);
	m_pImpl->particleSystem.Init(pDevice, this, pSet);
	m_pImpl->debug3D.Init(pDevice, this, pResMgr);

	m_debugStats.Init(&m_pImpl->debug3D, this);
	DebugStats::Inst()->RegisterGroup(&m_debugStats);

	RegisterTimers();
}

const RenderPassHndl& Scene::GetShadowRenderPass() const
{
	return m_pImpl->lightMgr.GetShadowPassHndl();
}


void Scene::SetActiveCamera(uint32 uCameraId, uint32 uViewContext)
{
	if (uViewContext > GetViewContextCount())
		return;

	for (auto itr : m_pImpl->cameras)
	{
		if (itr->GetID() == uCameraId)
		{
			GetViewContext(uViewContext)->SetCamera(itr);
			GetViewContext(uViewContext)->SetRenderMask(itr->GetRenderMask());

			// We only want to show shadows for things valid in our scene
			GetLightMgr().SetShadowCastingFlags(itr->GetRenderMask() | usg::RENDER_MASK_FORCE_SHADOW);
			return;
		}
	}
}


void Scene::Cleanup(GFXDevice* pDevice)
{
	m_pImpl->debug3D.Cleanup(pDevice);
	m_debugStats.Cleanup(pDevice);
	m_pImpl->particleSystem.Cleanup(pDevice, this);
	m_pImpl->lightMgr.Cleanup(pDevice);
	ASSERT(m_pImpl->viewContexts.Size() == 0);
	for (FastPool<ViewContext>::Iterator it = m_pImpl->viewContexts.EmptyBegin(); !it.IsEnd(); ++it)
	{
		(*it)->Cleanup(pDevice);
	}

	ASSERT(m_pImpl->shadowContexts.Size() == 0);
	for (FastPool<ShadowContext>::Iterator it = m_pImpl->shadowContexts.EmptyBegin(); !it.IsEnd(); ++it)
	{
		(*it)->Cleanup(pDevice);
	}

	ASSERT(m_pImpl->omniShadowContexts.Size() == 0);
	for (FastPool<OmniShadowContext>::Iterator it = m_pImpl->omniShadowContexts.EmptyBegin(); !it.IsEnd(); ++it)
	{
		(*it)->Cleanup(pDevice);
	}
}

void Scene::RegisterTimers()
{
#ifndef FINAL_BUILD
	for (int i = 0; i < TIMER_COUNT; i++)
	{
		if(DebugStats::Inst())
		{
			Color col(1.0f, 0.4f, 0.0f, 0.0f);
			m_debugStats.RegisterTimer(g_szTimerNames[i], &m_pImpl->profileTimers[i], col, 0.01f);
		}
	}
#endif
}

void Scene::Reset()
{

}


TransformNode* Scene::CreateTransformNode()
{
	ScTransformNode* pSceneNode = m_pImpl->transformNodes.Alloc();
	
	return (TransformNode*)pSceneNode;
}

void Scene::DeleteTransformNode(const TransformNode* pNode)
{
	ScTransformNode* pSCNode = (ScTransformNode*)pNode;
	ASSERT(pSCNode->GetId() == ScTransformNode::SC_TRANS_NODE_ID);
	m_pImpl->transformNodes.Free(pSCNode);
}

RenderGroup* Scene::CreateRenderGroup(const TransformNode* pTransform)
{
	RenderGroup* pSceneComponent = m_pImpl->sceneComponents.Alloc();
	pSceneComponent->Init(pTransform, this);

	// TODO: Transforms can't be shared between render groups, assert we don't
	// already have an instance of the transform
	if(pTransform)
	{
		m_pImpl->octree.InsertObject(pTransform, (void*)pSceneComponent, pSceneComponent->GetRenderMask() );
		m_uPVSCount++;
	}
	else
	{
		m_pImpl->staticComponents.push_back(pSceneComponent);
	}
	return pSceneComponent;
}


void Scene::RegisterOffscreenRenderNode(OffscreenRenderNode* pNode)
{
	m_pImpl->offscreenNodes.push_back(pNode);
}

void Scene::DeregisterOffscreenRenderNode(OffscreenRenderNode* pNode)
{
	m_pImpl->offscreenNodes.remove(pNode);
}

void Scene::UpdateMask(const TransformNode* pTransform, uint32 uMask)
{
	m_pImpl->octree.UpdateMask(pTransform, uMask);
}


void Scene::DeleteRenderGroup(RenderGroup* pRemove)
{
	RenderGroup* pComponent = NULL;
	for(FastPool<RenderGroup>::Iterator it = m_pImpl->sceneComponents.Begin(); !it.IsEnd(); ++it)
	{
		if(pRemove == (*it))
		{
			pComponent = *it;
			break;
		}
	}

	ASSERT(pComponent!=NULL);

	if(pRemove->HasTransform())
	{
		m_pImpl->octree.RemoveObject(pRemove->GetTransform());
		m_uPVSCount--;
	}
	else
	{
		m_pImpl->staticComponents.remove(pComponent);
	}
	pRemove->Cleanup();
	m_pImpl->sceneComponents.Free(pComponent);
}

ViewContext* Scene::CreateViewContext(GFXDevice* pDevice)
{
	ViewContext* pContext = m_pImpl->viewContexts.Alloc();
	pContext->SetScene(this);
	if (!pContext->IsDeviceDataValid())
	{
		pContext->InitDeviceData(pDevice);
	}
	m_pImpl->sceneContexts.push_back(pContext);
	return pContext;
}


void Scene::DeleteViewContext(ViewContext* pRemove)
{
	m_pImpl->sceneContexts.remove(pRemove);
	m_pImpl->viewContexts.Free(pRemove);
}

ViewContext* Scene::GetViewContext(uint32 uId)
{
	// FIXME: Give an ID to these contexts, or ideally a hash
	if (m_pImpl->viewContexts.Size() > uId)
	{
		return m_pImpl->viewContexts.GetByIndex(uId);
	}
	return nullptr;
}

uint32 Scene::GetViewContextCount() const
{
	return m_pImpl->viewContexts.Size();
}

SceneRenderPasses& Scene::GetRenderPasses(uint32 uViewContext)
{
	return GetViewContext(uViewContext)->GetRenderPasses();
}

OmniShadowContext* Scene::CreateOmniShadowContext(GFXDevice* pDevice)
{
	OmniShadowContext* pContext = m_pImpl->omniShadowContexts.Alloc();
	ASSERT(pContext != nullptr);
	if (pContext)
	{
		if (!pContext->IsDeviceDataValid())
		{
			pContext->InitDeviceData(pDevice);
		}
		pContext->SetScene(this);
		m_pImpl->sceneContexts.push_back(pContext);
	}

	return pContext;
}


void Scene::DeleteOmniShadowContext(OmniShadowContext* pRemove)
{
	m_pImpl->sceneContexts.remove(pRemove);
	m_pImpl->omniShadowContexts.Free(pRemove);
}

ShadowContext* Scene::CreateShadowContext(GFXDevice* pDevice)
{
	ShadowContext* pContext = m_pImpl->shadowContexts.Alloc();
	ASSERT(pContext != nullptr);
	if (pContext)
	{
		if (!pContext->IsDeviceDataValid())
		{
			pContext->InitDeviceData(pDevice);
		}
		pContext->SetScene(this);
		m_pImpl->sceneContexts.push_back(pContext);
	}

	return pContext;
}


void Scene::DeleteShadowContext(ShadowContext* pRemove)
{
	m_pImpl->sceneContexts.remove(pRemove);
	m_pImpl->shadowContexts.Free(pRemove);
}


void Scene::TransformUpdate(float fElapsed)
{
	m_pImpl->profileTimers[TIMER_PRE_CULL].ClearAndStart();
	for (FastPool<ScTransformNode>::Iterator it = m_pImpl->transformNodes.Begin(); !it.IsEnd(); ++it)
	{
		(*it)->PreCullUpdate();
	}

	m_pImpl->profileTimers[TIMER_PRE_CULL].Stop();
	
	m_pImpl->profileTimers[TIMER_PARTICLES].ClearAndStart();
	m_pImpl->particleSystem.Update(fElapsed);
	m_pImpl->profileTimers[TIMER_PARTICLES].Stop();

	m_pImpl->lightMgr.Update(fElapsed, m_uFrame);
}


void Scene::PerformVisibilityTesting(GFXDevice* pDevice)
{
	m_pImpl->profileTimers[TIMER_VISIBILITY].ClearAndStart();
	uint32 uContextCount = 0;
	m_pDevice = pDevice;

	m_pImpl->octree.UpdateTransforms();

	// Primary visibility list
	for(auto it : m_pImpl->sceneContexts)
	{
		if( it->IsActive() )
		{
			it->ClearLists();
			if(it->GetCamera())
			{
				m_activeView.BuildCameraFromModel(it->GetCamera()->GetModelMatrix());
			}

			uContextCount++;
		}
	}

	for (auto it : m_pImpl->sceneContexts)
	{
		if (it->IsActive())
		{
			m_pImpl->octree.GetVisibleList(it->GetSearchObject());
		}
	}

	m_pImpl->profileTimers[TIMER_VISIBILITY].Stop();

	m_pImpl->profileTimers[TIMER_NODE_UPDATE].ClearAndStart();


	for (auto it : m_pImpl->sceneContexts)
	{
		if (it->IsActive())
		{
			it->UpdateVisibleGroups();
		}
	}

	m_pImpl->profileTimers[TIMER_NODE_UPDATE].Stop();


	m_pImpl->profileTimers[TIMER_STATIC_UPDATE].ClearAndStart();
	for(RenderGroup* pGroup : m_pImpl->staticComponents )
	{
		bool bVisible = false;
		RenderGroup* pCmp = pGroup;
		for (auto pSceneCtxt : m_pImpl->sceneContexts)
		{
			if( pSceneCtxt->GetRenderMask() & pGroup->GetRenderMask() )
			{
				pSceneCtxt->AddToDrawList(pGroup);
				bVisible = true;
			}
		}
		

		if(bVisible)
		{
			if(pCmp->GetLastUpdateFrame() != m_uFrame)
			{
				pGroup->VisibilityFunc(pDevice, m_activeView);
			}
		}
	}
	m_pImpl->profileTimers[TIMER_STATIC_UPDATE].Stop();


	m_pDevice = NULL;
}

void Scene::PreUpdate()
{
	m_uFrame++;
	m_pImpl->debug3D.Clear();
}

void Scene::PreDraw(GFXContext* pContext)
{
	m_pImpl->lightMgr.GlobalShadowRender(pContext, this);
	for (auto itr : m_pImpl->offscreenNodes)
	{
		itr->Draw(pContext);
	}
}

void Scene::Update(GFXDevice* pDevice)
{
	PerformVisibilityTesting(pDevice);
	UpdateSceneContexts(pDevice);
	// Ideally don't want to be updating the buffers of any effects which arent visible
	m_pImpl->particleSystem.UpdateBuffers(pDevice);
	m_pImpl->debug3D.UpdateBuffers(pDevice);
	m_pImpl->lightMgr.GPUUpdate(pDevice);
}

void Scene::UpdateSceneContexts(GFXDevice* pDevice)
{
	m_pImpl->profileTimers[TIMER_SCENE].ClearAndStart();
	for(auto it : m_pImpl->sceneContexts)
	{
		if( it->IsActive() )
		{
			it->Update(pDevice);
		}
	}
	m_pImpl->profileTimers[TIMER_SCENE].Stop();
}


void Scene::UpdateObject(RenderGroup* pComponent)
{
	if(pComponent->GetLastUpdateFrame() != m_uFrame)
	{
		// TODO: Don't do this for shadow contexts
		pComponent->VisibilityFunc(m_pDevice, m_activeView);
#ifndef FINAL_BUILD
		if(m_bShowBounds)
		{
			const Sphere &colSphere = pComponent->GetTransform()->GetWorldSphere();
			Debug3D::GetRenderer()->AddSphere(colSphere.GetPos(), colSphere.GetRadius(), Color(0.0f, 0.0f, 1.0f, 0.2f));	
		}
#endif
	}
}


const Camera* Scene::GetSceneCamera(uint32 uIndex) const
{
	uint32 uIndexCmp = 0;
	for(FastPool<ViewContext>::Iterator it = m_pImpl->viewContexts.Begin(); !it.IsEnd(); ++it)
	{
		if(uIndexCmp == uIndex)
		{
			return (*it)->GetCamera();
		}
		uIndexCmp++;
	}

	return NULL;
}

void Scene::AddCamera(const Camera* pCamera)
{
	if (m_pImpl->cameras.size() == 0)
	{
		for (FastPool<ViewContext>::Iterator it = m_pImpl->viewContexts.Begin(); !it.IsEnd(); ++it)
		{
			if (!(*it)->GetCamera())
			{
				(*it)->SetCamera(pCamera);
				(*it)->SetRenderMask(pCamera->GetRenderMask());
			}
		}
	}
	m_pImpl->cameras.push_back(pCamera);
}

void Scene::RemoveCamera(const Camera* pCamera)
{
	m_pImpl->cameras.remove(pCamera);
}

LightMgr& Scene::GetLightMgr()
{
	return m_pImpl->lightMgr;
}

const LightMgr&	Scene::GetLightMgr() const
{
	return m_pImpl->lightMgr;
}

const AABB& Scene::GetWorldBounds() const
{
	return m_pImpl->octree.GetWorldBounds();
}

ParticleMgr& Scene::GetParticleMgr()
{
	return m_pImpl->particleSystem; 
}

void Scene::CreateEffect(ParticleEffectHndl& hndlOut, const Matrix4x4& mMat, const char* szName, const Vector3f &vVelocity)
{
	hndlOut = m_pImpl->particleSystem.CreateEffect(mMat, vVelocity, szName, true);
}

void Scene::CreateScriptedEffect(ParticleEffectHndl& hndlOut, const Matrix4x4& mMat, const char* szName, const Vector3f &vVelocity, float fScale)
{
	hndlOut = m_pImpl->particleSystem.CreateEffect(mMat, vVelocity, szName, false, fScale);
}

void Scene::CreateEffect(const Matrix4x4& mMat, const char* szName, const Vector3f &vVelocity)
{
	m_pImpl->particleSystem.CreateEffect(mMat, vVelocity, szName, true);
}

void Scene::CreateScriptedEffect(const Matrix4x4& mMat, const char* szName, const Vector3f &vVelocity, float fScale)
{
	m_pImpl->particleSystem.CreateEffect(mMat, vVelocity, szName, false, fScale);
}

#ifndef FINAL_BUILD
list<SceneContext*>& Scene::GetSceneContexts()
{
	return m_pImpl->sceneContexts;
}
#endif

}	// namespace usg

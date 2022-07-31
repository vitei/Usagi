/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Maths/MathUtil.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/SceneConstantSets.h"
#include "ParticleEffect.h"

namespace usg{

ParticleEffect::ParticleEffect()
{
	m_pScene = NULL;
	m_pRenderGroup = NULL;
	m_eState = STATE_INACTIVE;
	m_fMultiplier = 1.0f;
	m_fScale = 1.0f;
}

ParticleEffect::~ParticleEffect()
{
	//if(m_eState != STATE_INACTIVE)
	//{
	//	Kill(true);
	//}
}


void ParticleEffect::Cleanup()
{
	ASSERT(m_pScene!=NULL);
	if(m_pRenderGroup)
	{
		m_pScene->DeleteRenderGroup(m_pRenderGroup);
	}
	m_pRenderGroup = NULL;
	m_eState = STATE_INACTIVE;

	for(auto it = m_emitters.begin(); it != m_emitters.end(); ++it)
	{
		ParticleEmitter* pEmitter = (*it);
		// FIXME: Nothing technically wrong with this, but the design makes me sad...
		pEmitter->FreeFromPool();
	}

	m_emitters.clear();
}

void ParticleEffect::EnableEmission(bool bEnable)
{
	if(m_eState != STATE_RUNNING)
		return; 

	for(auto it = m_emitters.begin(); it != m_emitters.end(); ++it)
	{
		ParticleEmitter* pEmitter = (*it);
		pEmitter->EnableEmission(bEnable);
	}
}

void ParticleEffect::Kill(bool bForce)
{
	if(m_eState != STATE_RUNNING)
		return; 

	if(bForce)
	{
		Cleanup();
	}
	else
	{
		// TODO: Tell the particle systems to stop creating new particles
		m_eState = STATE_SHUTDOWN;
		for( auto pEmitter : m_emitters)
		{
			pEmitter->EnableEmission(false);
		}
	}
}

void ParticleEffect::Init(GFXDevice* pDevice, Scene* pScene, const Matrix4x4& mMat, const Vector3f& vVelocity, float fScale)
{
	m_pScene = pScene;
	Sphere dummySphere;
	dummySphere.SetPos( Vector3f(0.0f, 0.0f, 0.0f) );
	// FIXME: Calculate the bounds
	dummySphere.SetRadius(25.0f);
	m_vPrevPos = mMat.vPos();
	m_mTransform = mMat;
	// FIXME: Add the transform node back
	m_pRenderGroup = m_pScene->CreateRenderGroup(NULL);//m_pTransformNode);Scale
	m_pRenderGroup->SetSortPos(mMat.vPos().v3());
	m_fScale = fScale;
	
	SetSystemVelocity(vVelocity);

	// Now initialise the particles, possibly with that data
	for(auto it : m_emitters )
	{
		it->Init(pDevice, this);
	}
	
	if(!m_constantSet.IsValid())
	{
		m_constantSet.Init(pDevice, SceneConsts::g_instanceCBDecl);
	}
	m_eState = STATE_RUNNING;
}


void ParticleEffect::Cleanup(GFXDevice* pDevice)
{
	m_constantSet.Cleanup(pDevice);
}

Vector4f ParticleEffect::GetPosition(float fLerp) const
{
	return Lerp(m_vPrevPos, m_mTransform.vPos(), fLerp);
}


void ParticleEffect::WorldShifted(const Vector3f& vShift)
{
	for (ParticleEmitter* pEmitter : m_emitters)
	{
		pEmitter->WorldShifted(vShift);
	}
}

bool ParticleEffect::Update(float fElapsed)
{
	bool bAlive = false;

	for(ParticleEmitter* pEmitter : m_emitters)
	{
		bAlive |= (pEmitter->Update(fElapsed) && m_eState == STATE_RUNNING);
		bAlive |= pEmitter->ActiveParticles();
		pEmitter->UpdateParticleCPUData(fElapsed);
	}

	m_vPrevPos = m_mTransform.vPos();

	if(!bAlive)
	{
		Cleanup();
	}

	if(m_pRenderGroup)
	{
		//m_pRenderGroup->ForceTransformBuffer(m_pTransformNode->GetMatrix());
		SceneConsts::ModelTransform* matrixData = m_constantSet.Lock<SceneConsts::ModelTransform>();
		matrixData->mModelMat = m_mTransform;
		m_constantSet.Unlock();
		m_pRenderGroup->SetSortPos(m_mTransform.vPos().v3());
	}

	return bAlive;
}

void ParticleEffect::UpdateBuffers(GFXDevice* pDevice)
{
	if (m_constantSet.IsValid())
	{
		m_constantSet.UpdateData(pDevice);
	}

	for (ParticleEmitter* pEmitter : m_emitters)
	{
		pEmitter->UpdateBuffers(pDevice);
	}
}

void ParticleEffect::AddEmitter(GFXDevice* pDevice, ParticleEmitter* pEmitter)
{
	m_emitters.push_back(pEmitter);
	RenderNode* pNode = (RenderNode*)pEmitter;
	m_pRenderGroup->AddRenderNodes(pDevice, &pNode, 1);
	pEmitter->SetScale(m_fScale);
	pEmitter->Init(pDevice, this);
	// Make sure it has the relevant data
}

void ParticleEffect::RemoveEmitter(ParticleEmitter* pEmitter)
{
	RenderNode* pNode = (RenderNode*)pEmitter;
	if(m_pRenderGroup)
	{
		m_pRenderGroup->RemoveRenderNode(pNode);
	}
	m_emitters.remove(pEmitter);
}

bool ParticleEffect::IsAlive()
{
	return m_eState != STATE_INACTIVE;
}

void ParticleEffect::SetWorldMat(const Matrix4x4& mMat)
{
	m_mTransform = mMat;
}




}
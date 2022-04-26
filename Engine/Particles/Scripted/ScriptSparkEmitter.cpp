/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Maths/MathUtil.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Particles/Scripted/ScriptedParticle.h"
#include "Engine/Maths/MathUtil.h"
#include "ScriptedParticle.h"
#include "ScriptSparkEmitter.h"

namespace usg
{

	ScriptSparkEmitter::ScriptSparkEmitter()
	{

	}

	ScriptSparkEmitter::~ScriptSparkEmitter()
	{

	}


	void ScriptSparkEmitter::FillOutConstantBuffer()
	{

	}

	float ScriptSparkEmitter::InitParticleData(void* pData, void* pMetaData, float fLerp)
	{
		const Matrix4x4& mParentMat = GetWorldMatrix();
		//mParentMat.SetPos(Lerp(m_vPrevPos, mParentMat.vPos(), fLerp));
		const particles::EmitterEmission& res = GetEmissionDef();
		Particle::ScriptedTrailParticle& out = *(Particle::ScriptedTrailParticle*)pData;
		Particle::ScriptedTrailMetaData& meta = *(Particle::ScriptedTrailMetaData*)pMetaData;

		float fLife = GetEmissionLife();

		out.fLifeStart = GetEffectTime();
		out.fInvLife = 1.0f/fLife;

		// TODO: Add these settings to the definition file
		out.vWidth.Assign(0.5f, 0.5f);
		
		InitEmissionPosAndVelocity(out.vPos[0], meta.vVelocity, mParentMat);
		for(uint32 i=1; i<Particle::ScriptedTrailParticle::POS_COUNT; i++)
		{
			// Have all vertices match the initial position so we create degenerate triangles
			out.vPos[i] = out.vPos[0];
		}

		out.fColorIndex = GetColorIndex();


		return fLife;


	}

	void ScriptSparkEmitter::UpdateParticleCPUDataInt(void* pGPUData, void* pMetaDataVoid, float fElapsed)
	{
		const particles::EmitterEmission& def = GetEmissionDef();

		Particle::ScriptedTrailParticle* pOut = (Particle::ScriptedTrailParticle*)pGPUData;
		Particle::ScriptedTrailMetaData* pMetaData = (Particle::ScriptedTrailMetaData*)pMetaDataVoid;

		for(uint32 i=0; i<Particle::ScriptedTrailParticle::POS_COUNT-1; i++)
		{
			// Shift the positions back
			pOut->vPos[i+1] = pOut->vPos[i];
		}

		pOut->vPos[0] += pMetaData->vVelocity * fElapsed;
		pMetaData->vVelocity += (def.vGravityDir*def.fGravityStrength) * fElapsed;
		pMetaData->vVelocity -= (pMetaData->vVelocity * fElapsed * (def.fDrag*5.f));

	}

}



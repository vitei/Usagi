/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Maths/MathUtil.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/SceneConstantSets.h"
#include "Engine/Particles/ParticleEffect.h"
#include "ParticleEmitter.h"

namespace usg{

ParticleEmitter::ParticleEmitter()
{
	m_bEmissionAllowed = true;

	SetLayer(LAYER_TRANSLUCENT);
}

ParticleEmitter::~ParticleEmitter()
{
	
}

void ParticleEmitter::Alloc(GFXDevice* pDevice)
{
	
}

void ParticleEmitter::Init(usg::GFXDevice* pDevice, ParticleEffect* pParent)
{
	m_pParent = pParent;
	m_bEmissionAllowed = true;
}

Vector4f ParticleEmitter::GetPosition() const
{
	return m_pParent->GetMatrix().vPos();
}

Vector4f ParticleEmitter::GetInterpolatedPos(float fInerpolation) const
{
	return m_pParent->GetPosition(fInerpolation);
}

bool ParticleEmitter::Update(float fElapsed)
{

	return true;
}



}



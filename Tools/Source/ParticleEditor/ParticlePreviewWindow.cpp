#include "Engine/Common/Common.h"
#include "Engine/GUI/IMGuiRenderer.h"
#include "Engine/Maths/AABB.h"
#include "Engine/Scene/ViewContext.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/Graphics/Lights/DirLight.h"
#include "Engine/Maths/MathUtil.h"
#include "ParticlePreviewWindow.h"
 
ParticlePreviewWindow::ParticlePreviewWindow()
{

}

ParticlePreviewWindow::~ParticlePreviewWindow()
{

}

void ParticlePreviewWindow::Init(usg::GFXDevice* pDevice, usg::IMGuiRenderer* pRenderer, const char* szName, const usg::Vector2f& vPos)
{
	Inherited::Init(pDevice, pRenderer, szName, vPos);
	usg::Matrix4x4 mEffectMat;
	mEffectMat.LoadIdentity();
	m_effect.Init(pDevice, &GetScene(), mEffectMat);
}

void ParticlePreviewWindow::CleanUp(usg::GFXDevice* pDevice)
{
	Inherited::CleanUp(pDevice);
	m_effect.CleanUp(pDevice);
}

bool ParticlePreviewWindow::Update(usg::GFXDevice* pDevice, float fElapsed)
{
	Inherited::Update(pDevice, fElapsed);

	if (m_effect.IsAlive())
	{
		if (!GetPaused())
		{
			m_effect.Update(fElapsed);
		}
	}

	if (GetRestart())
	{
		usg::Matrix4x4 mEffectMat;
		mEffectMat.LoadIdentity();

		//m_emitter.Init(&m_effect);
		m_effect.Kill(true);
		m_effect.Init(pDevice, &GetScene(), mEffectMat);
	}

	m_effect.UpdateBuffers(pDevice);

	return true;
}

void ParticlePreviewWindow::Draw(usg::GFXContext* pImmContext)
{
	Inherited::Draw(pImmContext);
}
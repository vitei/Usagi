#include "Engine/Common/Common.h"
#include "Engine/GUI/IMGuiRenderer.h"
#include "Engine/Maths/AABB.h"
#include "Engine/Scene/ViewContext.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/Graphics/Lights/DirLight.h"
#include "Engine/Maths/MathUtil.h"
#include "ParticlePreviewWindow.h"
 
ParticlePreviewWindow::ParticlePreviewWindow()
	: m_bReload(false)
{

}

ParticlePreviewWindow::~ParticlePreviewWindow()
{

}

void ParticlePreviewWindow::Init(usg::GFXDevice* pDevice, usg::IMGuiRenderer* pRenderer, const char* szName, const usg::Vector2f& vPos, uint32 uFlags)
{
	Inherited::Init(pDevice, pRenderer, szName, vPos, uFlags);
	usg::Matrix4x4 mEffectMat;
	mEffectMat.LoadIdentity();
	m_effect.Init(pDevice, &GetScene(), mEffectMat);
}

void ParticlePreviewWindow::Cleanup(usg::GFXDevice* pDevice)
{
	Inherited::Cleanup(pDevice);
	m_effect.Cleanup(pDevice);
}

bool ParticlePreviewWindow::Update(usg::GFXDevice* pDevice, float fElapsed)
{
	Inherited::Update(pDevice, fElapsed);

	m_bReload |= GetRestart();

	if (m_effect.IsAlive())
	{
		if (!GetPaused())
		{
			m_effect.Update(fElapsed * GetPlaySpeed());
		}
	}

	if (m_bReload)
	{

	}

	m_effect.UpdateBuffers(pDevice);

	m_bReload = false;

	return true;
}

void ParticlePreviewWindow::Draw(usg::GFXContext* pImmContext)
{
	Inherited::Draw(pImmContext);
}
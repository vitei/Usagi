#pragma once
#include "Engine/Particles/ParticleEffect.h"
#include "PreviewWindow.h"

class ParticlePreviewWindow : public PreviewWindow
{
	using Inherited = PreviewWindow;
public:
	ParticlePreviewWindow();
	virtual ~ParticlePreviewWindow();

	virtual void Init(usg::GFXDevice* pDevice, usg::IMGuiRenderer* pRenderer, const char* szName, const usg::Vector2f& vPos) override;
	virtual void CleanUp(usg::GFXDevice* pDevice) override;
	virtual bool Update(usg::GFXDevice* pDevice, float fElapsed) override;
	virtual void Draw(usg::GFXContext* pImmContext) override;

	usg::ParticleEffect& GetEffect() { return m_effect; }

private:
	usg::ParticleEffect		m_effect;
};

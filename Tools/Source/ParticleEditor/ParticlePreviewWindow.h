#pragma once
#include "Engine/Particles/ParticleEffect.h"
#include "PreviewWindow.h"

class ParticlePreviewWindow : public PreviewWindow
{
	using Inherited = PreviewWindow;
public:
	ParticlePreviewWindow();
	virtual ~ParticlePreviewWindow();

	virtual void Init(usg::GFXDevice* pDevice, usg::IMGuiRenderer* pRenderer, const char* szName, const usg::Vector2f& vPos, uint32 uFlags = SHOW_PLAY_CONTROLS | SHOW_PREVIEW_MODEL) override;
	virtual void Cleanup(usg::GFXDevice* pDevice) override;
	virtual bool Update(usg::GFXDevice* pDevice, float fElapsed) override;

	void SetReload() { m_bReload = true; }

	usg::ParticleEffect& GetEffect() { return m_effect; }

private:
	usg::ParticleEffect		m_effect;
	bool					m_bReload;
};

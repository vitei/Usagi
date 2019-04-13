#ifndef _USG_PARTICLE_EDITOR_RIBBON_INSTANCE_H_
#define _USG_PARTICLE_EDITOR_RIBBON_INSTANCE_H_
#include "Engine/Common/Common.h"
#include "Engine/Particles/RibbonTrail.h"
#include "Engine/Particles/Scripted/EffectGroup.pb.h"
#include "Engine/GUI/GuiItems.h"
#include "Engine/GUI/GuiWindow.h"
#include "Engine/GUI/IMGuiRenderer.h"
#include "FileList.h"

class ColorSelection;

class RibbonInstance
{
public:
	RibbonInstance() {}
	~RibbonInstance() {}

	void Init(usg::GFXDevice* pDevice, usg::Scene& scene, EffectGroup* pGroup, uint32 uIndex, ColorSelection* pSelection);
	void CleanUp(usg::GFXDevice* pDevice);
	bool Update(usg::GFXDevice* pDevice, float fElapsed);
	void AddToScene(usg::GFXDevice* pDevice, usg::particles::RibbonData* pInstance = NULL);
	void RemoveFromScene();
	bool GetActive() { return m_bActive; }
	const usg::particles::RibbonData& GetData() { return m_emitterData; }
	usg::RibbonTrail& GetEmitter() { return m_trail; }
	//usg::RibbonTrail& GetEmitter() { return m_emitter; }
private:
	void Add(bool bAdd) { m_emitterWindow.SetVisible(bAdd); m_bActive = bAdd; }
	void UpdateInstanceMatrix(usg::GFXDevice* pDevice);

	enum
	{
		COLOR_START = 0,
		COLOR_END,
		COLOR_COUNT
	};

	enum
	{
		MAX_FILE_COUNT = 64
	};
	usg::GUIComboBox			m_textureSelect;
	usg::GUIWindow				m_emitterWindow;
	usg::GUIFloat				m_position;
	usg::GUIColorSelect			m_colors[COLOR_COUNT];
	usg::GUIFloat				m_lifeTime;
	usg::GUIFloat				m_scale;
	
	usg::GUIButton				m_removeTrailButton;
	ColorSelection*				m_pColorSelection;
	// TODO: Add the ribbon trail visualiser

	usg::RibbonTrail			m_trail;
	usg::particles::RibbonData m_emitterData;
	bool						m_bActive;
	class EffectGroup*			m_pParent;
};


#endif

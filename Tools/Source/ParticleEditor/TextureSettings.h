#ifndef _USG_PARTICLE_EDITOR_TEXTURE_SETTINGS_H_
#define _USG_PARTICLE_EDITOR_TEXTURE_SETTINGS_H_
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Graphics/Device/StateEnums.pb.h"
#include "Engine/GUI/GuiWindow.h"
#include "Engine/GUI/GuiItems.h"
#include "EmitterModifier.h"
#include "FloatAnim.h"
#include "FileList.h"

namespace usg
{
	class IMGuiRenderer;
}

class TextureSettings : public EmitterModifier
{
public:
	TextureSettings();
	~TextureSettings();

	virtual void Init(usg::GFXDevice* pDevice, usg::IMGuiRenderer* pRenderer);
	void CleanUp(usg::GFXDevice* pDevice);
	virtual void SetWidgetsFromDefinition(usg::particles::EmitterEmission& structData);
	virtual bool Update(usg::GFXDevice* pDevice, usg::particles::EmitterEmission& structData, usg::ScriptEmitter* pEffect);

private:
	void UpdateAnimFrames(usg::GFXDevice* pDevice);
	void SetAnimPreview(usg::GFXDevice* pDevice, usg::particles::EmitterEmission& structData);
	// Returns the aspect
	float GetUVCoords(uint32 uFrame, usg::Vector2f& vMin, usg::Vector2f &vMax);
	enum 
	{
		FRAME_BOXES = 8,
		MAX_ANIM_FRAMES = 16
	};

	usg::TextureHndl		m_pTexture;
	usg::SamplerHndl		m_sampler;

	FileList<512>			m_fileList;
	usg::GUIWindow			m_animFrameWindow;
	usg::GUITexture			m_texture;
	usg::GUITexture			m_animTextures[MAX_ANIM_FRAMES];
	usg::GUIButton			m_previewButton;
	usg::GUITextInput		m_textInput;
	usg::GUIIntInput		m_repeat;
	usg::GUIComboBox		m_comboBox;
	usg::GUIComboBox		m_fileListBox;
	usg::U8String			m_textureName;
	usg::GUISlider			m_animTimeScale;
	usg::GUICheckBox		m_checkBox;
	usg::GUIText			m_animTitle;
	usg::GUIIntInput		m_frameCount;
	usg::GUIIntInput		m_frameBoxes[FRAME_BOXES];
	bool					m_bForceReload;
	float					m_fAnimTime;
};


#endif
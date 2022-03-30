#ifndef _USG_PARTICLE_EDITOR_TEXTURE_SETTINGS_H_
#define _USG_PARTICLE_EDITOR_TEXTURE_SETTINGS_H_

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

class TextureSettings : public EmitterModifier, public usg::GUICallbacks
{
public:
	TextureSettings();
	~TextureSettings();

	virtual void Init(usg::GFXDevice* pDevice, usg::IMGuiRenderer* pRenderer);
	void Cleanup(usg::GFXDevice* pDevice);
	virtual void SetWidgetsFromDefinition(usg::particles::EmitterEmission& structData);
	virtual bool Update(usg::GFXDevice* pDevice, usg::particles::EmitterEmission& structData, usg::ScriptEmitter* pEffect, float fElapsed);

private:
	virtual void MultiLoadCallback(const char* szName, const usg::vector<usg::FilePathResult>& results);
	void ConvertTGA(usg::GFXDevice* pDevice, const usg::string& name, const usg::string& outName);

	void UpdateAnimFrames(usg::GFXDevice* pDevice);
	void SetAnimPreview(usg::GFXDevice* pDevice, usg::particles::EmitterEmission& structData, float fElapsed);
	// Returns the aspect
	float GetUVCoords(uint32 uFrame, usg::Vector2f& vMin, usg::Vector2f &vMax);
	enum 
	{
		FRAME_BOXES = 8,
		MAX_ANIM_FRAMES = 128
	};

	usg::TextureHndl		m_pTexture;
	usg::SamplerHndl		m_sampler;

	FileList<512>			m_fileList;
	usg::GUIWindow			m_animFrameWindow;
	usg::GUITexture			m_texture;
	usg::GUITexture			m_animTextures[MAX_ANIM_FRAMES];
	usg::GUIButton			m_previewButton;
	usg::GUILoadButton		m_createFlipBook;
	usg::GUITextInput		m_textInput;
	usg::GUIIntInput		m_repeat;
	usg::GUIComboBox		m_comboBox;
	usg::GUIComboBox		m_fileListBox;
	usg::string				m_textureName;
	usg::GUISlider			m_animTimeScale;
	usg::GUICheckBox		m_checkBox;
	usg::GUIText			m_animTitle;
	usg::GUIIntInput		m_frameCount;
	usg::GUIIntInput		m_frameBoxes[FRAME_BOXES];
	bool					m_bForceReload;
	float					m_fAnimTime;
};


#endif
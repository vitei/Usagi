#ifndef _USG_PARTICLE_EDITOR_PREVIEW_MODEL_H_
#define _USG_PARTICLE_EDITOR_PREVIEW_MODEL_H_

#include "Engine/GUI/GuiItems.h"
#include "Engine/GUI/GuiWindow.h"
#include "Engine/GUI/IMGuiRenderer.h"
#include "FileList.h"

namespace usg
{
	class Model;
}


class PreviewModel
{
public:
	enum
	{
		MAX_FILE_COUNT = 50
	};

	PreviewModel() { m_pModel = NULL;  }
	~PreviewModel();

	void Init(usg::GFXDevice* pDevice, usg::Scene* pScene, usg::IMGuiRenderer* pRenderer);
	void Update(usg::GFXDevice* pDevice, float fElapsed);
	void CleanUp(usg::GFXDevice* pDevice);
private:

	usg::Model*			  m_pModel;
	

	FileList<MAX_FILE_COUNT>	m_modelFileList;
	usg::GUIComboBox			m_loadFilePaths;
	usg::GUIButton				m_loadButton;
	usg::GUICheckBox			m_visible;

	
	usg::GUIWindow				m_window;
	usg::GUIFloat				m_position;
	usg::Scene*					m_pScene;
};


#endif

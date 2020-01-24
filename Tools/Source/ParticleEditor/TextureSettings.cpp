#include "Engine/Common/Common.h"
#include "Engine/GUI/IMGuiRenderer.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/Maths/MathUtil.h"
#include "Engine/Graphics/Textures/TGAFile.h"
#include "TextureSettings.h"
#include "gli/gli.hpp"


static const char* g_szAnimationTiming[] =
{
	"No animation",
	"Flip book scaled",
	"Flip book looped",
	"Random image",
	"Scaled to time",
	NULL
};


TextureSettings::TextureSettings()
{
		m_bForceReload = false;
}


TextureSettings::~TextureSettings()
{

}

void TextureSettings::Init(usg::GFXDevice* pDevice, usg::IMGuiRenderer* pRenderer)
{
	usg::Vector2f vWindowPos(800.0f, 0.0f);
	usg::Vector2f vWindowSize(320.f, 240.f);
	usg::Vector2f vTextureSize(64.f, 64.f);
	int defaultRepeat[] = { 1, 1 };
	m_window.Init("Texture", vWindowPos, vWindowSize, usg::GUIWindow::WINDOW_TYPE_COLLAPSABLE);
	m_pTexture = usg::ResourceMgr::Inst()->GetTextureAbsolutePath(pDevice, "Textures/missing_texture", true, usg::GPU_LOCATION_STANDARD);
	vTextureSize.x *= ((float)m_pTexture->GetWidth()/(float)m_pTexture->GetHeight());
	m_texture.Init(pDevice, "Particle tex", vTextureSize, m_pTexture);
	m_fileList.Init("Textures/particles/", ".dds");
	m_fileListBox.Init("Texture Select", m_fileList.GetFileNamesRaw());
	m_repeat.Init("Repeat X, Y", defaultRepeat, 2, 1, 32);
	m_repeat.SetToolTip("Number of sub images in the image along X and Y");
	m_comboBox.Init("Anim Mode", g_szAnimationTiming, 0);
	m_comboBox.SetToolTip("How the sub images are to be animated");
	m_checkBox.Init("Random offset", false);
	m_checkBox.SetToolTip("Apply a random offset to the image to be displayed");

	m_createFlipBook.Init("Create Flipbook");
	m_createFlipBook.SetAllowMultiple(true);
	m_createFlipBook.SetExtension("tga");
	m_createFlipBook.AddFilter("Targa Image File", "*.tga");
	m_createFlipBook.SetCallbacks(this);

	usg::SamplerDecl samplerDecl(usg::SF_LINEAR, usg::SC_WRAP);
	m_sampler = pDevice->GetSampler(samplerDecl);

	m_previewButton.InitAsTexture(pDevice, "Preview", m_pTexture);

	usg::Vector2f vAnimWindowSize(330.0f, 140.0f);
	m_animFrameWindow.Init("Frames", vWindowPos, vAnimWindowSize, usg::GUIWindow::WINDOW_TYPE_CHILD );
	m_animFrameWindow.SetShowBorders(true);

	char frameName[256];
	for(uint32 i=0; i<MAX_ANIM_FRAMES; i++)
	{
		usg::Vector2f vFrameSize(85.f, 85.f);
		str::ParseVariableArgsC(frameName, 256, "Frame %d", i);
		m_animTextures[i].Init(pDevice, frameName, vFrameSize, m_pTexture);
		m_animTextures[i].SetSameLine((i%4)!=0);
		m_animTextures[i].SetToolTip(frameName);
		m_animTextures[i].SetVisible(false);
		m_animFrameWindow.AddItem(&m_animTextures[i]);
	}

	m_animTitle.Init("Anim timings");
	int frameCount = 1;
	m_frameCount.Init("Frame count", &frameCount, 1, 1, MAX_ANIM_FRAMES);

	m_animTimeScale.Init("Anim time scale", 0.1f, 2.0f, 1.0f);

	m_window.AddItem(&m_createFlipBook);
	m_window.AddItem(&m_texture);
	m_window.AddItem(&m_fileListBox);
	m_window.AddItem(&m_repeat);
	m_window.AddItem(&m_animFrameWindow);
	m_window.AddItem(&m_comboBox);
	m_window.AddItem(&m_checkBox);
	m_window.AddItem(&m_animTitle);
	m_window.AddItem(&m_frameCount);

	UpdateAnimFrames(pDevice);
	m_fAnimTime = 0.0f;

	int defaultFrames[] = {0, 0, 0, 0};
	char names[256];
	for(uint32 i=0; i<FRAME_BOXES; i++)
	{
		str::ParseVariableArgsC(names, 256, "%d - %d", (i*4+1), (i*4+4));
		m_frameBoxes[i].Init(names, defaultFrames, 4 );
		m_window.AddItem(&m_frameBoxes[i]);
	}
	m_window.AddItem(&m_animTimeScale);
	m_window.AddItem(&m_previewButton);

	//pRenderer->AddWindow(&m_window);
}

void TextureSettings::MultiLoadCallback(const char* szName, const usg::vector<usg::FilePathResult>& results)
{
	if (str::Compare(szName, "Create Flipbook"))
	{
		if (results.size() > 0)
		{
			usg::FileOpenPath::Filter filter;
			filter.szDisplayName = "Targa";
			filter.szExtPattern = "*.tga";
			usg::FileOpenPath fileName;
			fileName.szWindowTitle = "Flipbook";
			fileName.szDefaultExt = "tga";
			fileName.pFilters = &filter;
			fileName.uFilterCount = 1;
			fileName.szOpenDir = nullptr;

			usg::vector<usg::TGAFile> files;
			files.resize(results.size());

			usg::FilePathResult result;
			bool bSuccess = true;
			if (usg::File::UserFileSavePath(fileName, result))
			{
				for (memsize i = 0; i < results.size(); i++)
				{
					if (!files[i].Load(results[i].szPath, usg::FILE_TYPE_RESOURCE))
					{
						FATAL_RELEASE(false, "Failed to load TGA file %s", results[i].szPath);
						bSuccess = false;
						break;
					}

					if (i != 0)
					{
						if (files[i].GetHeader().uWidth != files[0].GetHeader().uWidth
							|| files[i].GetHeader().uHeight != files[0].GetHeader().uHeight
							|| files[i].GetHeader().uDataTypeCode != files[0].GetHeader().uDataTypeCode)
						{
							FATAL_RELEASE(false, "Icompataible TGA files %s and %s", results[i].szRelativePath, results[0].szRelativePath);
							bSuccess = false;
							break;
						}
					}
				}
			}

			if (bSuccess)
			{
				// Make the file
				usg::TGAFile outFile;
				// First calculate the size
				sint iWidth = sqrt( results.size() + 1 );
				sint iHeight = ((results.size() + iWidth - 1) / iWidth);

				sint texWidth = files[0].GetHeader().uWidth * iWidth;
				sint texHeight = files[0].GetHeader().uHeight * iHeight;

				usg::TGAFile::Header hdr = files[0].GetHeader();
				hdr.uWidth = texWidth;
				hdr.uHeight = texHeight;
				outFile.PrepareImage(files[0].GetHeader());
				
				outFile.Save("d:\\aaa\\test.tga", usg::FILE_TYPE_RESOURCE);
			}
		}
	}
}

void TextureSettings::CleanUp(usg::GFXDevice* pDevice)
{
	m_previewButton.CleanUp(pDevice);
	for (uint32 i = 0; i < MAX_ANIM_FRAMES; i++)
	{
		m_animTextures[i].CleanUp(pDevice);
	}
	m_texture.CleanUp(pDevice);
}

void TextureSettings::SetWidgetsFromDefinition(usg::particles::EmitterEmission& structData)
{
	usg::particles::TextureData& textureVars = structData.textureData[0];
	m_bForceReload = true;
	m_fileList.Update();
	for(uint32 i=0; i<m_fileList.GetFileCount(); i++)
	{
		usg::U8String name = "particles/";
		name += m_fileList.GetFileName(i);
		name.TruncateExtension();
		if(name == textureVars.name)
		{
			m_fileListBox.SetSelected(i);
			break;
		}
	}
	m_textInput.SetInput(textureVars.name);
	int repeat[] = { (int)textureVars.uPatternRepeatHor, (int)textureVars.uPatternRepeatVer };
	m_repeat.SetValues(repeat);
	m_comboBox.SetSelected((uint32)textureVars.textureAnim.eTexMode);
	m_checkBox.SetValue(textureVars.textureAnim.bRandomOffset);
	int frameCount = (int)textureVars.textureAnim.animIndex_count;
	m_frameCount.SetValues(&frameCount);
	m_animTimeScale.SetValue(textureVars.textureAnim.fAnimTimeScale);

	int indices[textureVars.textureAnim.animIndex_max_count];
	memset(indices, 0, sizeof(int)*textureVars.textureAnim.animIndex_max_count);
	for(uint32 i=0; i<textureVars.textureAnim.animIndex_count; i++)
	{
		indices[i] = textureVars.textureAnim.animIndex[i];
	}

	uint32 item =0;
	for(uint32 i=0; i<FRAME_BOXES; i++)
	{
		m_frameBoxes[i].SetRange(0, ((int)(textureVars.uPatternRepeatHor*textureVars.uPatternRepeatVer))-1);
	}

	for(uint32 i=0; i<FRAME_BOXES; i++)
	{
		m_frameBoxes[i].SetValues(&indices[i*4]);
	}
}



bool TextureSettings::Update(usg::GFXDevice* pDevice, usg::particles::EmitterEmission& structData, usg::ScriptEmitter* pEffect)
{
	bool bAltered = false;
	bool bAnimAltered = false;
	usg::particles::TextureData& textureVars = structData.textureData[0];

	static uint32 uFrame = 0;
	if(uFrame > 48)
	{
		m_fileList.Update();
		m_fileListBox.UpdateOptions(m_fileList.GetFileNamesRaw());
		uFrame = 0;
	}
	uFrame++;


	if(m_textureName != m_fileList.GetFileName(m_fileListBox.GetSelected()) || m_bForceReload)
	{
		m_textureName = m_fileList.GetFileName(m_fileListBox.GetSelected());
		usg::Vector2f vTextureSize(64.f, 64.f);
		usg::U8String texName = "particles/";
		texName += m_fileList.GetFileName(m_fileListBox.GetSelected());
		// The resource manager adds the extension
		texName.TruncateExtension();
		m_pTexture = usg::ResourceMgr::Inst()->GetTexture(pDevice, texName.CStr(), usg::GPU_LOCATION_STANDARD);
		vTextureSize.x *= ((float)m_pTexture->GetWidth()/(float)m_pTexture->GetHeight());
		m_texture.SetTexture(pDevice, m_pTexture);
		m_texture.SetSize(vTextureSize);
		pEffect->GetMaterial().SetTexture(0, m_pTexture, m_sampler);
		str::Copy(textureVars.name, texName.CStr(), sizeof(textureVars.name));
		bAnimAltered = true;
		m_bForceReload = false;
	}

	bAnimAltered |= Compare(textureVars.textureAnim.fAnimTimeScale, m_animTimeScale.GetValue());
	bAnimAltered |= Compare(textureVars.uPatternRepeatHor, m_repeat.GetValue()[0]);
	bAnimAltered |= Compare(textureVars.uPatternRepeatVer, m_repeat.GetValue()[1]);
	bAnimAltered |= Compare(textureVars.textureAnim.animIndex_count, m_frameCount.GetValue(0));
	bAnimAltered |= Compare(textureVars.textureAnim.eTexMode, m_comboBox.GetSelected());
	bAnimAltered |= Compare(textureVars.textureAnim.bRandomOffset, m_checkBox.GetValue());

	bool bShowAnimDetails = false;
	switch(textureVars.textureAnim.eTexMode)
	{
	case usg::particles::TEX_MODE_FLIPBOOK_LOOP:
	case usg::particles::TEX_MODE_FLIPBOOK_ONCE:
		bShowAnimDetails = true;
		break;
	default:
		break;
	}

	m_animTitle.SetVisible(bShowAnimDetails);
	m_frameCount.SetVisible(bShowAnimDetails);
	m_animTimeScale.SetVisible(textureVars.textureAnim.eTexMode==usg::particles::TEX_MODE_FLIPBOOK_LOOP);

	uint32 item =0;
	for(uint32 i=0; i<FRAME_BOXES; i++)
	{
		m_frameBoxes[i].SetVisible(((textureVars.textureAnim.animIndex_count+3)/4 > i) && bShowAnimDetails);
		m_frameBoxes[i].SetRange(0, ((int)(textureVars.uPatternRepeatHor*textureVars.uPatternRepeatVer))-1);
		for(uint32 subItem = 0; subItem < 4; subItem++)
		{
			bAnimAltered |= Compare(textureVars.textureAnim.animIndex[item], m_frameBoxes[i].GetValue(subItem));			
			item++;
		}
		
	}

	if(bAnimAltered)
	{
		UpdateAnimFrames(pDevice);
	}

	SetAnimPreview(pDevice, structData);

	if(bAltered)
	{
		// TODO: Update the emission parameters
	}

	// Nothing we do here makes it necessary to re-create the effect
	return bAltered || bAnimAltered;
}

float TextureSettings::GetUVCoords(uint32 uAnimFrame, usg::Vector2f& vMin, usg::Vector2f &vMax)
{
	uint32 uX = m_repeat.GetValue()[0];
	uint32 uY = m_repeat.GetValue()[1];
	float fXRange = 1.0f / (float)uX;
	float fYRange = 1.0f / (float)uY;
	uint32 uXCell = uAnimFrame % uX;
	uint32 uYCell = uAnimFrame / uX;

	vMin.Assign(fXRange * uXCell, fYRange * uYCell);
	vMax.Assign((fXRange * (uXCell+1)), (fYRange * (uYCell+1)));

	return (vMax.x-vMin.x)/(vMax.y - vMin.y);
}

void TextureSettings::SetAnimPreview(usg::GFXDevice* pDevice, usg::particles::EmitterEmission& structData)
{
	if(m_previewButton.GetValue())
	{
		m_fAnimTime = 0.0f;
	}

	float fElapsed = 1.0f/60.f;

	m_previewButton.SetVisible(true);

	uint32 patternIdx = 0;
	switch(structData.textureData[0].textureAnim.eTexMode)
	{
	case usg::particles::TEX_MODE_FLIPBOOK_ONCE:
		patternIdx = (uint32)((m_fAnimTime / structData.life.frames[0].fValue)*structData.textureData[0].textureAnim.animIndex_count);
		patternIdx = usg::Math::Clamp(patternIdx, (uint32)0, (uint32)structData.textureData[0].textureAnim.animIndex_count-1);
		break;
	case usg::particles::TEX_MODE_RANDOM_IMAGE:
		patternIdx = usg::Math::Rand()%structData.textureData[0].textureAnim.animIndex_count;
		break;
	case usg::particles::TEX_MODE_FIT_TO_TIME:
		patternIdx = (uint32)((m_fAnimTime / structData.life.frames[0].fValue)*( structData.textureData[0].uPatternRepeatHor * structData.textureData[0].uPatternRepeatVer) );
		patternIdx = usg::Math::Clamp(patternIdx, (uint32)0, (uint32)(structData.textureData[0].uPatternRepeatHor * structData.textureData[0].uPatternRepeatVer) - 1);
		break;
	case usg::particles::TEX_MODE_FLIPBOOK_LOOP:
		patternIdx = ((uint32)((30.f)*m_fAnimTime))%structData.textureData[0].textureAnim.animIndex_count;
		fElapsed *= m_animTimeScale.GetValue(0);
		break;
	case usg::particles::TEX_MODE_NONE:
	default:
		m_previewButton.SetVisible(false);
		return;
	}

	if (structData.textureData[0].textureAnim.eTexMode != usg::particles::TEX_MODE_FIT_TO_TIME)
	{
		patternIdx = structData.textureData[0].textureAnim.animIndex[patternIdx];
	}

	usg::Vector2f vMin, vMax;
	float fAspect = GetUVCoords(patternIdx, vMin, vMax);
	m_previewButton.SetTexture(pDevice, m_pTexture);
	m_previewButton.SetUVs(vMin, vMax);

	if (m_pTexture->GetWidth() > 350.0f)
	{
		float fScale = 350.0f / (float)(m_pTexture->GetWidth()/ m_repeat.GetValue()[0]);
		m_previewButton.SetScale(usg::Vector2f(fScale, fScale));
	}
	else
	{
		m_previewButton.SetScale(usg::Vector2f(1.0f, 1.0f));
	}

	m_fAnimTime += fElapsed;
}

void TextureSettings::UpdateAnimFrames(usg::GFXDevice* pDevice)
{
	uint32 uX = m_repeat.GetValue()[0];
	uint32 uY = m_repeat.GetValue()[1];
	uint32 uAnims = usg::Math::Clamp(uX*uY, (uint32)0, (uint32)MAX_ANIM_FRAMES);
	float fAspect = (float)m_pTexture->GetWidth()/(float)m_pTexture->GetHeight();

	usg::Vector2f vUVMin;
	usg::Vector2f vUVMax;
	uint32 uAnimId = 0;
	for(; uAnimId < uAnims; uAnimId++)
	{
		float fUVAspect = GetUVCoords(uAnimId, vUVMin, vUVMax);
		float fAdjAspect = fAspect * fUVAspect;
		m_animTextures[uAnimId].SetUVs(vUVMin, vUVMax);
		m_animTextures[uAnimId].SetTexture(pDevice, m_pTexture);
		m_animTextures[uAnimId].SetVisible(true);
		usg::Vector2f vFrameSize;
		if(fAdjAspect > 1.0f)
			vFrameSize.Assign(65.f, 65.f/fAdjAspect);
		else
			vFrameSize.Assign(65.f*fAdjAspect, 65.f);

		m_animTextures[uAnimId].SetSize(vFrameSize);
	}

	for(uint32 i=uAnimId; i < MAX_ANIM_FRAMES; i++)
	{
		m_animTextures[i].SetVisible(false);
	}

}
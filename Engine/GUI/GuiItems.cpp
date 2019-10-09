/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/ThirdParty/imgui/imgui.h"
#include "Engine/Graphics/Device/GFXContext.h"
#include "GuiItems.h"
#include "Engine/Core/String/String_Util.h"
#include <commdlg.h>

namespace usg
{

	const float GUIItem::ms_fToolTipDelay = 1.0f;

	GUIItem::GUIItem()
		: m_pCallbacks(nullptr)
		, m_bSameLine(false)
		, m_bHovered(false)
		, m_bVisible(true)
		, m_szName{}
	{
	}

	GUIItem::~GUIItem()
	{

	}

	void GUIItem::InitBase(const char* szName)
	{
		str::Copy(m_szName, szName, sizeof(m_szName));
	}

	void GUIItem::CommonDraw()
	{
		if (m_toolTip.size() > 0 && m_bHovered && ((ImGui::GetTime() - m_fNewHoverTime) > ms_fToolTipDelay) )
		{
			ImGui::SetTooltip(m_toolTip.c_str());
		}
	}

	void GUIItem::UpdateBase()
	{
		if(m_bSameLine)
		{
			ImGui::SameLine();
		}
	}

	GUIButton::GUIButton()
	{
		m_bHasTexture = false;
		m_bTexDescValid = false;
		m_bValue = false;
		m_vUVMin.Assign(0.0f, 0.0f);
		m_vUVMax.Assign(1.0f, 1.0f);
	}

	GUIButton::~GUIButton()
	{
		
	}

	void GUIButton::SetUVs(Vector2f vUVMin, Vector2f vUVMax)
	{
		m_vUVMin = vUVMin;
		m_vUVMax = vUVMax;
	}

	void GUIButton::Init(const char* szName)
	{
		m_bValue = false;
		m_bHasTexture = false;
		InitBase(szName);
	}
	
	void GUIButton::InitAsTexture(GFXDevice* pDevice, const char* szName, TextureHndl pTexture)
	{
		m_bValue = false;
		if (!m_bTexDescValid)
		{
			static const DescriptorDeclaration decl[] =
			{
				DESCRIPTOR_ELEMENT(0, DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),
				DESCRIPTOR_END()
			};

			SamplerDecl samplerDecl(SF_LINEAR, SC_WRAP);
			m_sampler = pDevice->GetSampler(samplerDecl);

			DescriptorSetLayoutHndl layout = pDevice->GetDescriptorSetLayout(decl);
			m_descriptor.Init(pDevice, layout);
			m_descriptor.SetImageSamplerPairAtBinding(0, pTexture, m_sampler);
			m_descriptor.UpdateDescriptors(pDevice);
		}
		m_pTexture = pTexture;
		m_bHasTexture = true;
		m_bTexDescValid = true;
		InitBase(szName);
	}

	void GUIButton::CleanUp(GFXDevice* pDevice)
	{
		if (m_bTexDescValid)
		{
			m_descriptor.CleanUp(pDevice);
		}
	}

	bool GUIButton::UpdateAndAddToDrawList()
	{
		bool bResult = false;
		UpdateBase();
		if(m_bHasTexture)
		{
			Vector2f vScale = m_vUVMax - m_vUVMin;
			ImVec2 vSize((float)m_pTexture->GetWidth()*vScale.x, (float)m_pTexture->GetHeight()*vScale.y);
			ImVec2 vUVMin(m_vUVMin.x, m_vUVMin.y);
			ImVec2 vUVMax(m_vUVMax.x, m_vUVMax.y);
			bResult = ImGui::ImageButton((void*)&m_descriptor, vSize, vUVMin, vUVMax);
		}
		else
		{
			bResult = ImGui::Button(GetName());
		}
		SetValue(bResult);

		return bResult;
	}

	void GUIMenuItem::Init(const char* szName, const char* szShortCut)
	{
		InitBase(szName);
		if (szShortCut)
		{
			m_szShortCut = szShortCut;
		}
	}

	void GUIMenuItem::Run()
	{
		if (m_pCallbacks)
		{
			m_pCallbacks->FileOption(m_szName);
		}
	}

	bool GUIMenuItem::UpdateAndAddToDrawList()
	{
		if (ImGui::MenuItem(GetName(), m_szShortCut.c_str(), false, m_bEnabled))
		{
			Run();
		}
		
		return false;
	}

	void GUIMenuLoadSave::AddFilter(const char* szDisplay, const char* szPattern)
	{
		m_filterStrings.push_back(usg::string(szDisplay));
		m_filterStrings.push_back(usg::string(szPattern));
	}

	void GUIMenuLoadSave::SetExtension(const char* szExt)
	{
		m_szExt = szExt;
	}

	void GUIMenuLoadSave::Run()
	{
		usg::vector<usg::FileOpenPath::Filter> filters;
		filters.resize(m_filterStrings.size() / 2);
		for (size_t i = 0; i < filters.size(); i ++)
		{
			filters[i].szDisplayName = m_filterStrings[i * 2].c_str();
			filters[i].szExtPattern = m_filterStrings[(i * 2)+1].c_str();
		}

		FileOpenPath fileName;
		fileName.szWindowTitle = m_szName;
		fileName.szDefaultExt = m_szExt.size() > 0 ? m_szExt.c_str() : nullptr;
		fileName.pFilters = filters.data();
		fileName.uFilterCount = (uint32)filters.size();
		fileName.szOpenDir = m_szPath.size() > 0 ? m_szPath.c_str() : nullptr;

		if (m_bSave)
		{
			if (File::UserFileSavePath(fileName))
			{
				if (m_pCallbacks)
				{
					m_pCallbacks->LoadCallback(m_szName, fileName.szPathOut, fileName.szRelativePathOut);
				}
			}
		}
		else
		{
			if (File::UserFileOpenPath(fileName))
			{
				if (m_pCallbacks)
				{
					m_pCallbacks->LoadCallback(m_szName, fileName.szPathOut, fileName.szRelativePathOut);
				}
			}
		}

	}

	void GUIText::Init(const char* szName)
	{
		InitBase(szName);
		m_color.Assign(1.0f, 1.0f, 1.0f, 1.0f);
	}

	void GUIText::SetText(const char* szName)
	{
		InitBase(szName);
	}

	void GUIText::SetColor(const Color& color)
	{
		m_color = color;
	}

	bool GUIText::UpdateAndAddToDrawList()
	{
		const Color& col = GetColor();
		ImVec4 imColor(col.r(), col.g(), col.b(), col.a());
		ImGui::TextColored(imColor, GetName());

		return false;
	}

	void GUIColorSelect::Init(const char* szName)
	{
		m_color[0] = 0.f;
		m_color[1] = 0.0f;
		m_color[2] = 0.0f;
		m_color[3] = 255.f;
		InitBase(szName);
	}

	void GUIColorSelect::SetValue(const Color& color)
	{
		m_color[0] = color.r() *255.f;
		m_color[1] = color.g() *255.f;
		m_color[2] = color.b() *255.f;
		m_color[3] = color.a() *255.f;
	}

	bool GUIColorSelect::UpdateAndAddToDrawList()
	{
		UpdateBase();
		Color cReturn = GetValue();
		bool bChanged = ImGui::ColorEdit4(GetName(), cReturn.m_rgba);

		SetValue(cReturn);

		return bChanged;
	}

	void GUIComboBox::Init(const char* szName, const char** szStringArray, uint32 uSelected)
	{
		InitBase(szName);
		m_uSelected = uSelected;
		m_selectedName = szStringArray[0];
		m_szNames = szStringArray;
		while (szStringArray[m_uItems] != NULL)
		{
			m_uItems++;
		}
		
	}

	void GUIComboBox::Init(const char* szName, const char* szZeroSeperatedString, uint32 uSelect)
	{
		m_uSelected = 0;
		m_selectedName = szZeroSeperatedString;
		InitBase(szName);
		UpdateOptions(szZeroSeperatedString);
		// We're using a single string
	}

	void GUIComboBox::UpdateOptions(const char* szZeroSeperatedString)
	{
		m_szNames = NULL;
		m_szZeroSepNames = szZeroSeperatedString;
		if(m_selectedName.Length() > 0)
		{
			const char* index = szZeroSeperatedString;
			uint32 uIndex = 0;
			while(*index != '\0')
			{
				if(m_selectedName == index)
				{
					m_uSelected = uIndex;
					break;
				}
				index+= str::StringLength(index)+1;
				uIndex++;
			}
		}
		m_uItems = USG_INVALID_ID;
	}


	void GUIComboBox::SetSelected(uint32 uSelected)
	{ 
		if(uSelected>=m_uItems)
		{
			uSelected = 0;
			DEBUG_PRINT("Warning, combo box out of range");
		}

		m_uSelected = uSelected;
		if(m_uItems != USG_INVALID_ID)
		{
			m_selectedName = m_szNames[uSelected];
		}
		else
		{
			const char* index = m_szZeroSepNames;
			for(uint32 i=0; i<(uint32)uSelected; i++)
			{
				index+= str::StringLength(index)+1;
			}
			m_selectedName = index;
		}
	}

	void GUIComboBox::SetSelectedByName(const char* szName)
	{
		if(m_uItems == USG_INVALID_ID)
		{
			const char* index = m_szZeroSepNames;
			uint32 uIndex = 0;
			while(*index != '\0')
			{
				if(str::Compare(szName, index))
				{
					m_uSelected = uIndex;
					m_selectedName = index;
					return;
				}
				index+= str::StringLength(index)+1;
				uIndex++;
			}
		}
		else
		{
			for (uint32 i=0; i<m_uItems; i++)
			{
				if(str::Compare(szName, m_szNames[i]))
				{
					m_uSelected = i;
					m_selectedName = m_szNames[i];
					return;
				}
			}
		}
	}

	bool GUIComboBox::UpdateAndAddToDrawList()
	{
		int out = m_uSelected;
		bool bChanged = false;
		if(m_uItems == USG_INVALID_ID)
		{
			bChanged = ImGui::Combo(m_szName, &out, m_szZeroSepNames, m_uItems);
			if(bChanged)
			{
				const char* index = m_szZeroSepNames;
				for(uint32 i=0; i<(uint32)out; i++)
				{
					index+= str::StringLength(index)+1;
				}
				m_selectedName = index;
			}
		}
		else
		{
			bChanged = ImGui::Combo(m_szName, &out, m_szNames, m_uItems);
			m_selectedName = m_szNames[out];
		}
		m_uSelected = (uint32)out;
		

		return bChanged;
	}


	void GUISlider::Init(const char* szName, float fMin, float fMax, float fDefault)
	{
		Init(szName, fMin, fMax, &fDefault, 1);
	}

	void GUISlider::Init(const char* szName, float fMin, float fMax, const float* fDefault, uint32 uItems)
	{
		ASSERT(uItems>0 && uItems < MAX_ITEMS);
		m_uCount = uItems;
		InitBase(szName);
		for(uint32 i=0; i<m_uCount; i++)
		{
			m_fValue[i] = fDefault[i];
		}
		m_fMinValue = fMin;
		m_fMaxValue = fMax;
	}

	bool GUISlider::UpdateAndAddToDrawList()
	{
		bool bChanged = false;
		UpdateBase();

		switch(m_uCount)
		{
		case 1:
			bChanged = ImGui::SliderFloat(m_szName, m_fValue, m_fMinValue, m_fMaxValue);
			break;
		case 2:
			bChanged = ImGui::SliderFloat2(m_szName, m_fValue, m_fMinValue, m_fMaxValue);
			break;
		case 3:
			bChanged = ImGui::SliderFloat3(m_szName, m_fValue, m_fMinValue, m_fMaxValue);
			break;
		case 4:
			bChanged = ImGui::SliderFloat4(m_szName, m_fValue, m_fMinValue, m_fMaxValue);
			break;
		default:
			ASSERT(false);
		}

		return bChanged;
	}

	void GUICheckBox::Init(const char* szName, bool bDefault)
	{
		InitBase(szName);
		m_bValue = bDefault;
	}
	
	bool GUICheckBox::UpdateAndAddToDrawList()
	{
		UpdateBase();
		bool bRet =  ImGui::Checkbox(m_szName, &m_bValue);
		return bRet;
	}

	void GUIIntInput::Init(const char* szName, int* pDefaults, const uint32 uCount, const int iMin, const int iMax)
	{
		InitBase(szName);
		m_uCount = uCount;
		m_iMin = iMin;
		m_iMax = iMax;
		ASSERT(m_uCount <= MAX_VALUES && m_uCount > 0);
		SetValues(pDefaults);
	}

	void GUIIntInput::SetValues(int* pValues)
	{
		for(uint32 i = 0; i < m_uCount; i++)
		{
			m_values[i] = Math::Clamp(pValues[i], m_iMin, m_iMax);
		}
	}
	
	bool GUIIntInput::UpdateAndAddToDrawList()
	{
		bool bChanged = false;
		UpdateBase();

		switch(m_uCount)
		{
		case 1:
			bChanged = ImGui::InputInt(m_szName, m_values);
			break;
		case 2:
			bChanged = ImGui::InputInt2(m_szName, m_values);
			break;
		case 3:
			bChanged = ImGui::InputInt3(m_szName, m_values);
			break;
		case 4:
			bChanged = ImGui::InputInt4(m_szName, m_values);
			break;
		default:
			ASSERT(false);
		}

		SetValues(m_values);

		return bChanged;
	}
	GUITexture::GUITexture()
	{
		m_pTexture = nullptr;
		m_bDescValid = false;
		m_vUVMin.Assign(0.0f, 0.0f);
		m_vUVMax.Assign(1.0f, 1.0f);
	}
	
	GUITexture::~GUITexture()
	{
	}

	void GUITexture::Init(GFXDevice* pDevice, const char* szName, Vector2f vSize, usg::TextureHndl pTex)
	{
		if (!m_bDescValid)
		{
			static const DescriptorDeclaration decl[] =
			{
				DESCRIPTOR_ELEMENT(0, DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),
				DESCRIPTOR_END()
			};

			SamplerDecl samplerDecl(SF_LINEAR, SC_WRAP);
			m_sampler = pDevice->GetSampler(samplerDecl);

			DescriptorSetLayoutHndl layout = pDevice->GetDescriptorSetLayout(decl);
			m_descriptor.Init(pDevice, layout);
			m_bDescValid = true;
		}
		m_descriptor.SetImageSamplerPairAtBinding(0, pTex, m_sampler);
		m_descriptor.UpdateDescriptors(pDevice);

		InitBase(szName);
		m_pTexture = pTex;
		m_vScale = vSize;
	}

	void GUITexture::CleanUp(GFXDevice* pDevice)
	{
		m_descriptor.CleanUp(pDevice);
	}

	void GUITexture::SetUVs(Vector2f vUVMin, Vector2f vUVMax)
	{
		m_vUVMin = vUVMin;
		m_vUVMax = vUVMax;
	}

	void GUITexture::SetTexture(GFXDevice* pDevice, TextureHndl pTexture)
	{
		m_pTexture = pTexture;
		m_descriptor.SetImageSamplerPairAtBinding(0, pTexture, m_sampler);
		m_descriptor.UpdateDescriptors(pDevice);
	}

	void GUIButton::SetTexture(GFXDevice* pDevice, TextureHndl pTexture)
	{
		m_pTexture = pTexture;
		m_descriptor.SetImageSamplerPairAtBinding(0, pTexture, m_sampler);
		m_descriptor.UpdateDescriptors(pDevice);
	}
	
	bool GUITexture::UpdateAndAddToDrawList()
	{
		UpdateBase();
		ImVec2 vSize(m_vScale.x, m_vScale.y);
		ImVec2 vUV0(m_vUVMin.x, m_vUVMin.y);
		ImVec2 vUV1(m_vUVMax.x, m_vUVMax.y);
		ImVec4 vBorder(1.0f, 1.0f, 1.0f, 1.0f);
		ImVec4 vTint(1.0f, 1.0f, 1.0f, 1.0f);
		if (m_bDescValid)
		{
			ImGui::Image((void*)&m_descriptor, vSize, vUV0, vUV1, vTint, vBorder);
		}

		return false;
	}


	GUITextInput::GUITextInput()
	{
		m_input[0] = '\0';
		m_bUpdated = false;
	}

	GUITextInput::~GUITextInput()
	{

	}

	void GUITextInput::Init(const char* szName, const char* szDefault)
	{
		InitBase(szName);
		SetInput(szDefault);
	}

	void GUITextInput::SetInput(const char* szData)
	{
		str::Copy(m_input, szData, sizeof(m_input));
	}

	bool GUITextInput::UpdateAndAddToDrawList()
	{
		UpdateBase();
		m_bUpdated = ImGui::InputText(m_szName, m_input, sizeof(m_input));

		return m_bUpdated;
	}
	
	bool GUIFloat::UpdateAndAddToDrawList()
	{
		bool bChanged = false;
		UpdateBase();
		switch(m_uCount)
		{
		case 1:
			bChanged = ImGui::InputFloat(m_szName, m_fValue);
			break;
		case 2:
			bChanged = ImGui::InputFloat2(m_szName, m_fValue);
			break;
		case 3:
			bChanged = ImGui::InputFloat3(m_szName, m_fValue);
			break;
		case 4:
			bChanged = ImGui::InputFloat4(m_szName, m_fValue);
			break;
		default:
			ASSERT(false);
		}

		return bChanged;
	}

}

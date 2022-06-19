#include "Engine/Common/Common.h"
#include "Engine/Layout/Global2D.h"
#include "Engine/Audio/Audio.h"
#include "Engine/Resource/CustomEffectResource.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/Graphics/Effects/Effect.h"
#include "Engine/Graphics/Device/GFXContext.h"
#include "CustomUIItem.h"
#include "UI.h"
#include "UIWindow.h"


namespace usg
{

UIWindow::UIWindow()
{
	m_pCustomItemDefs = nullptr;
	m_pTextItemDefs = nullptr;
	m_pUIItemsDefs = nullptr;
	m_pButtonDefs = nullptr;
	m_bMatrixDirty = true;
	m_bVertsDirty = true;
	m_bEnabled = true;
	for (int i = 0; i < UI_ITEM_INVALID; i++)
	{
		m_uItemCounts[i] = 0;
	}
}

UIWindow::~UIWindow()
{
	ASSERT(m_pUIItemsDefs == nullptr);
}

void UIWindow::Init(usg::GFXDevice* pDevice, usg::ResourceMgr* pRes, const usg::RenderPassHndl& renderPass, const UIWindow* pParent, const UIDef& uiDef, const UIWindowDef& windowDef, usg::string path, bool bOffscreen)
{
	m_windowPos = windowDef.vPos;
	m_windowSize = windowDef.vSize;
	m_horAlign = windowDef.eHAlign;
	m_vertAlign = windowDef.eVAlign;
	m_name = windowDef.name;
	m_renderPass = renderPass;
	m_bEnabled = windowDef.bEnabled;
	m_path = path;

	m_uItemCounts[UI_ITEM_IMAGE] = windowDef.imageItems_count;
	m_uItemCounts[UI_ITEM_TEXT] = windowDef.textItems_count;
	m_uItemCounts[UI_ITEM_CUSTOM] = windowDef.customItems_count;
	m_uItemCounts[UI_ITEM_BUTTON] = windowDef.buttonItems_count;

	memsize uImageCount = m_uItemCounts[UI_ITEM_IMAGE] + m_uItemCounts[UI_ITEM_BUTTON];

	if (m_uItemCounts[UI_ITEM_IMAGE] || m_uItemCounts[UI_ITEM_TEXT] || m_uItemCounts[UI_ITEM_CUSTOM] || m_uItemCounts[UI_ITEM_BUTTON])
	{
		m_windowConstants.Init(pDevice, usg::g_global2DCBDecl);
		m_descriptor.Init(pDevice, pDevice->GetDescriptorSetLayout(usg::g_sGlobalDescriptors2D));
		m_descriptor.SetConstantSet(0, &m_windowConstants);
	}

	if (uImageCount > 0)
	{
		// TODO: Perhaps put this into the HUD class if all windows are going to use the same effect
		usg::EffectHndl effect = pRes->GetEffect(pDevice, "2DEffects.HUD");
		usg::CustomEffectResHndl effectDecl = effect->GetCustomEffect();

		m_runtimeEffect.Init(pDevice, effect->GetCustomEffect());
		const usg::CustomEffectResHndl& runtimeRes = m_runtimeEffect.GetResource();

		const ImageDef* pHudItems = windowDef.imageItems;
		const ButtonInstanceDef* pButtons = windowDef.buttonItems;
		m_vertices.resize(uImageCount);
		m_pUIItemsDefs = vnew(usg::ALLOC_GEOMETRY_DATA) HUDItemData[uImageCount];

		if (m_uItemCounts[UI_ITEM_BUTTON] > 0)
		{
			m_pButtonDefs = vnew(usg::ALLOC_GEOMETRY_DATA) ButtonData[m_uItemCounts[UI_ITEM_BUTTON]];
		}

		usg::SamplerDecl sampDecl(usg::SAMP_FILTER_LINEAR, usg::SAMP_WRAP_CLAMP);
		sampDecl.eMipFilter = usg::MIP_FILTER_POINT;
		sampDecl.LodBias = -0.25f;
		usg::SamplerHndl linear = pDevice->GetSampler(sampDecl);

		for (memsize i = 0; i < m_uItemCounts[UI_ITEM_IMAGE]; i++)
		{
			m_pUIItemsDefs[i].def = pHudItems[i];
			m_pUIItemsDefs[i].defOverride = pHudItems[i];
		}

		for(memsize i=0; i< m_uItemCounts[UI_ITEM_BUTTON]; i++)
		{
			memsize uImageId = m_uItemCounts[UI_ITEM_IMAGE] + i;
			const ButtonInstanceDef* pInstDef = &windowDef.buttonItems[i];
			const ButtonTemplateDef* pDef = GetUITemplate(uiDef, pInstDef->templateName);
			if(pDef)
			{
				m_pButtonDefs[i].def = windowDef.buttonItems[i];
				m_pButtonDefs[i].templateDef = *pDef;
			
				bool bActive = pInstDef->bActive;
				if (pDef->eType != UIButtonType::UIButtonType_Toggle)
				{
					bActive = true;
				}
				m_pButtonDefs[i].bActive = bActive;
				ImageDef_init(&m_pUIItemsDefs[uImageId].def);
				m_pUIItemsDefs[uImageId].def.cColor = pInstDef->bActive ? pDef->cEnabledColor : pDef->cDisabledColor;
				m_pUIItemsDefs[uImageId].def.cColor *= pDef->cNoHighlightColor;
				m_pUIItemsDefs[uImageId].def.eHAlign = pInstDef->eHAlign;
				m_pUIItemsDefs[uImageId].def.eVAlign = pInstDef->eVAlign;
				strcpy_s(m_pUIItemsDefs[uImageId].def.name, pInstDef->name);
				strcpy_s(m_pUIItemsDefs[uImageId].def.textureName, pDef->textureName);
				strcpy_s(m_pUIItemsDefs[uImageId].def.tags, pInstDef->tags);
				m_pUIItemsDefs[uImageId].def.vPos = pInstDef->vPos;
				m_pUIItemsDefs[uImageId].def.vSize = pInstDef->vSize;
				m_pUIItemsDefs[uImageId].def.bVisible = true;
				m_pButtonDefs[i].uActiveFrame = bActive ? pDef->uHighlightActiveFrameId : pDef->uHighlightInactiveFrameId;
				m_pButtonDefs[i].eAnimState = bActive ? ButtonAnimState::BUTTON_ANIM_STATE_ACTIVE : ButtonAnimState::BUTTON_ANIM_STATE_DEACTIVATED;
				
				SetButtonUVs(m_pUIItemsDefs[uImageId].def.vUVMin, m_pUIItemsDefs[uImageId].def.vUVMax, pDef, m_pButtonDefs[i].uActiveFrame);

				// Copy to the override
				m_pUIItemsDefs[uImageId].defOverride = m_pUIItemsDefs[uImageId].def;

				if (m_pButtonDefs[i].def.uActionCRC != 0)
				{
					memsize uActionIdx = GetActionIdIndex(m_pButtonDefs[i].def.uActionCRC);
					UIItemRef ref;
					ref.eType = UI_ITEM_BUTTON;
					ref.pWindow = this;
					ref.uItemIdx = (uint32)i;
					m_actionData[uActionIdx].bActive = bActive;
					m_actionData[uActionIdx].uiItems.push_back(ref);
				}
			}
		}

		for (memsize i = 0; i < uImageCount; i++)
		{
			m_pUIItemsDefs[i].descriptor.Init(pDevice, runtimeRes->GetDescriptorLayoutHndl());
			usg::string fullPath = m_path + m_pUIItemsDefs[i].def.textureName;
			m_pUIItemsDefs[i].descriptor.SetImageSamplerPairAtBinding(0, pRes->GetTextureAbsolutePath(pDevice, fullPath.c_str()), linear);
			m_pUIItemsDefs[i].descriptor.UpdateDescriptors(pDevice);
			// TODO: Get material indices, worst case scenario each element needs its own descriptor set
		}

		m_vertexData.Init(pDevice, nullptr, (uint32)(sizeof(VertexData)), (uint32)uImageCount, m_name.c_str() , usg::GPU_USAGE_DYNAMIC);

		usg::PipelineStateDecl pipeline;
		pipeline.ePrimType = usg::PT_POINTS;
		pipeline.inputBindings[0].Init(effectDecl->GetVertexElements());
		pipeline.uInputBindingCount = 1;

		usg::DescriptorSetLayoutHndl matDescriptors = effectDecl->GetDescriptorLayoutHndl();
		pipeline.layout.descriptorSets[0] = pDevice->GetDescriptorSetLayout(usg::g_sGlobalDescriptors2D);
		pipeline.layout.descriptorSets[1] = matDescriptors;
		pipeline.layout.uDescriptorSetCount = 2;

		usg::DepthStencilStateDecl& depthDecl = pipeline.depthState;


		usg::RasterizerStateDecl& rasterizerDecl = pipeline.rasterizerState;
		rasterizerDecl.eCullFace = usg::CULL_FACE_NONE;

		pipeline.pEffect = effect;

		usg::AlphaStateDecl& alphaDecl = pipeline.alphaState;
		alphaDecl.SetColor0Only();
		alphaDecl.bBlendEnable = true;
		if(bOffscreen)
		{
			alphaDecl.blendEq = usg::BLEND_EQUATION_ADD;
			alphaDecl.srcBlend = usg::BLEND_FUNC_SRC_ALPHA;
			alphaDecl.dstBlend = usg::BLEND_FUNC_ONE_MINUS_SRC_ALPHA;
			alphaDecl.blendEqAlpha = usg::BLEND_EQUATION_ADD;
			alphaDecl.srcBlendAlpha = usg::BLEND_FUNC_SRC_ALPHA;
			alphaDecl.dstBlendAlpha = usg::BLEND_FUNC_DST_ALPHA;
		}
		else
		{
			alphaDecl.blendEq = usg::BLEND_EQUATION_ADD;
			alphaDecl.srcBlend = usg::BLEND_FUNC_SRC_ALPHA;
			alphaDecl.dstBlend = usg::BLEND_FUNC_ONE_MINUS_SRC_ALPHA;
			alphaDecl.blendEqAlpha = usg::BLEND_EQUATION_ADD;
			alphaDecl.srcBlendAlpha = usg::BLEND_FUNC_SRC_ALPHA;
			alphaDecl.dstBlendAlpha = usg::BLEND_FUNC_DST_ALPHA;
		}

		m_pipelineState = pDevice->GetPipelineState(renderPass, pipeline);


		m_bVertsDirty = true;
	}
	else
	{
		m_bVertsDirty = false;
	} 

	if (m_uItemCounts[UI_ITEM_TEXT] > 0)
	{
		m_pTextItemDefs = vnew(usg::ALLOC_LAYOUT) TextItemData[m_uItemCounts[UI_ITEM_TEXT]];
		for (uint32 i = 0; i < m_uItemCounts[UI_ITEM_TEXT]; i++)
		{
			const TextItemDef& def = windowDef.textItems[i];
			m_pTextItemDefs[i].def = def;
			m_pTextItemDefs[i].text.Init(pDevice, pRes, renderPass);
			m_pTextItemDefs[i].text.SetFromKeyString(pDevice, pRes, def.keyStringCRC);
			m_pTextItemDefs[i].text.SetPosition(def.vPos.x, def.vPos.y);
			m_pTextItemDefs[i].text.SetColor(def.cColor);
			m_pTextItemDefs[i].text.SetBackgroundColor(def.cBgColor);
			m_pTextItemDefs[i].text.SetOriginTL(true);
			m_pTextItemDefs[i].originalText = m_pTextItemDefs[i].text.GetText().c_str();
			m_pTextItemDefs[i].text.SetAlign(GetTextAlign(def.eHAlign, def.eVAlign));
			m_pTextItemDefs[i].text.SetWidthLimit(def.fMaxWidth);
			usg::Vector2f vScale;
			m_pTextItemDefs[i].text.GetScale(vScale);
			m_pTextItemDefs[i].text.SetScale(vScale * def.fScale);

			if (m_pTextItemDefs[i].def.uActionCRC != 0)
			{
				memsize uActionIdx = GetActionIdIndex(m_pTextItemDefs[i].def.uActionCRC);
				UIItemRef ref;
				ref.eType = UI_ITEM_TEXT;
				ref.pWindow = this;
				ref.uItemIdx = (uint32)i;
				m_actionData[uActionIdx].uiItems.push_back(ref);
			}
		}
	}

	if (m_uItemCounts[UI_ITEM_CUSTOM] > 0)
	{
		m_pCustomItemDefs = vnew(usg::ALLOC_LAYOUT) CustomItemData[m_uItemCounts[UI_ITEM_CUSTOM]];
		for (uint32 i = 0; i < m_uItemCounts[UI_ITEM_CUSTOM]; i++)
		{
			const CustomItemDef& def = windowDef.customItems[i];
			m_pCustomItemDefs[i].def = def;
			m_pCustomItemDefs[i].pItem = nullptr;
		}
	}


	m_bMatrixDirty = true;

	// FIXME: Remove when spawning is delayed to the next frame
	Update(pParent, 0.0f, nullptr, nullptr);
	GPUUpdate(pDevice);
}


memsize UIWindow::GetActionIdIndex(uint32 uIdx)
{
	ASSERT(uIdx != 0);	// This means no action
	for (memsize i = 0; i < m_actionData.size(); i++)
	{
		if (m_actionData[i].uActionId == uIdx)
		{
			return i;
		}
	}
	m_actionData.push_back();
	m_actionData.back().uActionId = uIdx;
	return m_actionData.size()-1;
}


void UIWindow::SetButtonUVs(usg::Vector2f& vUVMin, usg::Vector2f& vUVMax, const ButtonTemplateDef* pButtonDef, uint32 uFrame)
{
	// Bit of a hack for the texture tool ODVS is using
	sint32 sFullImage = pButtonDef->uFramesVer * pButtonDef->uFramesHor;
	sint32 sEmptyStart = pButtonDef->uFramesVer - (sFullImage - pButtonDef->uTotalFrames);
	if (pButtonDef->uTotalFrames > 0 && sEmptyStart > 0)
	{
		// If using a more standard layout expect total frames to be 0
		for(uint32 i= sEmptyStart * pButtonDef->uFramesHor; i<=uFrame; i++)
		{
			if (((i + 1) % pButtonDef->uFramesHor) == 0)
			{
				uFrame++;
			}
		}
	}

	ASSERT(uFrame < (sEmptyStart * pButtonDef->uFramesHor) || ((uFrame+1) % 11) != 0);

	uint32 uXId = uFrame % pButtonDef->uFramesHor;
	uint32 uYId = uFrame / pButtonDef->uFramesHor;

	vUVMin.x = (float)uXId  / pButtonDef->uFramesHor;
	vUVMin.y = (float)uYId / pButtonDef->uFramesVer;

	vUVMax.x = vUVMin.x + (1.f/ pButtonDef->uFramesHor);
	vUVMax.y = vUVMin.y + (1.f / pButtonDef->uFramesVer);
}

const ButtonTemplateDef* UIWindow::GetUITemplate(const UIDef& uiDef, const char* szName)
{
	for (uint32 i = 0; i < uiDef.buttonDefinitions_count; i++)
	{
		if (strcmp(szName, uiDef.buttonDefinitions[i].templateName) == 0)
		{
			return &uiDef.buttonDefinitions[i];
		}
	}

	ASSERT(false);
	return nullptr;
}

void UIWindow::RegisterCustomItem(uint32 uIndex, class CustomUIItem* pItem)
{
	if (uIndex < m_uItemCounts[UI_ITEM_CUSTOM])
	{
		m_pCustomItemDefs[uIndex].pItem = pItem;
	}
}

int UIWindow::GetTextAlign(UIHorAlign eHorAlign, UIVertAlign eVerAlign)
{
	int iAlign = 0;
	switch (eHorAlign)
	{
	case UIHorAlign::UIHorAlign_Left:
		iAlign |= usg::TextAlign::kTextAlignLeft| usg::TextAlign::kTextAlignHOriginLeft;
		break;
	case UIHorAlign::UIHorAlign_Right:
		iAlign |= usg::TextAlign::kTextAlignRight | usg::TextAlign::kTextAlignHOriginRight;
		break;
	case UIHorAlign::UIHorAlign_CenterH:
		iAlign |= usg::TextAlign::kTextAlignCenter | usg::TextAlign::kTextAlignHOriginCenter;
		break;
	default:
		ASSERT(false);
	}

	switch (eVerAlign)
	{
	case UIVertAlign::UIVertAlign_Top:
		iAlign |= usg::TextAlign::kTextAlignVOriginTop;
		break;
	case UIVertAlign::UIVertAlign_Bottom:
		iAlign |= usg::TextAlign::kTextAlignVOriginBottom;
		break;
	case UIVertAlign::UIVertAlign_CenterV:
		iAlign |= usg::TextAlign::kTextAlignVOriginMiddle;
		break;
	default:
		ASSERT(false);
	}

	return iAlign;
}

usg::Vector2f UIWindow::GetPos(const usg::Vector2f& vBasePos, const usg::Vector2f& vSize, UIHorAlign eHorAlign, UIVertAlign eVerAlign, const UIWindow* pOwner)
{
	usg::Vector2f vReturn = vBasePos;
	usg::Vector2f vWindowPos(0.0f, 0.0f);
	usg::Vector2f vWindowBR = pOwner->GetSize();
	usg::Vector2f vWindowCenter = (vWindowPos*0.5f) + (vWindowBR*0.5f);
	switch (eHorAlign)
	{
	case UIHorAlign::UIHorAlign_Left:
		vReturn.x = vReturn.x;
		break; 
	case UIHorAlign::UIHorAlign_Right:
		vReturn.x = vWindowBR.x - vBasePos.x - vSize.x;
		break;
	case UIHorAlign::UIHorAlign_CenterH:
		vReturn.x = vReturn.x + vWindowCenter.x - (vSize.x/2.f);
		break;
	default:
		ASSERT(false);
	}

	switch (eVerAlign)
	{
	case UIVertAlign::UIVertAlign_Top:
		vReturn.y = vReturn.y;
		break; 
	case UIVertAlign::UIVertAlign_Bottom:
		vReturn.y = (vWindowBR.y - vBasePos.y) - vSize.y;
		break;
	case UIVertAlign::UIVertAlign_CenterV:
		vReturn.y = vReturn.y + vWindowCenter.y - (vSize.y/2.f);
		break;
	default:
		ASSERT(false);
	}

	return vReturn;
}

void UIWindow::SetPos(usg::Vector2f vPos)
{
	m_windowPos = vPos;
	m_bMatrixDirty = true;
}

void UIWindow::SetSize(usg::Vector2f vSize)
{
	m_windowSize = vSize;
	m_bMatrixDirty = true;
	m_bVertsDirty = true;
}


bool UIWindow::GetItemRef(const char* szName, UIItemType eType, UIItemRef& out)
{
	for (auto& itr : m_children)
	{
		if( itr->GetItemRef(szName, eType, out) )
		{
			return true;
		}
	}


	if (eType == UI_ITEM_BUTTON)
	{
		for (uint32 i = 0; i < m_uItemCounts[UI_ITEM_BUTTON]; i++)
		{
			if (strcmp(m_pButtonDefs[i].def.name, szName) == 0)
			{
				out.pWindow = this;
				out.uItemIdx = i;
				return true;
			}
		}
	}
	else if(eType == UI_ITEM_IMAGE)
	{
		for (uint32 i=0; i<(uint32)m_vertices.size(); i++)
		{
			if( strcmp(m_pUIItemsDefs[i].def.name, szName) == 0)
			{
				out.pWindow = this;
				out.uItemIdx = i;
				return true;
			}
		}
	}
	else if(eType == UI_ITEM_TEXT)
	{
		for (uint32 i=0; i< m_uItemCounts[UI_ITEM_TEXT]; i++)
		{
			if( strcmp(m_pTextItemDefs[i].def.name, szName) == 0)
			{
				out.pWindow = this;
				out.uItemIdx = i;
				return true;
			}
		}
	}
	else if (eType == UI_ITEM_CUSTOM)
	{
		for (uint32 i = 0; i < m_uItemCounts[UI_ITEM_CUSTOM]; i++)
		{
			if (strcmp(m_pCustomItemDefs[i].def.name, szName) == 0)
			{
				out.pWindow = this;
				out.uItemIdx = i;
				return true;
			}
		}
	}
	else
	{
		ASSERT(false);
	}
	return false;
}

void UIWindow::SetUVRange(uint32 uIndex, const usg::Vector2f& vUVMin, const usg::Vector2f& vUVMax)
{
	ImageDef& def = m_pUIItemsDefs[uIndex].defOverride;
	m_vertices[uIndex].vTexCoordRange.Assign(vUVMin.x, vUVMin.y, vUVMax.x, vUVMax.y);
	def.vUVMin = vUVMin;
	def.vUVMax = vUVMax;
	m_bVertsDirty = true;
}

void UIWindow::SetText(uint32 uIndex, const char* szText)
{
	if (uIndex < m_uItemCounts[UI_ITEM_TEXT])
	{
		m_pTextItemDefs[uIndex].text.SetText(szText);
	}
}

const char* UIWindow::GetOriginalText(uint32 uTextIdx) const
{
	if (uTextIdx >= m_uItemCounts[UI_ITEM_TEXT])
		return nullptr;
	return m_pTextItemDefs[uTextIdx].originalText.c_str();
}


void UIWindow::SetItemVisible(uint32 uIndex, enum UIItemType eType, bool bVisible)
{
	switch (eType)
	{
	case UI_ITEM_IMAGE:
	{
		ImageDef& defOverride = m_pUIItemsDefs[uIndex].defOverride;
		defOverride.bVisible = bVisible;

		break;
	}
	case UI_ITEM_BUTTON:
	{
		memsize uImageId = m_uItemCounts[UI_ITEM_IMAGE] + uIndex;
		ImageDef& defOverride = m_pUIItemsDefs[uImageId].defOverride;
		defOverride.bVisible = bVisible;

		break;
	}
	case UI_ITEM_TEXT:
	{
		TextItemDef& def = m_pTextItemDefs[uIndex].def;
		def.bVisible = bVisible;
		break;
	}
	default:
		ASSERT(false);
	}
}

void UIWindow::SetItemSize(uint32 uIndex, enum UIItemType eType, const usg::Vector2f& vSize, bool bRelative)
{
	switch (eType)
	{
	case UI_ITEM_IMAGE:
	{
		ImageDef& def = m_pUIItemsDefs[uIndex].def;
		ImageDef& defOverride = m_pUIItemsDefs[uIndex].defOverride;
		usg::Vector2f vAdjSize = bRelative ? def.vSize * vSize : vSize;
		m_vertices[uIndex].vSize = vAdjSize;
		usg::Vector2f vCorrectedPos = GetPos(def.vPos, m_vertices[uIndex].vSize, def.eHAlign, def.eVAlign, this);
		m_vertices[uIndex].vPosition = usg::Vector3f(vCorrectedPos.x, vCorrectedPos.y, 0.0f);

		defOverride.vSize = vAdjSize;

		m_bVertsDirty = true;
		break;
	}
	default:
		ASSERT(false);
	}
}

bool UIWindow::IsMouseInRangeOfButton(uint32 uButton)
{
	uint32 uIndex = m_uItemCounts[UI_ITEM_IMAGE] + uButton;
	

	const VertexData& vert = m_vertices[uIndex];
	// We use the vertices because they have been aligned
	if (m_vMousePos.x >= vert.vPosition.x
		&& m_vMousePos.y >= vert.vPosition.y
		&& m_vMousePos.x <= (vert.vPosition.x + vert.vSize.x)
		&& m_vMousePos.y <= (vert.vPosition.y + vert.vSize.y))
	{
		return true;
	}

	return false;
}

bool UIWindow::IsMouseInRangeOfText(uint32 uText)
{
	uint32 uIndex = m_uItemCounts[UI_ITEM_TEXT];

	if(!m_pTextItemDefs[uText].def.bVisible)
		return false;

	usg::Vector2f vMin, vMax;
	m_pTextItemDefs[uText].text.GetBounds(vMin, vMax);
	// We use the vertices because they have been aligned
	if (m_vMousePos.x >= vMin.x
		&& m_vMousePos.y >= vMin.y
		&& m_vMousePos.x <= (vMax.x)
		&& m_vMousePos.y <= (vMax.y))
	{
		return true;
	}

	return false;
}

void UIWindow::SetItemPos(uint32 uIndex, enum UIItemType eType, const usg::Vector2f& vPos, bool bRelative)
{
	switch (eType)
	{
	case UI_ITEM_IMAGE:
	{
		ImageDef& def = m_pUIItemsDefs[uIndex].def;
		ImageDef& defOut = m_pUIItemsDefs[uIndex].defOverride;
		usg::Vector2f vAdjPos = bRelative ? def.vPos + vPos : vPos;
		defOut.vPos = vAdjPos;
		//m_vertices[uIndex].vPosition.Assign(vAdjPos.x, vAdjPos.y, 0.0f);
		m_bVertsDirty = true;
		break;
	}
	case UI_ITEM_TEXT:
	{
		TextItemDef& def = m_pTextItemDefs[uIndex].def;
		usg::Vector2f vPos = bRelative ? def.vPos + vPos : vPos;
		m_pTextItemDefs[uIndex].text.SetPosition(vPos.x, vPos.y);
	}
	break;
	default:
		ASSERT(false);
	}
}

usg::Vector2f UIWindow::GetOriginalItemSize(uint32 uIndex, enum UIItemType eType) const
{
	switch (eType)
	{
	case UI_ITEM_IMAGE:
	{
		ImageDef& def = m_pUIItemsDefs[uIndex].def;
		return def.vSize;
	}
	case UI_ITEM_CUSTOM:
	{
		CustomItemDef& def = m_pCustomItemDefs[uIndex].def;
		return def.vSize;
	}
	default:
		ASSERT(false);
	}

	return usg::Vector2f::ZERO;
}

usg::Color UIWindow::GetOriginalColor(uint32 uIndex, enum UIItemType eType) const
{
	switch (eType)
	{
	case UI_ITEM_IMAGE:
	{
		ImageDef& def = m_pUIItemsDefs[uIndex].def;
		return def.cColor;
	}
	case UI_ITEM_TEXT:
	{
		TextItemDef& def = m_pTextItemDefs[uIndex].def;
		return def.cColor;
	}
	case UI_ITEM_CUSTOM:
	{
		CustomItemDef& def = m_pCustomItemDefs[uIndex].def;
		return def.cColor;
	}
	default:
		ASSERT(false);
	}

	return usg::Color::Black;
}

void UIWindow::SetItemColor(uint32 uIndex, enum UIItemType eType, const usg::Color& cColor)
{
	switch (eType)
	{
	case UI_ITEM_IMAGE:
	{
		m_vertices[uIndex].cColor = cColor;
		m_bVertsDirty = true;
		break;
	}
	case UI_ITEM_TEXT:
	{
		m_pTextItemDefs[uIndex].text.SetColor(cColor);
	}
	break;
	default:
		ASSERT(false);
	}
}

void UIWindow::SetButtonAnimState(uint32 uButton, ButtonAnimState eState)
{
	if(m_pButtonDefs[uButton].eAnimState == eState)
		return;
	
	m_pButtonDefs[uButton].eAnimState = eState;
	m_pButtonDefs[uButton].fStateTime = 0.0f;
}


void UIWindow::AddResult(UIResults* pResult, uint32 uCRC, UIActionType eAction, int iValue)
{
	if(!pResult)
		return;


	UIResult result;
	result.eActionType = eAction;
	result.actionCRC = uCRC;
	result.iValue = iValue;

	(*pResult)[uCRC] = result;
}

bool UIWindow::SetButtonHighlighted(uint32 uButton, bool bHighlighted, UIResults* pResults)
{
	ButtonData* pButton = &m_pButtonDefs[uButton];
	bool bColorChange = false;

	if (bHighlighted && !pButton->bHighlighted)
	{
		if (pButton->eAnimState == ButtonAnimState::BUTTON_ANIM_STATE_ACTIVE)
		{
			SetButtonAnimState(uButton, ButtonAnimState::BUTTON_ANIM_STATE_ACTIVE_HIGHLIGHT);
		}
		else if (pButton->eAnimState == ButtonAnimState::BUTTON_ANIM_STATE_DEACTIVATED)
		{
			SetButtonAnimState(uButton, ButtonAnimState::BUTTON_ANIM_STATE_DEACTIVATED_HIGHLIGHT);
		}

		pButton->bHighlighted = true;
		bColorChange = true;
	}
	else if (!bHighlighted && pButton->bHighlighted)
	{
		if (pButton->eAnimState == ButtonAnimState::BUTTON_ANIM_STATE_ACTIVE_HIGHLIGHT)
		{
			SetButtonAnimState(uButton, ButtonAnimState::BUTTON_ANIM_STATE_ACTIVE);
		}
		if (pButton->eAnimState == ButtonAnimState::BUTTON_ANIM_STATE_DEACTIVATED_HIGHLIGHT)
		{
			SetButtonAnimState(uButton, ButtonAnimState::BUTTON_ANIM_STATE_DEACTIVATED);
		}

		pButton->bHighlighted = false;
		bColorChange = true;
	}

	uint32 uImageId = m_uItemCounts[UI_ITEM_IMAGE] + uButton;
	m_pUIItemsDefs[uImageId].defOverride.cColor = pButton->bActive ? pButton->templateDef.cEnabledColor : pButton->templateDef.cDisabledColor;
	m_pUIItemsDefs[uImageId].defOverride.cColor *= pButton->bHighlighted ? pButton->templateDef.cHighlightColor : pButton->templateDef.cNoHighlightColor;

	return bColorChange;
}

bool UIWindow::IsPair(uint32 uButton) const 
{
	ButtonData* pButton = &m_pButtonDefs[uButton];
	return (pButton->templateDef.eType == UIButtonType::UIButtonType_Decrement || 
		pButton->templateDef.eType == UIButtonType::UIButtonType_Increment );
}

UIActionType UIWindow::SetButtonPressed(uint32 uButton, UIResults* pResults)
{
	ButtonData* pButton = &m_pButtonDefs[uButton];
	UIActionType eActionType = UI_ACTION_NONE;

	if (pButton->templateDef.eType == UIButtonType::UIButtonType_Select)
	{
		SetButtonAnimState(uButton, ButtonAnimState::BUTTON_ANIM_STATE_ACTIVATE);
		eActionType = UI_ACTION_ACTIVATED;
		pButton->bActive = true;
	}
	else if (pButton->templateDef.eType == UIButtonType::UIButtonType_Toggle)
	{
		// Don't allow you to deactivate the active group member, one should be active at all times
		if(pButton->bActive)
		{
			if (pButton->def.uGroupId == 0)
			{
				SetButtonAnimState(uButton, ButtonAnimState::BUTTON_ANIM_STATE_DEACTIVATE);
				eActionType = UI_ACTION_DEACTIVATED;
				pButton->bActive = false;
			}
		}
		else
		{
			SetButtonAnimState(uButton, ButtonAnimState::BUTTON_ANIM_STATE_ACTIVATE);
			eActionType = UI_ACTION_ACTIVATED;
			pButton->bActive = true;
		}
	}
	else if ( pButton->templateDef.eType == UIButtonType::UIButtonType_Decrement )
	{
		SetButtonAnimState(uButton, ButtonAnimState::BUTTON_ANIM_STATE_ACTIVATE);
		eActionType = UI_ACTION_DECREMENT;
		pButton->bActive = true;
	}
	else if (pButton->templateDef.eType == UIButtonType::UIButtonType_Increment)
	{
		SetButtonAnimState(uButton, ButtonAnimState::BUTTON_ANIM_STATE_ACTIVATE);
		eActionType = UI_ACTION_INCREMENT;
		pButton->bActive = true;
	}
	else
	{
		ASSERT(false);
	}


	// If this button is part of a group deactivate the other group members
	if (pButton->def.uGroupId != 0)
	{
		for (uint32 uOther = 0; uOther < m_uItemCounts[UI_ITEM_BUTTON]; uOther++)
		{
			if (uOther == uButton)
				continue;

			ButtonData* pOther = &m_pButtonDefs[uOther];
			if (pOther->def.uGroupId == pButton->def.uGroupId && pOther->bActive)
			{
				SetButtonAnimState(uOther, ButtonAnimState::BUTTON_ANIM_STATE_DEACTIVATE);
				pOther->bActive = false;

			}

			for (memsize i = 0; i < m_actionData.size(); i++)
			{
				if (m_actionData[i].uActionId == pOther->def.uActionCRC)
				{
					m_actionData[i].bActive = false;
				}
			}
		}
	}

	if (pButton->templateDef.uPressedSoundId != 0)
	{
		usg::Audio::Inst()->Prepare2DSound(pButton->templateDef.uPressedSoundId, 1.0f);
	}

	return eActionType;
}

void UIWindow::SetMousePos(const UIWindow* pParent, const UIInput* pInput, UIResults* pResults)
{
	usg::Vector2f vMousePos = pInput->vCursorLocation;
	bool bButtonPressed = pInput->bSelect;
	bool bColorChanged = false;

	usg::Vector3f vTempPos(vMousePos.x, vMousePos.y, 0.0f);
	usg::Matrix4x4 mProjInv;
	m_globalMatrix.GetInverse(mProjInv);
	vTempPos = mProjInv.TransformVec3(vTempPos);
	m_vMousePos.Assign(vTempPos.x, vTempPos.y);


	for (memsize i = 0; i < m_actionData.size(); i++)
	{
		UIActionType eAction = UI_ACTION_NONE;

		bool bHighlighted = false;
		for (auto itr : m_actionData[i].uiItems)
		{
			switch (itr.eType)
			{
				case UI_ITEM_TEXT:
				{
					bool bOver = IsMouseInRangeOfText( itr.uItemIdx );
					bHighlighted |= bOver;
	
					break;
				}
				case UI_ITEM_BUTTON:
				{
					bool bOver = IsMouseInRangeOfButton(itr.uItemIdx);
					if ( IsPair(itr.uItemIdx) )
					{
						if(bButtonPressed && bOver)
						{
							eAction = SetButtonPressed( itr.uItemIdx, pResults );
						}
						else
						{
							bColorChanged |= SetButtonHighlighted( itr.uItemIdx, bOver, pResults );
						}
					}
					bHighlighted |= bOver;
					break;
				}
				default:
					ASSERT(false);
			}
		}


		if(bButtonPressed && bHighlighted)
		{
			for (auto itr : m_actionData[i].uiItems)
			{
				switch (itr.eType)
				{
				case UI_ITEM_BUTTON:
					if( !IsPair(itr.uItemIdx) )
					{
						eAction = SetButtonPressed(itr.uItemIdx, pResults);
					}
					break;
				default:
					break;
				}
			}

			// For text without pairing
			if (eAction == UI_ACTION_NONE)
			{
				eAction = UI_ACTION_ACTIVATED;
			}
		}
		else if (eAction == UI_ACTION_NONE && bHighlighted != m_actionData[i].bHighlighted)
		{
			UIActionType eAction = bHighlighted && bButtonPressed ? UI_ACTION_ACTIVATED : UI_ACTION_NONE;
			for (auto itr : m_actionData[i].uiItems)
			{
				switch (itr.eType)
				{
				case UI_ITEM_TEXT:
					m_pTextItemDefs[itr.uItemIdx].text.SetColor(bHighlighted ? m_pTextItemDefs[itr.uItemIdx].def.cHighlightColor : m_pTextItemDefs[itr.uItemIdx].def.cColor);
					break;
				case UI_ITEM_BUTTON:				
					if( !IsPair(itr.uItemIdx) )
					{
						bColorChanged |= SetButtonHighlighted(itr.uItemIdx, bHighlighted, pResults);	
					}
					break;
				default:
					break;
				}
			}

			AddResult(pResults, m_actionData[i].uActionId, bHighlighted ? UI_ACTION_HIGHLIGHT : UI_ACTION_HIGHLIGHT_END);

			m_actionData[i].bHighlighted = bHighlighted;

		}

		if (eAction != UI_ACTION_NONE)
		{
			AddResult(pResults, m_actionData[i].uActionId, eAction);
		}

	}

	if (bColorChanged)
	{
		m_bVertsDirty = true;
	}
	
}


void UIWindow::SetButtonEnabled(uint32 uIndex, bool bEnabled)
{
	ButtonData* pButton = &m_pButtonDefs[uIndex];

	pButton->bActive = bEnabled;
	SetButtonAnimState(uIndex, bEnabled ? ButtonAnimState::BUTTON_ANIM_STATE_ACTIVE : ButtonAnimState::BUTTON_ANIM_STATE_DEACTIVATED);

	if (pButton->def.uGroupId != 0 && bEnabled)
	{
		for (uint32 uOther = 0; uOther < m_uItemCounts[UI_ITEM_BUTTON]; uOther++)
		{
			if (uOther == uIndex)
				continue;

			ButtonData* pOther = &m_pButtonDefs[uOther];
			if (pOther->def.uGroupId == pButton->def.uGroupId && pOther->bActive)
			{
				SetButtonAnimState(uOther, ButtonAnimState::BUTTON_ANIM_STATE_DEACTIVATED);
				pOther->bActive = false;
			}
		}
	}
}

void UIWindow::SetItemPos(const char* szName, usg::Vector2f vPos, bool bRelative)
{
	for (memsize i = 0; i < m_vertices.size(); i++)
	{
		if (strcmp(szName, m_pUIItemsDefs[i].def.name) == 0)
		{
			usg::Vector2f vAdjPos = GetPos(bRelative ? m_pUIItemsDefs[i].def.vPos + vPos : vPos, m_pUIItemsDefs[i].def.vSize, m_pUIItemsDefs[i].def.eHAlign, m_pUIItemsDefs[i].def.eVAlign, this);
			m_vertices[i].vPosition.Assign(vAdjPos.x, vAdjPos.y, 0.0f);
			m_bVertsDirty = true;
			return;
		}
	}

	for (auto& itr : m_children)
	{
		itr->SetItemPos(szName, vPos, bRelative);
	}
}


void UIWindow::UpdateButtons(float fElapsed)
{
	for (uint32 i = 0; i < m_uItemCounts[UI_ITEM_BUTTON]; i++)
	{
		uint32 uTargetFrame = m_pButtonDefs[i].uActiveFrame;
		uint32 uFrameOffset = (uint32)(m_pButtonDefs[i].fStateTime * m_pButtonDefs[i].templateDef.fFrameRate);
		const ButtonTemplateDef* pDef = &m_pButtonDefs[i].templateDef;
		switch (m_pButtonDefs[i].eAnimState)
		{
		case ButtonAnimState::BUTTON_ANIM_STATE_ACTIVE:
			uTargetFrame = pDef->uHighlightActiveFrameId;
			break;
		case ButtonAnimState::BUTTON_ANIM_STATE_DEACTIVATED:
			uTargetFrame = pDef->uHighlightInactiveFrameId;
			break;
		case ButtonAnimState::BUTTON_ANIM_STATE_DEACTIVATE:
			uTargetFrame = pDef->uDeactivateFrameId + usg::Math::Clamp(uFrameOffset, 0U, pDef->uDeactivateFrameCnt - 1);
			if (uFrameOffset >= pDef->uDeactivateFrameCnt)
			{
				SetButtonAnimState(i, ButtonAnimState::BUTTON_ANIM_STATE_DEACTIVATED);
			}
			break;
		case ButtonAnimState::BUTTON_ANIM_STATE_ACTIVATE:
			uTargetFrame = pDef->uActivateFrameId + usg::Math::Clamp(uFrameOffset, 0U, pDef->uActivateFrameCnt - 1);
			if (uFrameOffset >= pDef->uActivateFrameCnt)
			{
				SetButtonAnimState(i, ButtonAnimState::BUTTON_ANIM_STATE_ACTIVE);
			}
			break;
		case ButtonAnimState::BUTTON_ANIM_STATE_ACTIVE_HIGHLIGHT:
			uTargetFrame = pDef->uHighlightActiveFrameId + usg::Math::Clamp(uFrameOffset, 0U, pDef->uHighlightActiveFrameCnt - 1);
			if (uFrameOffset >= pDef->uHighlightActiveFrameCnt)
			{
				SetButtonAnimState(i, ButtonAnimState::BUTTON_ANIM_STATE_ACTIVE);
			}
			break;
		case ButtonAnimState::BUTTON_ANIM_STATE_DEACTIVATED_HIGHLIGHT:
			uTargetFrame = pDef->uHighlightInactiveFrameId + usg::Math::Clamp(uFrameOffset, 0U, pDef->uHighlightInactiveFrameCnt - 1);
			if (uFrameOffset >= pDef->uHighlightInactiveFrameCnt)
			{
				SetButtonAnimState(i, ButtonAnimState::BUTTON_ANIM_STATE_DEACTIVATED);
			}
			break;
		}

		if (uTargetFrame != m_pButtonDefs[i].uActiveFrame)
		{
			uint32 uImageId = m_uItemCounts[UI_ITEM_IMAGE] + i;
			SetButtonUVs(m_pUIItemsDefs[uImageId].defOverride.vUVMin, m_pUIItemsDefs[uImageId].defOverride.vUVMax, pDef, uTargetFrame);
			m_pButtonDefs[i].uActiveFrame = uTargetFrame;
			m_bVertsDirty = true;
			m_bMatrixDirty = true;
		}

		m_pButtonDefs[i].fStateTime += fElapsed;
	}
}

void UIWindow::Update(const UIWindow* pParent, float fElapsed, const UIInput* pInput, UIResults* pResults)
{
	if (m_bEnabled || m_bFirst)
	{
		if (m_bMatrixDirty)
		{
			UpdateMatrix(pParent);
		}

		if (pInput)
		{
			SetMousePos(this, pInput, pResults);
		}

		UpdateButtons(fElapsed);

		if (m_bVertsDirty || m_bMatrixDirty)
		{
			UpdateVertices();
		}

		for (uint32 i = 0; i < m_uItemCounts[UI_ITEM_CUSTOM]; i++)
		{
			if (m_pCustomItemDefs[i].pItem)
			{
				m_pCustomItemDefs[i].pItem->Update(fElapsed);;
			}
		}

		for (auto& itr : m_children)
		{
			if (m_bMatrixDirty)
			{
				itr->SetMatrixDirty();
			}
			itr->Update(this, fElapsed, pInput, pResults);
		}
	}
}

void UIWindow::UpdateVertices()
{
	// We need to update everything
	for (memsize i = 0; i < m_vertices.size(); i++)
	{
		const ImageDef& def = m_pUIItemsDefs[i].defOverride;
		// FIXME: Save overrides
		usg::Vector2f vCorrectedPos = GetPos(def.vPos, def.vSize, def.eHAlign, def.eVAlign, this);
		m_vertices[i].vPosition = usg::Vector3f(vCorrectedPos.x, vCorrectedPos.y, 0.0f);
		m_vertices[i].cColor = def.cColor;
		m_vertices[i].vTexCoordRange = usg::Vector4f(def.vUVMin.x, def.vUVMin.y, def.vUVMax.x, def.vUVMax.y);
		m_vertices[i].vSize = def.vSize;
	}
}


void UIWindow::UpdateMatrix(const UIWindow* pParent)
{
	if (m_bMatrixDirty)
	{
		if (pParent)
		{
			// FIXME: Should just be translation and scale
			usg::Vector2f vScale = m_windowSize / pParent->GetSize();
			usg::Vector2f vCorrectedPos = GetPos(m_windowPos, m_windowSize, m_horAlign, m_vertAlign, pParent);
			usg::Vector3f vOffset(vCorrectedPos.x, vCorrectedPos.y, 0.0f);
			m_localMatrix = usg::Matrix4x4::Identity();
			m_localMatrix.SetTranslation(vOffset);
			//m_localMatrix.Scale(m_windowSize.x, m_windowSize.y, 1.0f, 1.0f);
			m_globalMatrix = m_localMatrix * pParent->GetGlobalMatrix();
		}
		else
		{
			m_localMatrix.Orthographic(m_windowPos.x, m_windowPos.x + m_windowSize.x, m_windowPos.y + m_windowSize.y, m_windowPos.y, 0.0f, 100.0f);
			m_globalMatrix = m_localMatrix;
		}

		if (m_windowConstants.IsValid())
		{
			usg::Global2DConstants* pConstants = m_windowConstants.Lock<usg::Global2DConstants>();
			pConstants->mProjMat = m_globalMatrix;
			m_windowConstants.Unlock();
		}
	}
}

void UIWindow::GPUUpdate(usg::GFXDevice* pDevice)
{
	if (!m_bEnabled && !m_bFirst)
		return;

	// No GPU work if no renderables
	if (m_bMatrixDirty && m_windowConstants.GetSize() > 0) 
	{
		m_windowConstants.UpdateData(pDevice);
		m_descriptor.UpdateDescriptors(pDevice);
	}

	if (m_vertices.size() > 0 && m_bVertsDirty)
	{
		m_vertexData.SetContents(pDevice, m_vertices.data(), (uint32)m_vertices.size());
	}

	for (uint32 i = 0; i < m_uItemCounts[UI_ITEM_CUSTOM]; i++)
	{
		if (m_pCustomItemDefs[i].pItem)
		{
			if (!m_pCustomItemDefs[i].pItem->HasBeenRegistered())
			{
				const CustomItemDef& def = m_pCustomItemDefs[i].def;
				usg::Vector2f vPos = GetPos(def.vPos, def.vSize, def.eHAlign, def.eVAlign, this);
				m_pCustomItemDefs[i].pItem->AddToUI(pDevice, m_renderPass, vPos, def.vSize, def.cColor);
				m_pCustomItemDefs[i].pItem->Registered();

				ASSERT(m_pCustomItemDefs[i].pItem->HasBeenRegistered());
			}
			m_pCustomItemDefs[i].pItem->GPUUpdate(pDevice);
		}
	}

	for (uint32 i = 0; i < m_uItemCounts[UI_ITEM_TEXT]; i++)
	{
		m_pTextItemDefs[i].text.UpdateBuffers(pDevice);
	}

	for (auto& itr : m_children)
	{
		itr->GPUUpdate(pDevice);
	}

	m_bVertsDirty = false;
	m_bMatrixDirty = false;
	m_bFirst = false;
}

void UIWindow::CleanUpRecursive(usg::GFXDevice* pDevice)
{
	for (auto& itr : m_children)
	{
		itr->CleanUpRecursive(pDevice);
		vdelete(itr);
	}
	m_children.clear();

	if (m_pUIItemsDefs)
	{
		for (memsize i = 0; i < m_vertices.size(); i++)
		{
			m_pUIItemsDefs[i].descriptor.Cleanup(pDevice);
		}
		vdelete[] m_pUIItemsDefs;
		m_pUIItemsDefs = nullptr;
	}

	if (m_pTextItemDefs)
	{
		for (uint32 i = 0; i < m_uItemCounts[UI_ITEM_TEXT]; i++)
		{
			m_pTextItemDefs[i].text.Cleanup(pDevice);
		}
		vdelete[] m_pTextItemDefs;
		m_pTextItemDefs = nullptr;
	}

	if (m_pButtonDefs)
	{
		vdelete[] m_pButtonDefs;
		m_pButtonDefs = nullptr;
	}

	// We don't delete the pItem its self, assuming handled externally
	if (m_pCustomItemDefs)
	{
		vdelete[] m_pCustomItemDefs;
		m_pCustomItemDefs = nullptr;
	}

	m_descriptor.Cleanup(pDevice);
	m_vertexData.Cleanup(pDevice);
	m_windowConstants.Cleanup(pDevice);
}


UIWindow* UIWindow::CreateChildWindow(usg::GFXDevice* pDevice, usg::ResourceMgr* pRes, const usg::RenderPassHndl& renderPass, const UIDef& def, const UIWindowDef& windowDef, bool bOffscreen)
{
	UIWindow* pChild = vnew(usg::ALLOC_OBJECT)UIWindow;
	pChild->Init(pDevice, pRes, renderPass, this, def, windowDef, m_path, bOffscreen);

	m_children.push_back(pChild);
	return pChild;
}

UIWindow* UIWindow::GetChildWindow(const usg::string& name)
{
	for (uint32 i = 0; i < m_children.size(); i++)
	{
		if (m_children[i]->GetName() == name)
			return m_children[i];

		UIWindow* pWindow = m_children[i]->GetChildWindow(name);
		if (pWindow)
		{
			return pWindow;
		}
	}

	return nullptr;
}

void UIWindow::Draw(usg::GFXContext* pContext)
{
	if (!m_bEnabled)
		return;

	// First draw ourselves
	if (m_vertices.size() > 0)
	{
		pContext->SetPipelineState(m_pipelineState);
		pContext->SetDescriptorSet(&m_descriptor, 0);
		pContext->SetVertexBuffer(&m_vertexData, 0);

		for (uint32 i = 0; i < (uint32)m_vertices.size(); i++)
		{
			if(m_pUIItemsDefs[i].defOverride.bVisible)
			{
				pContext->SetDescriptorSet(&m_pUIItemsDefs[i].descriptor, 1);
				pContext->DrawImmediate(1, i);
			}
		}
	}

	for (uint32 i = 0; i < m_uItemCounts[UI_ITEM_CUSTOM]; i++)
	{
		if (m_pCustomItemDefs[i].pItem)
		{
			pContext->SetDescriptorSet(&m_descriptor, 0);
			m_pCustomItemDefs[i].pItem->Draw(pContext);;
		}
	}

	for (uint32 i = 0; i < m_uItemCounts[UI_ITEM_TEXT]; i++)
	{
		if(m_pTextItemDefs[i].def.bVisible)
		{
			pContext->SetDescriptorSet(&m_descriptor, 0);
			m_pTextItemDefs[i].text.Draw(pContext);
		}
	}

	// Then the children
	for (auto& child : m_children)
	{
		child->Draw(pContext);
	}
}

}
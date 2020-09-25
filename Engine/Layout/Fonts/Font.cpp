/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/Core/ProtocolBuffers/ProtocolBufferFile.h"
#include "Engine/Graphics/Textures/Texture.h"
#include "Engine/Core/String/String_Util.h"
#include "Engine/Graphics/StandardVertDecl.h"
#include "Engine/Graphics/Device/RenderState.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Font.h"

namespace usg
{

	const DescriptorDeclaration Font::m_sDescriptorDecl[] =
	{
		DESCRIPTOR_ELEMENT(0,						 DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),
		DESCRIPTOR_END()
	};

	Font::Font() : 
		ResourceBase(StaticResType),
		m_pTexture(NULL)
	{
	}

	Font::~Font()
	{
		
	}

	void Font::Cleanup(GFXDevice* pDevice)
	{
		m_descriptor.Cleanup(pDevice);
	}

	bool Font::Load( GFXDevice* pDevice, ResourceMgr* pResMgr, const char* filename )
	{
		m_name = filename;
		SetupHash( m_name.CStr() );
		
		U8String szFilename = filename;

		m_descriptor.Init(pDevice, pDevice->GetDescriptorSetLayout(m_sDescriptorDecl));


		U8String szTemp = "";
		szTemp.ParseString("%s.vpb", szFilename.CStr());

		// @todo it may actually not be so hot to use usg::text::FontDefinition at runtime. investigate.
		ProtocolBufferFile viteifont(szTemp.CStr());
		bool bReadSucceeded = viteifont.Read(&m_fontDefinition);
		ASSERT(bReadSucceeded);

		szTemp.ParseString("%s", szFilename.CStr());
		m_pTexture = pResMgr->GetTextureAbsolutePath(pDevice, szTemp.CStr());
		ASSERT(m_pTexture.get() != NULL);
		SamplerDecl pointDecl(SF_LINEAR, SC_CLAMP);
		pointDecl.eAnisoLevel = SamplerDecl::ANISO_LEVEL_16;

		m_descriptor.SetImageSamplerPair(0, m_pTexture, pDevice->GetSampler(pointDecl));
		m_descriptor.UpdateDescriptors(pDevice);

		SetReady(true);

		return true;

	}

	float Font::GetStringWidth(const char* string, float height) const
	{
		float totalWidth = 0.0f;
		for(const char* c = string; (*c)!='\0'; c++)
		{
			totalWidth += GetCharacterWidth(*c, height);
		}
		return totalWidth;
	}

	bool Font::HasCharacter(uint32 uChar) const
	{
		usg::text::CharDefinition* pChars = m_fontDefinition.Chars.Get().array;
		for (uint32 i = 0; i < m_fontDefinition.Chars.Get().count; i++)
		{
			::usg::text::CharDefinition& ch = pChars[i];

			if (ch.CharData == uChar)
			{
				return true;
			}
		}

		return false;
	}

	bool Font::GetCharacterCoords(uint32 uChar, float& fLeft, float& fRight, float& fTop, float& fBottom) const
	{
		bool bContains = false;
		usg::text::CharDefinition* pChars = m_fontDefinition.Chars.Get().array;
		for (uint32 i=0; i<m_fontDefinition.Chars.Get().count; i++)
		{
			::usg::text::CharDefinition& ch = pChars[i];

			if (ch.CharData == uChar)
			{
				fLeft = ch.UV_TopLeft.x;
				fTop = ch.UV_TopLeft.y;
				fRight = ch.UV_BottomRight.x;
				fBottom = ch.UV_BottomRight.y;
				bContains = true;
				break;
			}
		}

		// if this assertion fails, you tried to look up a glyph that's not
		// defined - re-build things.
		//ASSERT(bContains);

		return bContains;
	}

	float Font::GetCharacterWidth(uint32 uChar, float height) const
	{
		float fLeft = 0.0f;
		float fRight = 0.0f;
		float fTop = 0.0f;
		float fBottom = 0.0f;

		bool bContains = GetCharacterCoords(uChar, fLeft, fRight, fTop, fBottom);
		if (!bContains)
		{
			return 0.0f;
		}

		float uvWidth = fRight - fLeft;
		float uvHeight = -(fTop - fBottom);

		return height * (uvWidth / uvHeight);
	}

	float Font::GetCharacterAspect(uint32 uChar) const
	{
		Vector2f vSize = GetCharacterSize(uChar);

		if (vSize.x <= 0.0f)
			return 0.f;

		return vSize.x / vSize.y;
		
	}

	float Font::GetCharacterAspect(float fLeft, float fRight, float fTop, float fBottom) const
	{
		Vector2f vSize = GetCharacterSize(fLeft, fRight, fTop, fBottom);

		if (vSize.x <= 0.0f)
			return 0.f;

		return vSize.x / vSize.y;
	}

	Vector2f Font::GetCharacterSize(uint32 uChar) const
	{
		float fLeft = 0.0f;
		float fRight = 0.0f;
		float fTop = 0.0f;
		float fBottom = 0.0f;

		bool bContains = GetCharacterCoords(uChar, fLeft, fRight, fTop, fBottom);
		if (!bContains)
		{
			return Vector2f(0.0f, 0.0f);
		}

		return GetCharacterSize(fLeft, fRight, fTop, fBottom);
	}

	Vector2f Font::GetCharacterSize(float fLeft, float fRight, float fTop, float fBottom) const
	{
		float uvWidth = fRight - fLeft;
		float uvHeight = -(fTop - fBottom);

		return Vector2f(uvWidth * m_pTexture->GetWidth(), uvHeight*m_pTexture->GetHeight());
	}
}

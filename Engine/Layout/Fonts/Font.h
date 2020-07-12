/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_GRAPHICS_FONTS_FONT_H_
#define _USG_GRAPHICS_FONTS_FONT_H_


#include "Engine/Core/String/U8String.h"
#include "Engine/Resource/ResourceBase.h"
#include "Engine/Layout/Fonts/TextStructs.pb.h"
#include "Engine/Graphics/Device/DescriptorSet.h"

namespace usg
{
	class Font : public ResourceBase
	{
	public:
		Font ();
		virtual ~Font();

		bool Load(GFXDevice* pDevice, ResourceMgr* pResMgr, const char* const filename);
		virtual void CleanUp(GFXDevice* pDevice) override;



		// Platform specific code
		float	GetStringWidth(const char* string, float height) const;

		bool	GetCharacterCoords(uint32 uChar, float& fLeft, float& fRight, float& fTop, float& fBottom) const;
		float	GetCharacterWidth(uint32 uChar, float height) const;
		
		const DescriptorSet& GetDescriptor() const { return m_descriptor; }
		const U8String& GetName    () const{ return m_name; }

		static const DescriptorDeclaration m_sDescriptorDecl[];

		Vector2f GetCharacterSize(uint32 uChar) const;
		Vector2f GetCharacterSize(float fLeft, float fRight, float fTop, float fBottom) const;

		float GetCharacterAspect(uint32 uChar) const;
		float GetCharacterAspect(float fLeft, float fRight, float fTop, float fBottom) const;

		float GetCharacterSpacing() const { return m_fontDefinition.Spacing; }

		float GetBaseOffset() const { return m_fontDefinition.LowerOffset; }
		float GetDrawScale() const { return m_fontDefinition.DrawScale; }	// There is padding and the lower part of the character to take into account

		const static ResourceType StaticResType = ResourceType::FONT;
	private:
		TextureHndl		m_pTexture;
		DescriptorSet	m_descriptor;

		usg::text::FontDefinition m_fontDefinition;

		U8String m_name;
	};
}

#endif /* _USG_GRAPHICS_FONTS_FONT_H_ */

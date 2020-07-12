/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef __usg_graphics_text__
#define __usg_graphics_text__


#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Layout/Fonts/TextDrawer.h"
#include "Engine/Layout/Fonts/Font.h"
#include "Engine/Layout/Fonts/TextEnums.h"
#include "Engine/Resource/ResourceDecl.h"

namespace usg
{
	class GFXContext; 

	class Text
	{
	public:
		Text();
		~Text();

		void Init(GFXDevice* pDevice, ResourceMgr* pResMgr, const RenderPassHndl& renderPass);
		void CleanUp(GFXDevice* pDevice);
		void UpdateBuffers(GFXDevice* pDevice);
		bool Draw(GFXContext* context, bool b3D = false);
		void SetFromKeyString(GFXDevice* pDevice, usg::ResourceMgr* pMgr, uint32 uCRC);
		bool SetText(const char* str);
		bool SetText(const U8String& u8Str);
		bool SetFont(FontHndl pFont);
		bool SetPosition(float x, float y, float z = 0.0f);
		bool SetColor(const Color& color);
		void SetOriginTL(bool bTL) { m_drawer.SetOriginTL(bTL); }
		bool SetGradationStartColor(Color newcolor);
		bool SetGradationEndColor(Color newcolor);
		bool SetBackgroundColor(Color newColor);
		bool SetAlign(unsigned int flags);
		bool SetWidthLimit(float limit);
		bool SetLineSpacing(float spacing);
		bool SetCharSpacing(float spacing);
		bool SetScale(Vector2f scale);
		bool SetTabWidth(int tabWidth);

		const U8String& GetText() const;
		uint32 GetTextLength() const;
		void GetPosition(unsigned int* outX, unsigned int* outY) const;
		void GetPosition(float* outX, float* outY) const;
		float GetDepth() const { return m_positionZ; }
		void GetColor(unsigned char* outR, unsigned char* outG, unsigned char* outB, unsigned char* outA) const;
		const Color& GetColor() const { return m_color; }
		Color GetGradationStartColor() const { return m_gradationStartColor; }
		Color GetGradationEndColor() const { return m_gradationEndColor; }
		Color GetBackgroundColor() const { return m_backgroundColor; }
		uint32 GetAlign() const;
		float GetWidthLimit () const;
		float GetLineSpacing() const;
		float GetCharSpacing() const;
		void GetScale( Vector2f &out ) const;
		int GetTabWidth() const;
		const FontHndl& GetFont() const { return m_pFont; }
		uint32 GetDirtyFlags() const { return m_dirtyFlags; }

		void PrintDebugInfo() const;

		enum DirtyFlag
		{
			kDirtyColor                 = (1 << 1),
			kDirtyGradationColor        = (1 << 2),
			kDirtyAlign                 = (1 << 4),
			kDirtyWidthLimit            = (1 << 5),
			kDirtyLineSpacing           = (1 << 6),
			kDirtyCharSpacing           = (1 << 7),
			kDirtyPosition              = (1 << 8),
			kDirtyScale                 = (1 << 9),
			kDirtyTabWidth              = (1 << 10),
			kDirtyText					= (1 << 11),	// We've changed the string
			kDirtyBgColor				= (1 << 12),
		};


	private:
		TextDrawer	m_drawer;

		U8String    m_cachedString;
		FontHndl	m_pFont;

		float		m_positionX;
		float		m_positionY;
		float		m_positionZ;

		Color		m_color;
		Color		m_gradationStartColor;
		Color		m_gradationEndColor;
		Color		m_backgroundColor;

		uint32		m_alignment;
		float		m_widthLimit;
		float		m_lineSpacing;
		float		m_charSpacing;
		Vector2f	m_scale;
		int			m_tabWidth;
		uint32		m_dirtyFlags;
	};
}

#endif /* __usg_graphics_text__ */

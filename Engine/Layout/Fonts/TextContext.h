/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Maintains the current style settings for a set of text
*****************************************************************************/
#ifndef _USG_GRAPHICS_FONTS_TEXT_CONTEXT_H_
#define _USG_GRAPHICS_FONTS_TEXT_CONTEXT_H_


#include "Engine/Maths/Vector2f.h"

// TODO: Rewrite Text.h so that it doesn't duplicate this data
namespace usg
{
	class Text;

	class TextContext
	{
	public:
		static const uint8 TAG_RUBY = 0x0B;
		static const uint8 TAG_COLOR = 0x0C;
		static const uint8 TAG_SCALE = 0x0E;
		static const uint8 TAG_BOLD = 0x10;
		static const uint8 TAG_SHADOW = 0x11;
		static const uint8 TAGS_END = 0x12;

	public:
		TextContext();
		virtual ~TextContext();

		void Init(const Text* pInitData);

		bool ProcessTag(uint8 code, const char* &szText);

		void SetRubyScale(float rubyScale);

		enum Flags
		{
			CONTEXT_NO_CHAR_SPACE = 0x1
		};

		float GetScaleH() const { return m_vScale.x; }
		float GetScaleV() const { return m_vScale.y; }
		float GetCursorX() const { return m_vCursorPos.x; }
		float GetCursorY() const { return m_vCursorPos.y; }
		uint32 GetAlignFlags() const { return m_uAlignFlags;  }
		float GetWidthLimit() const { return m_fWidthLimit; }
		const Vector2f& GetScale() const { return m_vScale; }
		const Vector2f& GetCursorPos() const { return m_vCursorPos;  }
		const Color& GetColor() const { return m_color; }
	private:

		void	SetCursorPos(const Vector2f& vPos) { m_vCursorPos = vPos;  }
		void	MoveCursorX(float fOffset) { m_vCursorPos.x += fOffset;  }
		void	SetCursorX(float fValue) { m_vCursorPos.x = fValue;  }
		void	SetScale(const Vector2f& vScale) { m_vScale = vScale;  }
		void	SetScale(float fH, float fV) { m_vScale.Assign(fH, fV);  }
		void	SetAlignFlag(uint32 uFlag) { m_uAlignFlags |= uFlag;  }
		void	InitAlignFlags(uint32 uFlags) { m_uAlignFlags = uFlags;  }
		void	RemoveAlignFlag(uint32 uFlaG) { m_uAlignFlags = m_uAlignFlags & ~uFlaG;  }
		void	SetTextColor(const Color& color) { m_color = color;  }
		void	ResetWidthLimit();

		Color		m_color;
		Vector2f	m_vCursorPos;
		Vector2f	m_vScale;
		float		m_fRubyScale;
		uint32		m_uFlags;
		uint32		m_uAlignFlags;
		float		m_fWidthLimit;
	};

}

#endif //_USG_GRAPHICS_FONTS_TEXT_CONTEXT_H_

/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Text.h"

namespace usg
{
	Text::Text() : m_drawer(this)
	{
		m_positionX = 0.0f;
		m_positionY = 0.0f;
		m_color.Assign(1.0f, 1.0f, 1.0f, 1.0f);
		m_gradationStartColor.Assign(1.0f, 1.0f, 1.0f, 1.0f);
		m_gradationEndColor.Assign(1.0f, 1.0f, 1.0f, 1.0f);
		m_backgroundColor.Assign(0.0f, 0.0f, 0.0f, 0.0f);
		m_alignment = 0;
		m_widthLimit = 0.0f;
		m_lineSpacing = 0.0f;
		m_charSpacing = 0.0f;
		m_scale.Assign(1.0f, 1.0f);
		m_tabWidth = 4;
		m_dirtyFlags = 0;
	}

	Text::~Text()
	{
	}

	void Text::Init(GFXDevice* pDevice, const RenderPassHndl& renderPass)
	{
		m_drawer.Init(pDevice, renderPass);
	}

	void Text::CleanUp(GFXDevice* pDevice)
	{
		m_drawer.CleanUp(pDevice);
	}


	void Text::UpdateBuffers(GFXDevice* pDevice)
	{
		m_drawer.UpdateBuffers(pDevice);
		m_dirtyFlags = 0;
	}


	bool Text::Draw( GFXContext* context, bool b3D )
	{
		bool bReturn = m_drawer.Draw(context, b3D);
		return bReturn;
	}

	bool Text::SetText( const char* str )
	{
/*#ifdef _DEBUG
		DEBUG_PRINT("Text::SetText(const char*) is deprecated; use Text::SetText(const U8String) instead.\n");
#endif*/

		U8String u8TempStr(str);
		return SetText(u8TempStr);
	}

	bool Text::SetText( const U8String& u8Str )
	{
		// If the new string's length is longer than the cached string's length, recreate the buffers.
		if (u8Str == m_cachedString)
			return true;

		const uint32 newStrLength = u8Str.CharCount();

		// Null string...
		if( newStrLength == 0 )
		{
#ifdef TEXT_PRINT_WARNINGS
			DEBUG_PRINT("Text warning: Tried to set a string of zero length.\n");
#endif
			return false;
		}

		if( newStrLength > m_drawer.GetMaxStringLength() )
		{
			if( !m_drawer.Resize(newStrLength) )
			{
				DEBUG_PRINT("Text error: Failed to build buffers.\n");
				return false;
			}
		}

		// Store the new length and string.
		m_cachedString = u8Str;

		// Build the text command list.
		m_dirtyFlags |= kDirtyText;

		return true;
	}

	bool Text::SetFont( usg::FontHndl font )
	{
		m_pFont = font;
		return true;
	}

	bool Text::SetPosition( float x, float y, float z )
	{
		if( m_positionX == x && m_positionY == y && m_positionZ == z)
		{
			return true;
		}

		m_positionX = x;
		m_positionY = y;
		m_positionZ = z;
		m_dirtyFlags |= kDirtyPosition;

		return true;
	}


	bool Text::SetColor(const Color& color)
	{
		if (m_color == color)
		{
			return true;
		}

		m_color = color;
		m_dirtyFlags |= kDirtyColor;

		return true;
	}


	bool Text::SetGradationStartColor(Color newcolor)
	{
		if (m_gradationStartColor == newcolor)
		{
			return true;
		}
		m_gradationStartColor = newcolor;
		m_dirtyFlags |= kDirtyGradationColor;

		return true;
	}

	bool Text::SetGradationEndColor(Color newcolor)
	{
		if (m_gradationEndColor == newcolor)
		{
			return true;
		}
		m_gradationEndColor = newcolor;
		m_dirtyFlags |= kDirtyGradationColor;

		return true;
	}


	bool Text::SetBackgroundColor(Color newColor)
	{
		if (m_backgroundColor == newColor)
		{
			return true;
		}
		m_backgroundColor = newColor;
		m_dirtyFlags |= kDirtyBgColor;

		return true;
	}

	bool Text::SetAlign( unsigned int flags )
	{
		if( m_alignment == flags )
		{ return true; }

		m_alignment = flags;
		m_dirtyFlags |= kDirtyAlign;

		return true;
	}

	bool Text::SetWidthLimit( float limit )
	{
		if( usg::Math::IsEqual(m_widthLimit, limit) )
		{ return true; }

		m_widthLimit = limit;
		m_dirtyFlags |= kDirtyWidthLimit;

		return true;
	}

	bool Text::SetLineSpacing( float spacing )
	{
		if( usg::Math::IsEqual(m_lineSpacing, spacing) )
		{ return true; }

		m_lineSpacing = spacing;
		m_dirtyFlags |= kDirtyLineSpacing;

		return true;
	}

	bool Text::SetCharSpacing( float spacing )
	{
		if( usg::Math::IsEqual(m_charSpacing, spacing) )
		{ return true; }

		m_charSpacing = spacing;
		m_dirtyFlags |= kDirtyCharSpacing;

		return true;
	}


	bool Text::SetScale( Vector2f scale )
	{
		if (m_scale == scale) { return true; }

		m_scale.Assign(scale.x, scale.y);
		m_dirtyFlags |= kDirtyScale;

		return true;
	}

	bool Text::SetTabWidth( int tabWidth )
	{
		if (m_tabWidth == tabWidth) { return true; }

		m_tabWidth = tabWidth;
		m_dirtyFlags |= kDirtyTabWidth;

		return true;
	}

	const U8String& Text::GetText() const
	{
		return m_cachedString;
	}

	unsigned int Text::GetTextLength() const
	{
		return m_cachedString.Length();
	}

	void Text::GetPosition(unsigned int* outX, unsigned int* outY) const
	{
		// @deprecated - if your code crashes, retarget it to use the float version
		ASSERT(false);
		if (outX)
		{
			*outX = (uint32)m_positionX;
		}

		if (outY)
		{
			*outY = (uint32)m_positionY;
		}
	}

	void Text::GetPosition(float* outX, float* outY) const
	{
		if (outX)
		{
			*outX = m_positionX;
		}

		if (outY)
		{
			*outY = m_positionY;
		}
	}

	void Text::GetColor( unsigned char* outR, unsigned char* outG, unsigned char* outB, unsigned char* outA ) const
	{
		if( outR )
		{
			*outR = m_color.r8();
		}

		if( outG )
		{
			*outG = m_color.g8();
		}

		if( outB )
		{
			*outB = m_color.b8();
		}

		if( outA )
		{
			*outA = m_color.a8();
		}
	}

	unsigned int Text::GetAlign() const
	{
		return m_alignment;
	}

	float Text::GetWidthLimit() const
	{
		return m_widthLimit;
	}

	float Text::GetLineSpacing() const
	{
		return m_lineSpacing;
	}

	float Text::GetCharSpacing() const
	{
		return m_charSpacing;
	}

	void Text::GetScale( Vector2f &out ) const
	{
		out.Assign(m_scale.x, m_scale.y);
	}


	int Text::GetTabWidth() const
	{
		return m_tabWidth;
	}

	void Text::PrintDebugInfo() const
	{
		DEBUG_PRINT("--BEGIN TEXT DUMP--\nm_vtxBufCmdBuf: %u\nm_DrawStringBuffer: %u\nm_cachedString: %s\nm_cachedStringLength: %u\nm_positionX/m_positionY: %u/%u\nm_color (r/g/b/a): %u/%u/%u/%u\nm_alignment: %u\nm_widthLimit: %f\nm_lineSpacing: %f\nm_charSpacing: %f\nm_dirtyFlags: %u\n--END TEXT DUMP--\n",
			m_cachedString.CStr(),
			m_cachedString.Length(),
			m_positionX, m_positionY,
			m_color.r8(), m_color.g8(), m_color.b8(), m_color.a8(),
			m_alignment,
			m_widthLimit,
			m_lineSpacing,
			m_charSpacing,
			m_dirtyFlags);
	}
}

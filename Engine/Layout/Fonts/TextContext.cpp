/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Layout/Fonts/Text.h"
#include "Engine/Core/String/String_Util.h"
#include "TextContext.h"


namespace usg
{

	TextContext::TextContext()
		: m_fRubyScale(0.5f)
	{
		ResetWidthLimit();
		m_color.Assign(1.0f, 1.0f, 1.0f, 1.0f);
		m_vCursorPos.Assign(0.0f, 0.0f);
		m_vScale.Assign(1.0f, 1.0f);
		m_fRubyScale = 1.0f;
		m_uFlags = 0;
		m_uAlignFlags = kDefaultDrawFlag;
		m_fWidthLimit = 0.0f;
	}

	TextContext::~TextContext()
	{
	}

	void TextContext::ResetWidthLimit()
	{

	}

	void TextContext::Init(const Text* pInitData)
	{
		m_color = pInitData->GetColor();
		float fX, fY;
		pInitData->GetPosition(&fX, &fY);
		m_vCursorPos.Assign(fX, fY);
		pInitData->GetScale(m_vScale);
		m_uAlignFlags = pInitData->GetAlign();
		m_fWidthLimit = pInitData->GetWidthLimit();
	}

	bool TextContext::ProcessTag(uint8 code, const char* &szText)
	{
		switch (code)
		{
		case TAG_RUBY:
		{
			const float32 limitWidth = GetWidthLimit();
			ResetWidthLimit();

			if ((m_uFlags & CONTEXT_NO_CHAR_SPACE) == 0)
			{
				//MoveCursorX((float)GetCharSpace());
			}

			const uint16 rubyLen = *szText++;
			const uint16 targetLen = *szText++;
			const char* ruby = szText;
			const char* target = ruby + rubyLen;
			const uint32 drawFlag = GetAlignFlags();
			float32 x = GetCursorX();
			float32 y = GetCursorY();
			ASSERT(rubyLen > 0);
			return true;

		}

		case TAG_COLOR:
		{
			uint32 rgbaArr[4];
			szText = strchr(szText, '(');
			int numParams = str::ScanVariableArgsC(szText, "(%u %u %u %u)", &rgbaArr[0], &rgbaArr[1], &rgbaArr[2], &rgbaArr[3]);

			if (numParams < 4)
			{
				ASSERT(false && "Incorrect parameters!\n");
			}

			const Color rgba((uint8)rgbaArr[0], (uint8)rgbaArr[1], (uint8)rgbaArr[2], (uint8)rgbaArr[3]);
			// use the two-color version of this method to force the
			// override of a gradient if one is present
			SetTextColor(rgba);

			szText = strchr(szText, ')') + 1;

			return true;

		}
		case TAG_SCALE:
		{
			float32 scale = 0.f;
			szText = strchr(szText, '(');
			int numParams = str::ScanVariableArgsC(szText, "(%f)", &scale, 1);
			if (numParams < 1)
			{
				ASSERT(false && "Incorrect parameters!\n");
			}

			scale *= 0.01f;
			float32 width = GetScaleH();
			float32 height = GetScaleV();

			SetScale(width * scale, height * scale);

			szText = strchr(szText, ')') + 1;

			return true;
		}

		case TAG_BOLD:
		{
			const uint16 textLen = *szText++;
			const float32 weight = *szText / 32.0f;
			const char* text = szText + 1;
			const float32 orgX = GetCursorX();
			const float32 scaleH = GetScaleH();
			const uint32 drawFlag = GetAlignFlags();


			szText = text + textLen;

			return true;
		}

		case TAG_SHADOW:
		{
			return true;
		}

		default:
		{
			// No code here
		}
		}

		return false;
	}


	void TextContext::SetRubyScale(float rubyScale)
	{
		m_fRubyScale = rubyScale;
	}


}
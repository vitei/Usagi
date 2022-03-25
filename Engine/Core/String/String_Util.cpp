/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "String_Util.h"

namespace str
{
	char ToUpper(char src)
	{
		if(src >= 'a' && src <= 'z')
		{
			src -= ('a' - 'A');
		}
		return src;
	}

	char ToLower(char src)
	{
		if(src >= 'A' && src <= 'Z')
		{
			src += ('a' - 'A');
		}
		return src;
	}

	void ParseVariableArgsC(char* pszDest, uint32 uMaxLen, const char* szSrc, ...)
	{
		va_list va;
		va_start(va, szSrc);
		ParseVariableArgs(pszDest, uMaxLen, szSrc, va);
		va_end(va);
	}

	int ScanVariableArgsC(const char* pszSrc, const char* szFormat, ...)
	{
		va_list va;
		va_start(va, szFormat);
		int result = ReadVariableArgs(pszSrc, szFormat, va);
		va_end(va);
		return result;
	}


	void Copy(char* szDest, const char* szSrc, uint32 uLength)
	{
		while (*szSrc && uLength > 1)
		{
			*szDest++ = *szSrc++;
			uLength--;
		}
		*szDest = '\0';
	}

	bool Compare(const char* szSrc1, const char* szSrc2)
	{
		while (*szSrc1 && *szSrc2)
		{
			if (*szSrc1++ != *szSrc2++)
				return false;
		}
		return (*szSrc1 == *szSrc2);	// Should be equal to zero
	}

	bool CompareLen(const char* szSrc1, const char* szSrc2, uint32 uLen)
	{
		while (*szSrc1 && uLen)
		{
			if (*szSrc1++ != *szSrc2++)
				return false;

			uLen--;
		}
		return (uLen == 0);	// Should be equal to zero
	}

	uint32 StringLength(const char16* szSrc)
	{
		uint32 uLength = 0;
		while (*szSrc++)
		{
			uLength++;
		}
		return uLength;
	}

	uint32 StringLength(const char* szSrc)
	{
		uint32 uLength = 0;
		while (*szSrc++)
		{
			uLength++;
		}
		return uLength;
	}

	uint32 StringLengthSingleLine(const char* szSrc)
	{
		uint32 uLength = 0;
		while (*szSrc && *szSrc != '\r' && *szSrc != '\n')
		{
			uLength++;
			szSrc++;
		}
		return uLength;
	}

	bool StartsWithToken(const char* szCmp, const char* szToken)
	{
		uint32 uLength = StringLength(szToken);
		return CompareLen(szCmp, szToken, uLength);
	}

	bool EndsWithToken(const char* szCmp, const char* szToken)
	{
		uint32 uTokenLength = StringLength(szToken);
		uint32 uCmpLength = StringLength(szCmp);
		return CompareLen(&szCmp[uCmpLength - uTokenLength], szToken, uTokenLength);
	}

	void StringCat(char* szDst, const char* szAdd, uint32 uMaxLen)
	{
		uint32 i, j;
		uint32 uPos;
		for (i = 0; (szDst[i] != '\0' && i < uMaxLen - 1); i++)
			;

		uPos = i;
		for (j = 0; (szAdd[j] != '\0' && uPos < uMaxLen - 1); j++)
		{
			szDst[uPos] = szAdd[j];
			++uPos;
		}
		szDst[i + j] = '\0';
	}

	const char* FindLineNumber(const char* szSrc, uint32 uLine)
	{
		for (uint32 uCurr = 1; (*szSrc != '\0' && uCurr < uLine); uCurr++)
		{
			while (*szSrc != '\0' && *szSrc != '\n')
			{
				szSrc++;
			}
			if (*szSrc != '\0')
			{
				szSrc++;
			}
		}

		return szSrc;
	}

	const char* Find(const char* szSrc, const char* szCmp)
	{
		uint32 uCmpLen = StringLength(szCmp);
		uint32 uSrcLen = StringLength(szSrc);

		if (uCmpLen > uSrcLen)
			return NULL;

		for (uint32 i = 0; i <= uSrcLen - uCmpLen; i++)
		{
			if (CompareLen(&szSrc[i], szCmp, uCmpLen))
			{
				return &szSrc[i];
			}
		}
		return NULL;
	}

	usg::string ParseString(const char* szSrc, ...)
	{
		char tmp[1024];
		va_list va;
		va_start(va, szSrc);
		ParseVariableArgs(tmp, sizeof(tmp), szSrc, va);
		va_end(va);
		return tmp;
	}

	void TruncateToPath(usg::string& dest)
	{
		for (memsize i = (dest.length())-2; i > 0; i--)
		{
			if (dest.at(i) == '\\' || dest.at(i) == '/')
			{
				dest = dest.substr(0, i);
				break;
			}
		}
	}

	void TruncateExtension(usg::string& string)
	{
		for (memsize i = string.length()-1; i > 0; i--)
		{
			if (string.at(i) == '.')
			{
				string = string.substr(0, i);
				break;
			}
		}
	}

	void RemovePath(usg::string& string)
	{
		for (memsize i = (string.length()) - 2; i > 0; i--)
		{
			if (string.at(i) == '\\' || string.at(i) == '/')
			{
				string = string.substr(i + 1);
				break;
			}
		}
	}

	uint32 GetByteCount(const char* szText)
	{
		// Assume the first character is a starter
		szText++;
		uint32 uCount = 1;
		// Add characters where it isn't
		while ((*szText & 0x80) != 0 && (*szText & 0xc0) != 0xc0 && *szText != '\0')
		{
			szText++;
			uCount++;
		}
		return uCount;
	}

}

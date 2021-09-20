/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Utility functions for char* strings, should not use C string
//	functions in order to guarantee platform independence
*****************************************************************************/
#ifndef _USG_STRING_UTIL_H
#define _USG_STRING_UTIL_H
#include "Engine/Core/stl/string.h"

namespace str
{
	char ToUpper(char src);
	char ToLower(char src);
	void ParseVariableArgs(char* pszDest, uint32 uMaxLen, const char* szSrc, va_list& args );
	int ReadVariableArgs(const char* pszSrc, const char* szFormat, va_list& args);
	void ParseVariableArgsC(char* pszDest, uint32 uMaxLen, const char* szSrc, ...);
	int ScanVariableArgsC(const char* pszSrc, const char* szFormat, ...);
	void Copy(char* szDest, const char* szSrc, uint32 uLength);
	bool Compare(const char* szSrc1, const char* szSrc2);
	bool CompareLen(const char* szSrc1, const char* szSrc2, uint32 uLen);
	uint32 StringLength(const char16* szSrc);
	uint32 StringLength(const char* szSrc);
	uint32 StringLengthSingleLine(const char* szSrc);
	bool StartsWithToken(const char* szCmp, const char* szToken);
	bool EndsWithToken(const char* szCmp, const char* szToken);
	void StringCat(char* szDst, const char* szAdd, uint32 uMaxLen);
	const char* FindLineNumber(const char* szSrc, uint32 uLine);
	const char* Find(const char* szSrc, const char* szCmp);

	usg::string ParseString(const char* szSrc, ...);
	void TruncateToPath(usg::string &dest);
	void TruncateExtention(usg::string& string);
	void RemovePath(usg::string& string);
}


#endif

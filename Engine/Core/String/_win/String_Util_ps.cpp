/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Core/String/String_Util.h"

void str::ParseVariableArgs(char* pszDest, uint32 uMaxLen, const char* szSrc, va_list& args )
{
	vsnprintf_s( pszDest, uMaxLen, _TRUNCATE, szSrc, args );
}


int str::ReadVariableArgs(const char* pszSrc, const char* szFormat, va_list& args)
{
	return vsscanf_s(pszSrc, szFormat, args);
}
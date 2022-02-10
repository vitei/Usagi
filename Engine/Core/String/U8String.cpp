/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Maths/MathUtil.h"
#include "Engine/Core/String/U8String.h"
#include "Engine/Core/Thread/CriticalSection.h"
#include "Engine/Core/String/String_Util.h"
#include "Engine/Memory/ArrayPool.h"

namespace usg{


uint32 U8Char::GetByteCount(const char* szText)
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

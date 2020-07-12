/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Simple fixed sized string class
*****************************************************************************/

#ifndef __USG_STRING_FIXED_STRING_H__
#define __USG_STRING_FIXED_STRING_H__

#include "String_Util.h"

namespace usg
{

template <uint32 t_uSize>
class FixedString
{
public:
	FixedString(){}
	FixedString(const char* pzString){ Set(pzString); }
	FixedString(const FixedString<t_uSize>& str){ Set(str.m_string); }
	~FixedString(){}

	const char* CStr() const { return m_string; }
	void Set(const char* pzString) { str::Copy(&m_string[0], pzString, str::StringLength(pzString)); }
	bool Equals(const char* pzString) const { return str::Compare(m_string, pzString); }
	void Clear() { m_string[0] = NULL; }

protected:
	char m_string[t_uSize];
};

typedef FixedString<32> LabelString;
typedef FixedString<16> ShortLabelString;

}	//	namespace usg

#endif	//	__USG_STRING_FIXED_STRING_H__
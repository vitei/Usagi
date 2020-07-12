/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef STRING_CRC_H
#define STRING_CRC_H


#include "Engine/Core/Utility.h"
#include "U8String.h"

namespace usg{

// Calculates a string checksum for a given string type
class string_crc
{
public:
	string_crc() { checksum = (uint32)0; }

	string_crc(const char* str);
	string_crc(U8String& str);
	string_crc(uint32 crc) { checksum = crc; }
	
	// Retrieve the value
	uint32 Get() const { return checksum;  }

	// Clear the value
	void Clear() { checksum = 0; }

	string_crc& operator=(string_crc rhs)
	{
		checksum = rhs.checksum;
		return *this;
	}

	string_crc& operator=(const char* str)
	{
		checksum = utl::CRC32(str);
		return *this;
	}

	string_crc& operator=(U8String& str)
	{
		checksum = utl::CRC32(str.CStr());
		return *this;
	}
	string_crc& operator=(uint32 crc)
	{
		checksum = crc;
		return *this;
	}

	bool operator==(string_crc rhs)
	{
		return checksum == rhs.checksum;
	}
	bool operator!=(string_crc rhs)
	{
		return checksum != rhs.checksum;
	}

	bool operator==(const char* str)
	{
		return *this == string_crc(str);
	}
	bool operator!=(const char* str)
	{
		return *this != string_crc(str);
	}

private:
	uint32 checksum;
};

}

#endif

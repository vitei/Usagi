/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Utility.h"

// Based on the example implementation in RFC 1952 / PNG Annex D,
// which are the same sources Ruby based its implementation on.
// Refs: http://tools.ietf.org/html/rfc1952#section-8
//       http://www.w3.org/TR/2003/REC-PNG-20031110/#D-CRCAppendix
static uint32 _CRC32(const char* pData, size_t uSize, uint32 acc, bool bStopOnZero)
{
	const char* szString = pData;

	// Initialise table
	static const uint32 POLYNOMIAL = 0xedb88320;
	static uint32 crc_table[256];
	static bool crc_table_initialised = false;
	if (!crc_table_initialised)
	{
		for (uint32 i = 0; i < 256; ++i)
		{
			uint32 c = i;
			for (uint32 j = 0; j < 8; ++j)
			{
				if (c & 1) { c = POLYNOMIAL ^ (c >> 1); }
				else { c = c >> 1; }
			}
			crc_table[i] = c;
		}

		crc_table_initialised = true;
	}

	// Calculate CRC
	uint32 crc = ~acc;
	const char* current = szString;
	if (bStopOnZero)
	{
		for (; *current != '\0'; ++current)
		{
			crc = crc_table[(crc ^ *current) & 0xff] ^ (crc >> 8);
		}
	}
	else
	{
		for (uint32 i = 0; i < uSize; i++)
		{
			crc = crc_table[(crc ^ *current) & 0xff] ^ (crc >> 8);
			++current;
		}
	}
	return ~crc;
}

uint32 utl::CRC32(const char* szString, uint32 acc)
{
	return _CRC32(szString, (size_t)(-1), acc, true);
}

uint32 utl::CRC32(const void* pData, uint32 uSize, uint32 acc)
{
	return _CRC32((const char*)pData, (size_t)uSize, acc, false);
}
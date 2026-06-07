#pragma once

namespace utl
{
	uint32 CRC32(const char* szString, uint32 acc = 0);
	uint32 CRC32(const void* pData, uint32 uSize, uint32 acc = 0);
}

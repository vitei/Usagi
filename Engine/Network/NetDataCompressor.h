/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef NET_DATA_COMPRESSOR_H
#define NET_DATA_COMPRESSOR_H

#include "Engine/Common/Common.h"

namespace usg
{

struct NetPacket;

class NetDataCompressorUtil
{
public:
	static void CompressPacket(NetPacket* packet);
	static void CompressPacketBasic(NetPacket* packet);
	static void DecompressPacket(NetPacket* packet);
	static void DecompressPacketBasic(NetPacket* packet);
	static uint32 CompressToBufferRLE(const uint8* src, uint8* dst, uint32 len);
	static uint32 DecompressToBufferRLE(const uint8* src, uint8* dst, uint32 len, uint32 expectedLen, uint8 bitmask);
	
	static uint32 CompressToBufferRLE2(const uint8* src, uint8* dst, uint32 len);
	static uint32 DecompressToBufferRLE2(const uint8* src, uint8* dst, uint32 len, uint32 expectedLen);

	static uint32 CompressToBufferRLE3(const uint8* src, uint8* dst, uint32 len);
	static uint32 DecompressToBufferRLE3(const uint8* src, uint8* dst, uint32 len, uint32 expectedLen);


	static uint32 CompressToBufferHuffman(const uint8* src, uint8* dst, uint32 len);
	static uint32 DecompressToBufferHuffman(const uint8* src, uint8* dst, uint32 len);
	static uint32 CompressToBufferDouble(const uint8* src, uint8* dst, uint32 len);
	static uint32 DecompressToBufferDouble(const uint8* src, uint8* dst, uint32 len);
	static void TestCompression(NetPacket* packet);
};

}

#endif

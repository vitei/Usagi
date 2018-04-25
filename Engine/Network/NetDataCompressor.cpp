/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "NetDataCompressor.h"
#include "NetPacket.h"

#include <string.h>

#define PACKET_DATA_LENGTH (NETWORK_PACKET_MAX_LENGTH - sizeof(NetPacketHeader))
#define PACKET_MAX_RUN_BITMASK (0x60)
static const uint8 PACKET_MAX_RUN_BITMASK_PLUS_ONE = PACKET_MAX_RUN_BITMASK + 1;

namespace usg
{

uint32 NetDataCompressorUtil::CompressToBufferRLE(const uint8* src, uint8* dst, uint32 len)
{
	uint8* origin = dst;
	uint8 curRun = PACKET_MAX_RUN_BITMASK - 1;
	uint8 curVal = *src;
	uint32 canWriteLength = len;

	while (len-- > 0)
	{
		if (*src == curVal && curRun < 127)
		{
			curRun++;
			src++;
		}
		else
		{
			// Write			
			if (curRun > PACKET_MAX_RUN_BITMASK_PLUS_ONE || (curVal & PACKET_MAX_RUN_BITMASK) == PACKET_MAX_RUN_BITMASK)
			{
				*dst++ = curRun;
				canWriteLength--;
			}
			*dst++ = curVal;
			canWriteLength--;

			curRun = PACKET_MAX_RUN_BITMASK_PLUS_ONE;
			curVal = *src++;
		}

		// Compression saved us nothing
		if (canWriteLength <= 2)
			return 0;
	}
	// Write			
	if ((curRun & PACKET_MAX_RUN_BITMASK) == PACKET_MAX_RUN_BITMASK)
	{
		*dst++ = curRun;
	}
	*dst++ = curVal;

	return (uint32)(dst - origin);
}

uint32 NetDataCompressorUtil::DecompressToBufferRLE(const uint8* src, uint8* dst, uint32 len, uint32 expectedLen, uint8 bitmask)
{
	uint8* origin = dst;
	const uint8* pData = src;
	uint32 remaining = len;
	uint8 run = 1;

	while (remaining-- > 0)
	{
		uint8 byte = *pData++;
		run = 1;
		if ((byte & bitmask) == bitmask)
		{
			run = (byte)-bitmask;
			byte = *pData++;
			remaining--;
		}

		while (run--)
		{
			*dst++ = byte;
		}
	}
	uint32 written = (uint32)(dst - origin);
	ASSERT(written == expectedLen);
	return written;
}

void NetDataCompressorUtil::CompressPacket(NetPacket* packet)
{
	packet->header.compression = 0;
	packet->header.origLength = packet->header.dataLength;

#if 0
	uint8 buffer2[PACKET_DATA_LENGTH] = { 0 };
	uint32 len2 = CompressToBufferRLE2(packet->data, buffer2, packet->header.dataLength);
	
	static int len2used = 0;

	if (len2 != 0)
	{
		len2used++;
		packet->header.compression = 2;
		memcpy(packet->data, buffer2, len2);
		packet->header.dataLength = len2;
	}
	//if (len > 0)
	//{
	//	ASSERT(len < (uint32)packet->header.origLength);
	//	
	//	uint8 decCheck[PACKET_DATA_LENGTH] = { 0 };
	//	DecompressToBufferRLE3(buffer, decCheck, len, packet->header.dataLength);
	//	for (sint16 i = 0; i < packet->header.dataLength; i++)
	//	{
	//		ASSERT(decCheck[i] == packet->data[i]);
	//	}
	//
	//	// Write to buffer
	//	packet->header.compression = 2;
	//	memcpy(packet->data, buffer, len);
	//	packet->header.dataLength = len;
	//}
#endif
}

void NetDataCompressorUtil::DecompressPacket(NetPacket* packet)
{
	if (packet->header.compression == 0)
		return;
	
	uint8 compressed[PACKET_DATA_LENGTH];

	// Copy the compressed data into the buffer
	memcpy(compressed, packet->data, packet->header.dataLength);
	if (packet->header.compression == 2)
		packet->header.dataLength = DecompressToBufferRLE2(compressed, packet->data, packet->header.dataLength, packet->header.origLength);
	if (packet->header.compression == 3)
		packet->header.dataLength = DecompressToBufferRLE3(compressed, packet->data, packet->header.dataLength, packet->header.origLength);

	packet->header.compression = 0;
}

uint32 NetDataCompressorUtil::CompressToBufferRLE2(const uint8* src, uint8* dst, uint32 len)
{
	uint8* origin = dst;
	uint8 curRun = 0;
	uint16 prevVal = 0x100;
	uint16 curVal = *src;
	uint32 canWriteLength = len;

	while (len-- > 0)
	{
		curVal = *src++;
		if (curVal != prevVal)
		{
			// Print out the previous run
			if (curRun > 0)
			{
				*dst++ = (uint8)prevVal;
				*dst++ = curRun - 1;
				canWriteLength -= 2;
				curRun = 0;
			}

			*dst++ = (uint8)curVal;
			canWriteLength--;
			prevVal = curVal;

			// Compression wasn't good enough
			if (canWriteLength <= 2)
			{
				return 0;
			}
		}
		else
		{
			curRun++;
			if (curRun > 254)
				prevVal = 0x100;
		}
	}
	if (curRun > 0)
	{
		*dst++ = (uint8)prevVal;
		*dst++ = curRun - 1;
	}
	return (uint32)(dst - origin);
}
uint32 NetDataCompressorUtil::DecompressToBufferRLE2(const uint8* src, uint8* dst, uint32 len, uint32 expectedLen)
{
	uint8* origin = dst;
	const uint8* pData = src;
	uint32 remaining = len;
	uint8 run = 0;
	uint8 curVal = 0;
	uint16 prevChar = 0x100;


	while (remaining-- > 0)
	{
		curVal = *src++;
		if (curVal == prevChar)
		{
			*dst++ = curVal;
			run = *src++;
			remaining--;
			while (run--)
			{
				*dst++ = curVal;
			}
			prevChar = 0x100;
		}
		else
		{
			*dst++ = curVal;
			prevChar = curVal;
		}
	}
	uint32 written = (uint32)(dst - origin);
	ASSERT(written == expectedLen);
	return written;
}


uint32 NetDataCompressorUtil::CompressToBufferRLE3(const uint8* src, uint8* dst, uint32 len)
{
	uint8* origin = dst;
	uint8 curRun = 0;
	uint8 repRun = 0;

	uint16 prevVal = 0x100;
	uint16 prevPrev = 0x100;
	uint32 origLen = len;

	uint16 curVal = *src;

	const uint8* curRunStart = src;

	while (len--)
	{
		curVal = *src++;
		if (curVal == prevVal && prevVal == prevPrev)
		{
			if (curRun > 2)
			{
				*dst++ = (uint8)(2 - curRun);

				while (curRun-- > 2)
				{
					*dst++ = *curRunStart++;
				}
				curRun = 0;
			}
			repRun++;
			if (repRun == 127)
			{
				repRun = 0;
				*dst++ = 127;
				*dst++ = *curRunStart;

				len += 2;
				src -= 2;
				curRunStart = src;
				prevVal = 0x100;
				continue;
			}
		}
		else
		{
			if (repRun != 0)
			{
				repRun = 0;
				*dst++ = (uint8)(src - curRunStart) - 1;
				*dst++ = *curRunStart;

				curRunStart = src-1;
			}
			curRun++;
			if (curRun == 127)
			{
				*dst++ = (uint8)(0 - curRun);
				while (curRun--)
				{
					*dst++ = *curRunStart++;
				}
				curRun = 0;
				curVal = 0x100;
			}
		}

		prevPrev = prevVal;
		prevVal = curVal;
	}
	// End of the run
	if (curRun)
	{
		*dst++ = (uint8)(0 - curRun);
		while (curRun--)
		{
			*dst++ = *curRunStart++;
		}
	}
	else if (repRun)
	{
		*dst++ = (uint8)(src - curRunStart);
		*dst++ = *curRunStart;
	}

	
	// Compression didn't help at all
	len = (uint32)(dst - origin);
	if (len > origLen)
		return 0;

	return len;
}
uint32 NetDataCompressorUtil::DecompressToBufferRLE3(const uint8* src, uint8* dst, uint32 len, uint32 expectedLen)
{
	uint8* origin = dst;
	const uint8* pData = src;
	uint8 run = 0;
	uint8 curVal = 0;

	while ((sint32)len-- > 0)
	{
		// Get out the character
		uint8 runType = *src++;
		if (runType > 127)
		{
			uint8 runLen = 0 - runType;
			while (runLen--)
			{
				*dst++ = *src++;
				len--;
			}
		}
		else
		{
			while (runType--)
			{
				*dst++ = *src;
			}
			src++;
			len--;
		}
	}



	uint32 written = (uint32)(dst - origin);
	ASSERT(written == expectedLen);
	return written;
}


void NetDataCompressorUtil::TestCompression(NetPacket* packet)
{


	uint8 data[PACKET_DATA_LENGTH] = { 0 };
	uint8 dataUn[PACKET_DATA_LENGTH] = { 0 };
	uint32 compLen = CompressToBufferRLE3(packet->data, data, packet->header.dataLength);
	DecompressToBufferRLE3(data, dataUn, compLen, packet->header.dataLength);
}

/*
struct dicEntry
{
	uint8 data[16];
	uint8 len;
};
uint16 dicEntries = 0;
dicEntry dictionary[1024];
bool doesDicContain(uint8* str, uint8 val, uint8 strLen)
{
	for (uint16 d = 0; d < dicEntries; d++)
	{
		if (dictionary[d].len != strLen + 1)
		{
			continue;
		}

		bool match = true;
		for (uint8 len = 0; len < strLen; len++)
		{
			if (dictionary[d].data[len] != str[len])
			{
				match = false;
				break;
			}
		}
		if (match)
		{
			if (dictionary[d].data[strLen] == val)
				return true;
		}
	}
	return false;
}
uint16 getDicIndex(uint8* str, uint8 len)
{
	for (uint16 d = 0; d < dicEntries; d++)
	{
		if (dictionary[d].len != len)
			continue;
		bool match = true;
		for (uint8 l = 0; l < len; l++)
		{
			if (dictionary[d].data[l] != str[l])
			{
				match = false;
				break;
			}
		}
		if (match)
		{
			return d;
		}
	}
	return -1;
}
void addToDic(uint8* str, uint8 len)
{
	for (uint8 p = 0; p < len; p++)
	{
		dictionary[dicEntries].data[p] = str[p];
	}
	dictionary[dicEntries].len = len;
	dicEntries++;
}
uint32 NetDataCompressorUtil::CompressToBufferDouble(const uint8* src, uint8* dst, uint32 len)
{
	dicEntries = 0;
	uint8* dstOrigin = dst;
	uint8 compPool[1024];
	uint8 compWritePool[1024];
	uint8* compWriteStart = compWritePool;
	uint8* writeStart = compPool;
	uint8* writeOrig = writeStart;

	uint8 curString[16] = { 0 };
	uint8 tempString[16] = { 0 };
	uint8 curStringLen = 0;
	uint8 val;


	while (len--)
	{
		val = *src++;

		if (curStringLen != 15 && doesDicContain(curString, val, curStringLen))
		{
			curString[curStringLen++] = val;
		}
		else
		{
			// Get the encoding value
			if (curStringLen == 1)
			{
				*writeStart++ = 0;
				*writeStart++ = curString[0];
			}
			else
			{
				uint16 ind = 256 + getDicIndex(curString, curStringLen);
				*writeStart++ = (uint8)((ind) >> 8);
				*writeStart++ = (uint8)(ind & 0xff);				
			}
			curString[curStringLen] = val;
			addToDic(curString, curStringLen + 1);
			curStringLen = 1;
			curString[0] = val;
		}
	}
	// Get the encoding value
	if (curStringLen == 1)
	{
		*writeStart++ = 0;
		*writeStart++ = curString[0];
	}
	else
	{
		uint16 ind = 256 + getDicIndex(curString, curStringLen);
		*writeStart++ = (uint8)((ind) >> 8);
		*writeStart++ = (uint8)(ind & 0xff);
	}

	uint32 compLen = (writeStart - writeOrig);
	uint32 half = compLen / 2;
	// Mash the pieces together
	for (uint32 x = 0; x < half; x++)
	{
		compWriteStart[x] = writeOrig[x * 2];
		compWriteStart[x + half] = writeOrig[x * 2 + half];
	}
	*(uint16*)dst = (uint16)compLen;
	uint16 compressedLen = (uint16)CompressToBufferRLE(compWriteStart, dst + 4, compLen);
	*(uint16*)(dst + 2) = compressedLen;

	dst += (compressedLen + 4);

	*(uint16*)dst = dicEntries;
	dst += 2;
	uint32 compSize = dicEntries;
	for (uint16 ent = 0; ent < dicEntries; ent++)
	{
		compWriteStart[ent] = dictionary[ent].len;
	}
	compWriteStart += compSize;
	for (uint16 str = 0; str < dicEntries; str++)
	{
		for (uint16 l = 0; l < dictionary[str].len; l++)
		{
			*compWriteStart++ = dictionary[str].data[l];
		}
	}
	compSize = compWriteStart - compWritePool;
	compressedLen = (uint16)CompressToBufferRLE(compWritePool, dst + 2, compSize);
	*(uint16*)dst = compressedLen;
	dst += (2 + compressedLen);


	return dstOrigin - dst;
}
uint32 NetDataCompressorUtil::DecompressToBufferDouble(const uint8* src, uint8* dst, uint32 len)
{
	// Get out the pieces
	uint16 uncompressedEncodedDataLen = *(uint16*)src;
	uint16 compressedEncodedDataLen = *(uint16*)(src + 2);
	uint8 uncompressedEncoding[1024];
	dicEntries = 0;

	uint32 unEncLen = DecompressToBufferRLE(src + 4, uncompressedEncoding, compressedEncodedDataLen, uncompressedEncodedDataLen, 0xC0);


	return 0;
}









struct treeNode
{
	uint8 type;
	uint8 letter;
	sint16 hits;
	
	treeNode* left;
	treeNode* right;
	treeNode* parent;
};
#include <cstdlib>
int treeHitSort(const void* a, const void* b)
{
	treeNode* nodeA = (treeNode*)a;
	treeNode* nodeB = (treeNode*)b;

	return nodeA->hits - nodeB->hits;
}

void hookupChildren(treeNode* node)
{
	if (node->left)
	{
		node->left->parent = node;
		hookupChildren(node->left);
	}
	if (node->right)
	{
		node->right->parent = node;
		hookupChildren(node->right);
	}
}

uint32 NetDataCompressorUtil::CompressToBufferHuffman(const uint8* src, uint8* dst, uint32 len)
{
	treeNode hits[256] = { 0 };
	treeNode tree[512] = { 0 };
	uint32 nodesUsed = 256;
	uint32 lettersUsed = 0;
	uint32 treeNodesUsed = 256;
	uint32 dicOutLen = 0;
	uint8 dic[1024];
	uint8* dstDic = dic;
	
	for (uint32 i = 0; i < len; i++)
	{
		hits[src[i]].hits++;
	}
	for (uint32 j = 0; j < 256; j++)
	{
		if (hits[j].hits == 0)
		{
			continue;
		}
		nodesUsed--;
		lettersUsed++;
		hits[j].letter = (uint8)j;
		hits[j].type = 1;
	}
	qsort(hits, 256, sizeof(treeNode), treeHitSort);
	
	while (nodesUsed < 255)
	{
		// Construct the huffman tree
		treeNode* first = hits + nodesUsed;
		treeNode* second = first + 1;
		treeNode* firstChild = 0;
		treeNode* secondChild = 0;
		if (first->type == 1)
		{
			memcpy(tree + first->letter, first, sizeof(treeNode));
			firstChild = tree + first->letter;
		}
		else
		{
			memcpy(tree + treeNodesUsed, first, sizeof(treeNode));
			firstChild = tree + treeNodesUsed;
			treeNodesUsed++;
		}


		if (second->type == 0)
		{
			memcpy(tree + treeNodesUsed, second, sizeof(treeNode));
			secondChild = tree + treeNodesUsed;
			treeNodesUsed++;
		}
		else
		{
			memcpy(tree + second->letter, second, sizeof(treeNode));
			secondChild = tree + second->letter;
		}

		second->hits += first->hits;
		second->type = 0;
		first->hits = 0;
		first->letter = 0;
		first = second + 1;

		treeNode* nextCheck = second;

		// Sort down
		while (first != hits + 256 &&
			   second->hits > first->hits)
		{
			treeNode temp;
			memcpy(&temp, second, sizeof(treeNode));
			memcpy(second, first, sizeof(treeNode));
			memcpy(first, &temp, sizeof(treeNode));

			second++;
			first++;
		}

		second->left = firstChild;
		second->right = secondChild;
		nodesUsed++;
	}

	// Get out the huffman tree
	treeNode* huffmanTree = &hits[255];

	// Hook up the children
	hookupChildren(huffmanTree);
	
	uint16 encodings[256] = { 0 };
	uint8 lengths[256] = { 0 };
	uint8 lastM = 0;
	for (uint32 m = 0; m < 256; m++)
	{
		treeNode* node = &tree[m];
		
		if (node->type == 0)
			continue;
		ASSERT(node->parent->left == node ||
			node->parent->right == node);
		do
		{
			encodings[m] = encodings[m] >> 1;
			if (node->parent->right == node)
				encodings[m] += 0x8000;
			node = node->parent;
			lengths[m]++;
		} while (node->parent);

		*dstDic = (uint8)m - lastM;
		*(dstDic + lettersUsed) = lengths[m];
		*(dstDic + lettersUsed * 2) = (uint8)(encodings[m] >> 8);
		*(dstDic + lettersUsed * 3) = (uint8)(encodings[m] & 0xff);
		dstDic++;

		lastM = (uint8)m;
	}

	// Print in the dictionary
	uint32 compressedDicLength = CompressToBufferRLE(dic, dst + 4, lettersUsed * 4);
	*(uint32*)dst = compressedDicLength;

	uint8 putBit = 7;
	uint32 outLen = 0;
	const uint8* orig = src;
	dst += (compressedDicLength + 4);

	// Compress the data
	while (len--)
	{
		uint8 encLength = lengths[*src];
		uint32 encodingVal = encodings[*src];
		src++;

		while (encLength != 0)
		{
			if (encodingVal & 0x8000)
			{
				*dst |= (1 << putBit);
			}
			if (putBit == 0)
			{
				putBit = 8;
				*dst++;
				outLen++;
			}
			putBit--;
			encodingVal = encodingVal << 1;
			encLength--;
		}
	}

	return outLen + compressedDicLength + 4;

}
uint32 NetDataCompressorUtil::DecompressToBufferHuffman(const uint8* src, uint8* dst, uint32 len)
{
	// Create the dictionary
	uint16 encodings[256] = { 0 };
	uint8 lengths[256] = { 0 };
	uint8 lastM = 0;
	uint32 dicEntries = *((uint32*)dst);

	
	uint8 dicInflated[1024];
	uint8* dicStart = dicInflated;
	//DecompressToBufferRLE(src + 4, dicInflated, dicEntries, )

	//for (uint32 i = 0; i < dicEntries; i++)
	//{
	//	uint8 letter = *dicStart + lastM;
	//	lengths[letter] = *(dicStart + dicEntries);
	//	encodings[letter] = *(dicStart + dicEntries * 2);
	//	encodings[letter] = encodings[letter] << 8;
	//	encodings[letter] = *(dicStart + dicEntries * 3);
	//}

	return 0;
}

*/

}


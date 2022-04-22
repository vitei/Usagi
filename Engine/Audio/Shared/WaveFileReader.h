﻿/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Utility class for reading a wave file
*****************************************************************************/
#ifndef __USG_AUDIO_WAVEFILEREADER_H__
#define __USG_AUDIO_WAVEFILEREADER_H__

#include "Engine/Core/File/File.h"

class WaveFileReader
{
public:

	struct WaveFormat
	{
		uint16 fmtId;
		uint16 channel;
		uint32 sampleRate;
		uint32 transRate;
		uint16 blockSize;
		uint16 quantumBits;
	};

    WaveFileReader();
    WaveFileReader(const char* pPathName);
    ~WaveFileReader();
	void Initialize(const void* pData, memsize size);
    void Initialize(const char* pPathName);
    void Finalize();
    sint64 GetDataSize() const { return m_DataSize; }
    sint32 GetChannelCount() const { return m_format.channel; }
    sint32 GetSampleRate() const { return m_format.sampleRate; }
    sint32 GetQuantumBits() const { return m_format.quantumBits; }
	sint32 GetTransRate() const { return m_format.transRate; }
    sint64 ReadData(void* pData, sint64 size);
	const WaveFormat& GetWaveFormat() const { return m_format; }
	bool GetLooped() const { return m_bLooped; }
	uint32 GetLoopStart() const { return m_loopStart; }
	uint32 GetLoopLength() const { return m_loopLength; }
	
	void SeekPos(size_t pos) { m_pCurr = &m_pData[pos]; }
	void Read(size_t size, void* pDst) { usg::MemCpy(pDst, m_pCurr, size); m_pCurr += size; }
	void AdvanceBytes(size_t size) { m_pCurr += size; }
	size_t GetPos() { return (size_t)(m_pCurr - m_pData); }

private:

	const uint8*	m_pData;
	const uint8*	m_pCurr;
	size_t			m_rawDataSize;
	bool			m_bOwnsData;

	WaveFormat m_format;

	bool m_bLooped;
	uint32 m_loopStart;
	uint32 m_loopLength;

	sint64 m_DataSize;
	sint64 m_DataOffset;
	sint64 m_ReadPosition;
	sint32 m_SampleRate;
	sint32 m_TransRate;
private:
	void ReadInfo();
};

#endif  // __USG_AUDIO_WAVEFILEREADER_H__

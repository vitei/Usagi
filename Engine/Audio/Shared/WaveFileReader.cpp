/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Maths/MathUtil.h"
#include "WaveFileReader.h"

struct WaveRiff
{
	uint32 tag;
	uint32 size;
	uint32 type;
};

struct WaveChunk
{
	uint32 tag;
	uint32 size;
};


struct CuePoint
{
	uint32 ID;
	uint32 position;
	uint32 chunkId;
    uint32 chunkStart;
	uint32 blockStart;
	uint32 sampleStart;
};



struct RIFFMIDISample
{
	uint32        manufacturerId;
	uint32        productId;
	uint32        samplePeriod;
	uint32        unityNode;
	uint32        pitchFraction;
	uint32        SMPTEFormat;
	uint32        SMPTEOffset;
	uint32        loopCount;
	uint32        samplerData;
};

struct MIDILoop
{
	static const uint32 LOOP_TYPE_FORWARD = 0x00000000;
	static const uint32 LOOP_TYPE_ALTERNATING = 0x00000001;
	static const uint32 LOOP_TYPE_BACKWARD = 0x00000002;

	uint32 cuePointId;
	uint32 type;
	uint32 start;
	uint32 end;
	uint32 fraction;
	uint32 playCount;
};

inline uint32 GetTagValue(const char* pTag)
{
    return (pTag[0] <<  0) | (pTag[1] <<  8) |
           (pTag[2] << 16) | (pTag[3] << 24);
}


WaveFileReader::WaveFileReader() :
	m_pData(nullptr), m_pCurr(nullptr), m_bOwnsData(false),
	m_bLooped(false), m_loopStart(0), m_loopLength(0),
    m_DataSize(0), m_DataOffset(0), m_ReadPosition(0),
    m_SampleRate(0)
{
}

WaveFileReader::WaveFileReader(const char* pPathName) :
	m_pData(nullptr), m_pCurr(nullptr), m_bOwnsData(false),
	m_bLooped(false), m_loopStart(0), m_loopLength(0),
    m_DataSize(0), m_DataOffset(0), m_ReadPosition(0),
    m_SampleRate(0)
{
    Initialize(pPathName);
}

WaveFileReader::~WaveFileReader()
{
    Finalize();
}

void WaveFileReader::Initialize(const void* pData, memsize size)
{
	m_bOwnsData = false;
	m_rawDataSize = size;
	m_pData = (const uint8*)pData;
	m_pCurr = m_pData;
	ReadInfo();
}

void WaveFileReader::Initialize(const char* pPathName)
{
    if (m_DataSize == 0 && m_DataOffset == 0)
    {
		usg::File reader;
		reader.Open(pPathName);
		m_pData = (uint8*)usg::mem::Alloc(usg::MEMTYPE_STANDARD, usg::ALLOC_LOADING, reader.GetSize());
		reader.Read(reader.GetSize(), (void*)m_pData);
		m_rawDataSize = reader.GetSize();
		m_pCurr = m_pData;
		m_bOwnsData = true;
        ReadInfo();
    }
}
void WaveFileReader::Finalize()
{
    if (m_DataSize != 0 && m_DataOffset != 0)
    {
		if (m_bOwnsData)
		{
			usg::mem::Free((void*)m_pData);
			m_bOwnsData = false;
		}
		m_pData = nullptr;
        m_DataSize = 0;
        m_DataOffset = 0;
        m_SampleRate = 0;
        m_TransRate = 0;
        m_bLooped = false;
        m_loopStart = 0;
        m_loopLength = 0;
    }
}

void WaveFileReader::ReadInfo()
{
	memsize uReaderSize = m_rawDataSize;

    SeekPos(0);

    WaveRiff riff;
    WaveChunk chunk;

    // Check RIFF header
    Read(sizeof(riff), &riff);
    if (riff.tag != GetTagValue("RIFF"))
    {
		// Not riff format
        ASSERT(false);
    }
    if (riff.type != GetTagValue("WAVE"))
    {
		// Not wave format
        ASSERT(false);
    }

    // Find fmt and data
    bool isFormatChunkFound = false;
    bool isDataChunkFound = false;
    bool isEOF = false;
    while (!isEOF)
    {
        Read(sizeof(WaveChunk), &chunk);

        // "fmt"
        if (chunk.tag == GetTagValue("fmt "))
        {
            WaveFormat format;
            Read(sizeof(format), &format);
			m_format = format;

            // If it is an extended region, read and ignore.
            if (chunk.size > sizeof(format))
            {
                uint16 ext_size;
                Read(sizeof(ext_size), &ext_size);
                AdvanceBytes(ext_size);
            }
            isFormatChunkFound = true;
        }
        // "data"
        else if (chunk.tag == GetTagValue("data"))
        {
            m_DataSize = chunk.size;
            m_DataOffset = GetPos();
            isDataChunkFound = true;
            AdvanceBytes(chunk.size);
        }
		else if (chunk.tag == GetTagValue("smpl"))
		{
			RIFFMIDISample sample;
			Read(sizeof(RIFFMIDISample), (void*)&sample);
			memsize smplPos = GetPos();
			for (uint32 idx = 0; idx < sample.loopCount; idx++)
			{
				MIDILoop loop;
				Read(sizeof(MIDILoop), &loop);

				if (loop.type == MIDILoop::LOOP_TYPE_FORWARD)
				{
					m_bLooped = true;
					m_loopStart = loop.start;
					m_loopLength = loop.end - loop.start + 1;

					return;
				}
			}
			SeekPos(smplPos + chunk.size);

		}
        else if (chunk.tag == GetTagValue("cue "))
        {
            uint32 uCueCount;
            Read(sizeof(uCueCount), (void*)&uCueCount);

            m_bLooped = uCueCount >= 2;
            for (uint32 i = 0; i < uCueCount; i++)
            {
                CuePoint point;
                Read(sizeof(CuePoint), (void*)&point);

                if (i == 0)
                    m_loopStart = point.sampleStart;
                else if (i == 1)
                    m_loopLength = point.sampleStart - m_loopStart + 1;
            }

        }
        // Skip over everything else
        else
        {
            AdvanceBytes(chunk.size);
        }

        isEOF = GetPos() >= uReaderSize;
    }
}

sint64 WaveFileReader::ReadData(void* pData, sint64 size)
{
    ASSERT(pData!=NULL);
    size = usg::Math::Min(size, m_DataSize - m_ReadPosition);
    SeekPos((uint32)(m_DataOffset + m_ReadPosition));
    Read((uint32)size, pData);
    // If it is an 8-bit WAV file, it must be converted from unsigned to signed said the code originally
	// but our 8-bit samples are coming in unsigned (as they should be)
    /*if (GetQuantumBits() == 8)
    {
        sint8* p = reinterpret_cast<sint8*>(pData);
        for (int i = 0; i < size; i++)
        {
            p[i] -= 128;
        }
    }*/
    m_ReadPosition += size;
    return size;
}

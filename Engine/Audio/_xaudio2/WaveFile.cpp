/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Audio/Shared/WaveFileReader.h"
#include "Engine/Audio/SoundObject.h"
#include "WaveFile.h"

namespace usg
{

WaveFile::WaveFile()
{
	m_pData = NULL;
	m_uSize = 0;
}


WaveFile::~WaveFile()
{
	if(m_pData)
	{
		vdelete m_pData;
		m_pData = NULL;
	}
}

void WaveFile::ProcessWaveFile(WaveFileReader& reader, const SoundFileDef* pDef, const char* szName, Audio* pAudio)
{
	sint64 blockAlign = reader.GetChannelCount() * reader.GetDataSize() / 8;
	sint64 totalNumBytes = reader.GetDataSize();

	m_pData = vnew(ALLOC_AUDIO) BYTE[(size_t)totalNumBytes];

	const WaveFileReader::WaveFormat& format = reader.GetWaveFormat();

	m_format.wFormatTag = format.fmtId;
	m_format.nChannels = format.channel;
	m_format.nSamplesPerSec = format.sampleRate;
	m_format.nAvgBytesPerSec = format.transRate;
	m_format.nBlockAlign = format.blockSize;
	m_format.wBitsPerSample = format.quantumBits;
	m_format.cbSize = (WORD)reader.GetDataSize();

	m_loopLength = reader.GetLoopLength();
	m_loopStart = reader.GetLoopStart();

	sint64 uBytesRead = reader.ReadData(m_pData, reader.GetDataSize());
	if (uBytesRead != totalNumBytes)
	{
		vdelete(m_pData);
		m_pData = NULL;
		ASSERT(false);
	}
	m_uSize = (uint32)totalNumBytes;

	InitInt(pDef, szName, pAudio);
}


void WaveFile::InitRaw(const SoundFileDef* pDef, const void* pData, size_t rawDataSize, Audio* pAudio)
{
	WaveFileReader reader;
	reader.Initialize(pData, rawDataSize);

	ProcessWaveFile(reader, pDef, pDef->filename, pAudio);

}

void WaveFile::Init(const SoundFileDef* pDef, Audio* pAudio, const char* pszLocalizedSubdir)
{
	string name = "Audio/";
	if (pDef->localized && pszLocalizedSubdir != NULL)
	{
		name += pszLocalizedSubdir;
		name += "/";
	}
	name += pDef->filename;
	name += ".wav";

	WaveFileReader reader(name.c_str());

	ProcessWaveFile(reader, pDef, name.c_str(), pAudio);
}

void WaveFile::BindToSoundObject(class SoundObject* pSoundObject, bool bPositional)
{
	pSoundObject->GetPlatform().SetSoundFile(this, bPositional, m_bLooping);
	pSoundObject->SetSoundFile(this);
}

void WaveFile::BindToSound(class SoundObject_ps* pSoundObject, uint32 uPriority)
{
	pSoundObject->BindWaveFile(*this, uPriority);
}


}

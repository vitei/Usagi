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
		vdelete (char*)m_pData;
		m_pData = NULL;
	}
}

void WaveFile::Init(const SoundFileDef* pDef, Audio* pAudio, const char* pszLocalizedSubdir = NULL)
{
	U8String name = "Audio/";
	if (pDef->localized && pszLocalizedSubdir != NULL)
	{
		name += pszLocalizedSubdir;
		name += "/";
	}
	name += pDef->filename;
	name += ".wav";

	WaveFileReader reader(name.CStr());

	sint64 blockAlign = reader.GetChannelCount() * reader.GetDataSize() / 8;
	sint64 totalNumBytes = reader.GetDataSize();

	m_pData = vnew(ALLOC_AUDIO) char[(size_t)totalNumBytes];

	const WaveFileReader::WaveFormat& format = reader.GetWaveFormat();

	//m_format.wFormatTag = format.fmtId;
	m_uChannels = format.channel;
	m_uBitsPerSample = format.quantumBits;
	m_uSampleRate = format.sampleRate;
	
	/*
	m_format.nSamplesPerSec = format.sampleRate;
	m_format.nAvgBytesPerSec = format.transRate;
	m_format.nBlockAlign = format.blockSize;
	m_format.wBitsPerSample = format.quantumBits;
	m_format.cbSize = (WORD)reader.GetDataSize();
	*/
	
	//DEBUG_PRINT("init wav file %s %d bytes, %d channels, %d bps, %d hz\n",name.CStr(), totalNumBytes, m_uChannels, m_uBitsPerSample, m_uSampleRate);

	
	sint64 uBytesRead = reader.ReadData(m_pData, reader.GetDataSize() );
	if( uBytesRead != totalNumBytes )
	{
		vdelete( (char*)m_pData );
		m_pData = NULL;
		ASSERT(false);
	}
	m_uSize = (uint32)totalNumBytes;

}

void WaveFile::BindToSoundObject(class SoundObject* pSoundObject, bool bPositional)
{
	pSoundObject->GetPlatform().BindWaveFile(*this, bPositional, m_bLooping);
}


}

/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Audio/Audio.h"
#include "Engine/Audio/SoundCallbacks.h"
#include "AudioFilter_ps.h"
#include "AudioRoom_ps.h"
#include "AudioEffect_ps.h"
#include "SoundObject_ps.h"

namespace usg{

	static const uint32 g_suChannelMapping[][SOUND_CHANNEL_COUNT] =
	{
		{ SOUND_CHANNEL_FRONT_LEFT, SOUND_CHANNEL_FRONT_RIGHT },
		{ SOUND_CHANNEL_FRONT_LEFT, SOUND_CHANNEL_FRONT_RIGHT },
		{ SOUND_CHANNEL_FRONT_LEFT, SOUND_CHANNEL_FRONT_RIGHT, SOUND_CHANNEL_LOW_FREQ },
		{ SOUND_CHANNEL_FRONT_LEFT, SOUND_CHANNEL_FRONT_RIGHT, SOUND_CHANNEL_CENTER, SOUND_CHANNEL_LOW_FREQ, SOUND_CHANNEL_SIDE_LEFT, SOUND_CHANNEL_SIDE_RIGHT },
		{ SOUND_CHANNEL_FRONT_LEFT, SOUND_CHANNEL_FRONT_RIGHT, SOUND_CHANNEL_CENTER, SOUND_CHANNEL_LOW_FREQ, SOUND_CHANNEL_BACK_LEFT, SOUND_CHANNEL_BACK_RIGHT, SOUND_CHANNEL_SIDE_LEFT, SOUND_CHANNEL_SIDE_RIGHT },
	};


	class XAudioVoiceCallback : public IXAudio2VoiceCallback
	{
	public:
		XAudioVoiceCallback(usg::weak_ptr<SoundCallbacks> pIn) { pCallbackInt = pIn; }

		void STDMETHODCALLTYPE OnStreamEnd() { if (auto spt = pCallbackInt.lock()) spt->StreamEnd(); }
		void STDMETHODCALLTYPE OnVoiceProcessingPassEnd() { if (auto spt = pCallbackInt.lock()) spt->PassedEnd(); }
		void STDMETHODCALLTYPE OnVoiceProcessingPassStart(UINT32 samples) { }
		void STDMETHODCALLTYPE OnBufferEnd(void * context) { if (auto spt = pCallbackInt.lock()) spt->BufferEnd(); }
		void STDMETHODCALLTYPE OnBufferStart(void * context) { if (auto spt = pCallbackInt.lock()) spt->BufferStart(); }
		void STDMETHODCALLTYPE OnLoopEnd(void * context) { if (auto spt = pCallbackInt.lock()) spt->LoopEnd(); }
		void STDMETHODCALLTYPE OnVoiceError(void * context, HRESULT Error) {}

		usg::weak_ptr<SoundCallbacks> pCallbackInt;
	};

SoundObject_ps::SoundObject_ps()
{
	m_pSourceVoice = nullptr;
	m_pSoundFile = nullptr;
	m_uChannels = 0;
	m_bPositional = false;
	m_bValid = false;
	m_bPaused = false;
	m_bCustomData = false;
	m_pCallback = nullptr;
	m_defaultLowPass.Type = XAUDIO2_FILTER_TYPE::LowPassFilter; 
	m_defaultLowPass.Frequency = 1.0f;
	m_defaultLowPass.OneOverQ = 1.0f;
}

SoundObject_ps::~SoundObject_ps()
{
	Reset();
}

void SoundObject_ps::Init(Audio* pAudio)
{

}

void SoundObject_ps::Reset()
{
	if(m_pSourceVoice)
	{
		m_pSourceVoice->DestroyVoice();
		m_pSourceVoice = NULL;

		for (auto itr : m_pEffects)
		{
			itr->Release();
		}
		m_pEffects.clear();

		m_bValid = false;
		m_bPaused = false;
		m_bPositional = false;
	}
	if (m_pCallback)
	{
		vdelete m_pCallback;
		m_pCallback = nullptr;
	}
}

void SoundObject_ps::BindWaveFile(WaveFile &waveFile, uint32 uPriority)
{
	Audio_ps& audioPS = Audio::Inst()->GetPlatform();

	IXAudio2Voice* pOutputVoice = audioPS.GetSubmixVoice(waveFile.GetAudioType());
	if (m_pSoundFile && m_pSoundFile->GetRoom())
	{
		AudioRoom_ps* pRoomPS = (AudioRoom_ps*)m_pSoundFile->GetRoom();
		pOutputVoice = pRoomPS->GetVoice();
	}

	XAUDIO2_SEND_DESCRIPTOR SFXSend = { 0, pOutputVoice };
	XAUDIO2_VOICE_SENDS SFXSendList = { 1, &SFXSend };

	HRESULT result = audioPS.GetEngine()->CreateSourceVoice(&m_pSourceVoice, &waveFile.GetFormat(),
		0, XAUDIO2_DEFAULT_FREQ_RATIO, nullptr, &SFXSendList, NULL);
	if( FAILED( result ) )
	{
		ASSERT(false);
		return;
	}

	ZeroMemory(&m_buffer, sizeof(m_buffer));
	m_buffer.pAudioData = waveFile.GetData();
	m_buffer.Flags = XAUDIO2_END_OF_STREAM;  // tell the source voice not to expect any data after this buffer
	m_buffer.AudioBytes = waveFile.GetSize();
	m_uChannels = waveFile.GetFormat().nChannels;

	if(m_bLooping)
	{
		m_buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
		m_buffer.LoopBegin = waveFile.GetLoopStart();
		m_buffer.LoopLength = waveFile.GetLoopLength();
	}

	if (FAILED(m_pSourceVoice->SubmitSourceBuffer(&m_buffer)))
	{
		m_pSourceVoice->DestroyVoice();
		m_pSourceVoice = NULL;
		ASSERT(false);
		return;
	}

	else
	{
		m_defaultLowPass.Frequency = 1.0f;
		m_defaultLowPass.OneOverQ = 1.0f;
	}

	if (m_pSoundFile)
	{
		if (m_pSoundFile->GetFilter())
		{
			AudioFilter_ps* pFilterPS = (AudioFilter_ps*)m_pSoundFile->GetFilter();
			m_pSourceVoice->SetFilterParameters(&pFilterPS->GetParameters());
			if(pFilterPS->GetParameters().Type == m_defaultLowPass.Type)
			{
				m_defaultLowPass.Frequency = pFilterPS->GetParameters().Frequency;
				m_defaultLowPass.OneOverQ = pFilterPS->GetParameters().OneOverQ;
			}
		}

		usg::vector<XAUDIO2_EFFECT_DESCRIPTOR> descriptors;

		for (int i = 0; i < m_pSoundFile->GetEffectCount(); i++)
		{
			AudioEffect_ps* pEffect = (AudioEffect_ps*)m_pSoundFile->GetEffect(i);
			IUnknown* pXAPO = pEffect->CreateEffect();

			XAUDIO2_EFFECT_DESCRIPTOR descriptor;
			descriptor.InitialState = m_uChannels;
			descriptor.OutputChannels = 1;
			descriptor.pEffect = pXAPO;

			descriptors.push_back(descriptor);

			m_pEffects.push_back(pXAPO);
		}

		if (descriptors.size() > 0)
		{
			XAUDIO2_EFFECT_CHAIN chain;
			chain.EffectCount = (uint32)descriptors.size();
			chain.pEffectDescriptors = &descriptors[0];

			m_pSourceVoice->SetEffectChain(&chain);
		}

		for (int i = 0; i < m_pSoundFile->GetEffectCount(); i++)
		{
			AudioEffect_ps* pEffect = (AudioEffect_ps*)m_pSoundFile->GetEffect(i);
			m_pSourceVoice->SetEffectParameters(i, &pEffect->GetParams(), sizeof(pEffect->GetParams()));
		}

	}

	m_bCustomData = false;
	m_bValid = true;
}

void SoundObject_ps::SetCustomData(const StreamingSoundDef& def)
{
	Audio_ps& audioPS = Audio::Inst()->GetPlatform();
	// TODO: Pass in audio type
	XAUDIO2_SEND_DESCRIPTOR SFXSend = { 0, audioPS.GetSubmixVoice(AUDIO_TYPE_CUSTOM) };
	XAUDIO2_VOICE_SENDS SFXSendList = { 1, &SFXSend };

	WAVEFORMATEX format = { 0 };
	format.wFormatTag = WAVE_FORMAT_PCM;
	format.nChannels = def.uChannels;
	format.wBitsPerSample = def.uBitsPerSample;
	format.nSamplesPerSec = def.uSampleRate;
	format.nBlockAlign = format.wBitsPerSample / 8 * format.nChannels;
	format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
	m_uChannels = format.nChannels;
	if (def.pCallbacks.use_count() > 0)
	{
		m_pCallback = vnew(ALLOC_AUDIO) XAudioVoiceCallback(def.pCallbacks);
	}
	Audio::Inst()->GetPlatform().GetEngine()->CreateSourceVoice(&m_pSourceVoice, &format, 0, XAUDIO2_DEFAULT_FREQ_RATIO, m_pCallback, &SFXSendList);

	m_bValid = true;
	m_bCustomData = true;
	m_pSoundFile = nullptr;
}

void SoundObject_ps::SubmitData(void* pData, memsize size)
{
	if (m_bCustomData)
	{
		XAUDIO2_BUFFER buffer = { 0 };
		buffer.AudioBytes = (uint32)size;
		buffer.pAudioData = (const BYTE*)pData;
		m_pSourceVoice->SubmitSourceBuffer(&buffer);
	}
	else
	{
		// Trying to add data to a sound loaded from file
		ASSERT(false);
	}
}


void SoundObject_ps::SetSoundFile(WaveFile* pWaveFile, bool bPositional, bool bLoop)
{
	m_pSoundFile = pWaveFile;
	m_bPositional = bPositional;
	m_bLooping = bLoop;
}

void SoundObject_ps::Start()
{
	if (m_bValid)
	{	
		m_pSourceVoice->Start();
		m_bPaused = false;
	}
}

void SoundObject_ps::Stop()
{
	if (m_bValid)
	{
		m_pSourceVoice->Stop();
		m_pSourceVoice->FlushSourceBuffers();
		m_bPaused = false;
		m_bValid = false;

		// FIXME: Reset the internal data
		if (m_pCallback)
		{
			if (auto spt = m_pCallback->pCallbackInt.lock()) spt->Stopped();
		}
	}
}



void SoundObject_ps::Pause()
{
	if (m_bValid)
	{
		m_pSourceVoice->Stop();
		m_bPaused = true;
	}
}

void SoundObject_ps::Update(const SoundObject* pParent)
{
	if(pParent->GetRequestedPlayState() == PLAY_STATE_PLAYING && !m_bValid && m_pSoundFile != NULL)
	{
		m_pSoundFile->BindToSound(this, pParent->GetPriority());
	}

	if(!m_bValid)
		return;

	static float32 matrixCoefficients[SOUND_CHANNEL_COUNT * SOUND_CHANNEL_COUNT] = {};
	
	if(m_bPositional)
	{
		const usg::PanningData& panning = pParent->GetPanningData();
		const uint32 destChannels = Audio_ps::GetChannelCount(panning.eConfig);
		uint32 uIndex = 0;
		for (uint32 i = 0; i < destChannels; i++)
		{
			for (uint32 j = 0; j < m_uChannels; j++)
			{
				matrixCoefficients[uIndex] = panning.fMatrix[ g_suChannelMapping[panning.eConfig][i] ];
				uIndex++;
			}
		}
		m_pSourceVoice->SetOutputMatrix(NULL, m_uChannels, destChannels, matrixCoefficients);
	}
	
	float fVolume = pParent->GetAdjVolume() * pParent->GetAttenMul();

	XAUDIO2_FILTER_PARAMETERS lowPass = m_defaultLowPass;
	lowPass.Frequency *= pParent->GetLowPassFrequency();
	if (lowPass.Frequency < 1.0f)
	{
		m_pSourceVoice->SetFilterParameters(&lowPass);
	}

	//ASSERT(fVolume >= 0.0f && fVolume <= 1.0f);
	m_pSourceVoice->SetVolume(pParent->GetAdjVolume() * pParent->GetAttenMul());
	m_pSourceVoice->SetFrequencyRatio(pParent->GetPitch() * pParent->GetDopplerFactor());


	// TODO: Would need interal handling
	//m_pSourceVoice->SetPriority( pParent->GetPriority() );

	switch (pParent->GetRequestedPlayState())
	{
	case PLAY_STATE_PLAYING:
		Start();
		break;
	case PLAY_STATE_STOPPED:
		Stop();
		break;
	case PLAY_STATE_PAUSED:
		Pause();
		break;
	case PLAY_STATE_NONE:
	default:
		break;
	}
}


bool SoundObject_ps::IsPlaying() const
{
	if(!m_bValid)
		return false;

	XAUDIO2_VOICE_STATE voiceState;
	m_pSourceVoice->GetState( &voiceState );
	return voiceState.BuffersQueued > 0;
}

uint64 SoundObject_ps::GetSamplesPlayed() const
{
	if (!m_bValid)
		return 0;

	XAUDIO2_VOICE_STATE voiceState;
	m_pSourceVoice->GetState(&voiceState);
	return voiceState.SamplesPlayed;
}


}

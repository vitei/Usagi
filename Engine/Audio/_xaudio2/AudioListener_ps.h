/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Camera location info for a sound receiver
*****************************************************************************/
#ifndef __USG_AUDIO_XAUDIO2_AUDIO_LISTENER_PS_H__
#define __USG_AUDIO_XAUDIO2_AUDIO_LISTENER_PS_H__


namespace usg{

class Matrix4x4;

class AudioListener_ps
{
public:	
	AudioListener_ps() {}
	~AudioListener_ps() {}

	void Init() {}
	void SetRemoteOutput(bool bRemoteOutput) {}
	void SetMatrix(const Matrix4x4& mLoc) {}

private:
};

}

#endif

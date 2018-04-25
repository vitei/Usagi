/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Handle to an instance of a sound
*****************************************************************************/
#ifndef __USG_AUDIO_SOUNDHANDLE_PS_H__
#define __USG_AUDIO_SOUNDHANDLE_PS_H__
#include "Engine/Common/Common.h"

namespace usg{

	class SoundObject;

class SoundObject_ps
{
public:	
	SoundObject_ps() {}
	~SoundObject_ps() {}

	void Init() {}
	void Reset() {}
	void Start() {}
	void Stop() {}
	void Update(const SoundObject* pParent) {}
	bool IsPlaying() const { return false;  }

private:

};

}

#endif

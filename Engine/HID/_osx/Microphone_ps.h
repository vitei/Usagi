/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Platform specific implementation of the microphone
*****************************************************************************/
#ifndef __USG_HID_OSX_MICROPHONE_PS__
#define __USG_HID_OSX_MICROPHONE_PS__
#include "Engine/Common/Common.h"
#include "Engine/HID/InputDefines.h"

namespace usg{

class Microphone_ps
{
public:
    Microphone_ps() {}
    ~Microphone_ps(){}

    void Initialize() {}
    void Finalize() {}

    void Start(SampleType eType, SampleRate eRate) {}
    void End() {}

	void Update(){}
	float GetMaxVolumeThisFrame() { return 0.0f; }
	float GetAvgVolumeLong() { return 0.0f; }
	float GetAvgVolumeShort() { return 0.0f; }

	// Range 0-1
	void SetGain(float fGain) {}

private:

};

}

#endif

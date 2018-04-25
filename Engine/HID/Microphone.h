/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: The base class for the microphone
*****************************************************************************/
#ifndef __USG_HID_MICROPHONE__
#define __USG_HID_MICROPHONE__
#include "Engine/Common/Common.h"
#include "Engine/HID/InputDefines.h"
#include OS_HEADER(Engine/HID, Microphone_ps.h)

namespace usg{

class Microphone
{
public:
    Microphone(){}
    ~Microphone(){}

    void Initialize() { m_platform.Initialize(); }
    void Finalize() { m_platform.Finalize(); }

    void Start(SampleType eType = SAMPLING_TYPE_SIGNED_16BIT, SampleRate eRate = SAMPLING_RATE_8KHZ) { m_platform.Start(eType, eRate); }
    void End()  { m_platform.End(); }

	void Update() { m_platform.Update(); }
	float GetMaxVolumeThisFrame() const { return m_platform.GetMaxVolumeThisFrame(); }
	float GetAvgVolumeLong() const { return m_platform.GetAvgVolumeLong(); }
	float GetAvgVolumeShort() const { return m_platform.GetAvgVolumeShort(); }

private:
	Microphone_ps	m_platform;
};


}

#endif

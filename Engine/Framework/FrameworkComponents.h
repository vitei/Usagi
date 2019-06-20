/****************************************************************************
//	Filename: ActiveDevice.h
//	Description: Definition of runtime data for the ActiveDevice component
*****************************************************************************/

#ifndef _USG_FRAMEWORK_FRAMEWORKCOMPONENTS_H_
#define _USG_FRAMEWORK_FRAMEWORKCOMPONENTS_H_

#include "Engine/Framework/GameComponents.h"

namespace usg
{

class GFXDevice;
class TimeTracker;

template<>
struct RuntimeData<usg::Components::ActiveDevice>
{
	GFXDevice* pDevice;
};

template<>
struct RuntimeData<usg::Components::Identifier>
{
	uint32 uNameHash = 0;
};

template<>
struct RuntimeData<usg::Components::SystemTimeComponent>
{
	TimeTracker* pTimeTracker;
};

inline void ActiveDevice_init(Required<usg::Components::ActiveDevice>& c, GFXDevice* pDevice)
{
	ASSERT(pDevice != NULL);
	c.GetRuntimeData().pDevice = pDevice;
}

}

#endif

/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Queries the operating system for time changes
*****************************************************************************/
#ifndef _USG_TIME_TRACKER_H_
#define _USG_TIME_TRACKER_H_

#include "Engine/Common/Common.h"

namespace usg {

class TimeTracker
{
public:
	TimeTracker();
	~TimeTracker() {}

	//Return the time that has passed in seconds since the last time
	//this function was called
	float GetDeltaTime();

	//Get the total time passed since creation
	float GetTotalTime() const;
    
    static uint64 GetSystemTime();
    static uint64 GetTickFrequency();

private:
    

    uint64 m_ticks;
    uint64 m_ticksLastRead;
    
};

}

#endif

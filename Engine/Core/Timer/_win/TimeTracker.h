/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Queries the operating system for time changes
*****************************************************************************/
#ifndef _USG_TIME_TRACKER_H_
#define _USG_TIME_TRACKER_H_

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

namespace usg {

class TimeTracker
{
public:
	TimeTracker();
	~TimeTracker() {}

	//Return the time that has passed in seconds since the last time
	//this function was called
	float GetDeltaTime();

	//Return the time that has passed in seconds since the last time
	//GetDeltaTime was called - note this does not reset the counter
	float PeekDeltaTime() const;

	//Get the total time passed since creation
	float GetTotalTime() const;

	static uint64 GetSystemTime();
	static uint64 GetTickFrequency();

private:
	LARGE_INTEGER m_ticksPerSecond;
	LARGE_INTEGER m_lastTick;
	LARGE_INTEGER m_firstTick;
};

}

#endif
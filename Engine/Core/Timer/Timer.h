/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Handles time management, including support for fast and slow
//	motion.
*****************************************************************************/
#ifndef _USG_TIMER_H_
#define _USG_TIMER_H_


#include OS_HEADER(Engine/Core/Timer, TimeTracker.h)

namespace usg {

class Timer
{
public:
	Timer(void);
	~Timer(void);

	void SetGameSpeedChange( float multiplier );

	void EndSpeedChange();

	float GetGameSpeedMultiplier() const { return m_speedMultiplier; }

	float GetDeltaGameTime() const { return m_deltaGameTime; }

	float GetDeltaRealTime() const { return m_deltaRealTime; }

	float GetTotalTime() const { return m_timeTracker.GetTotalTime(); }

	float GetTotalGameModeTime() const;

	// Updates the game time and real time
	void Update( );

    void ResetGameModeTimer();
    
private:
	enum
	{
		GAME_TIME_SAMPLES = 6
	};

	TimeTracker		m_timeTracker;
	float			m_deltaGameTime;
	float			m_deltaRealTime;
	float			m_speedMultiplier;
	float			m_totalGameModeTime;	//	the total time since entering the current game mode
	float			m_fGameSamples[GAME_TIME_SAMPLES];
	uint32			m_uGameTimeId;
	bool			m_bGameTimeDistorted;
};

}


#endif

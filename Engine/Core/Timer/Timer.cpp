/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "engine/maths/mathutil.h"
#include "timer.h"

namespace usg {

const float kMaxDeltaTime = 1/20.0f;	

Timer::Timer()
{
	//Update();
	m_speedMultiplier = 1.0f;

	for(int i=0; i<GAME_TIME_SAMPLES; i++)
	{
		m_fGameSamples[i] = 1.0f/30.f;
	}
	m_uGameTimeId = 0;
}

Timer::~Timer()
{

}


void Timer::EndSpeedChange()
{
	m_speedMultiplier = 1.0f;
	m_bGameTimeDistorted = false;
}


void Timer::Update( )
{
	float fDeltaTime = m_timeTracker.GetDeltaTime();
	m_deltaRealTime = fDeltaTime;

	m_uGameTimeId++;
	if(m_uGameTimeId>=GAME_TIME_SAMPLES)
	{
		m_uGameTimeId = 0;
	}
		
	m_fGameSamples[m_uGameTimeId] = fDeltaTime;
	m_deltaGameTime = 0.0f;

	for(int i=0; i<GAME_TIME_SAMPLES; i++)
	{
		m_deltaGameTime += m_fGameSamples[i];
	}
	m_deltaGameTime /= (float)GAME_TIME_SAMPLES;

	m_deltaGameTime = Math::Clamp(m_deltaGameTime, 0.0f, kMaxDeltaTime);

	if( m_bGameTimeDistorted )
	{
		m_deltaGameTime *= m_speedMultiplier;
	}
}

float Timer::GetTotalGameModeTime() const
{
	return GetTotalTime() - m_totalGameModeTime;
}

void Timer::ResetGameModeTimer()
{
	m_totalGameModeTime = GetTotalTime();
}

}

/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef NET_TIME_H
#define NET_TIME_H

#include "Engine/Common/Common.h"
#include "Engine/Maths/MovingAverage.h"


namespace usg
{

#define NET_TIME_INPUT_DELAY (.10f) // Pretty close to the local multiplayer latency (measured using NetClient::GetPingTime())

class NetTime
{
public:
	NetTime();
	virtual ~NetTime();
	void Update(class NetClient* host, float deltaTime);

	static float GetServerTime();
	static double GetServerTimePrecise();

	void Reset();

	double GetAccuracy() const;

	static NetTime* Inst(){ return m_Instance; }

	void OnTimeResponse(struct NetMessageTimeReturn* timeReturn);
	void OnTimeRequest(struct NetMessageTimeRequest* request, class NetClient* client);

	void SetClockSynchronizationEnabled(bool bValue);

	// Stop trying to improve sync once this level of accuracy is reached. Default: 1.0, where standard deviation of the estimated differences to host is less than 25% of average delta time.
	void SetDesiredAccuracy(const float32 fAccuracy);
	float32 GetDesiredAccuracy();
protected:
	static NetTime* m_Instance;
	float m_nextTimeSync;
	float32 m_fDesiredAccuracy;
	bool m_bSyncEnabled;

private:
	MovingAverage<float, 8> m_averageDeltaTime;
	MovingAverage<double, 24> m_serverTimeDifference;
	double m_netStartTime;
};

}

#endif

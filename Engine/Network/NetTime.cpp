/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "NetPlatform.h"
#include "NetTime.h"
#include "NetMessage.h"
#include "NetClient.h"

#define NET_TIME_NUM_SYNCS (16)

#define NET_TIME_SYNC_WAIT_TIME_SECONDS (0.066f)

namespace usg
{
	static double s_fDiffToSessionTime = 0.0f;

	NetTime* NetTime::m_Instance = 0;

	NetTime::NetTime() :
		m_fDesiredAccuracy(1.0f),
		m_bSyncEnabled(false)
	{
		m_serverTimeDifference.Clear();
		ASSERT(m_Instance == NULL);
		m_Instance = this;
		Reset();
	}

	NetTime::~NetTime()
	{
		// Workaround (temporary fix):
		// This check is necessary since the constructor (unfortunately)
		// sets m_Instance to this
		if (m_Instance == this)
		{
			m_Instance = NULL;
		}
	}

	// WARNING: this function is VERY unprecise... time step is around 0.25 seconds
	float NetTime::GetServerTime()
	{
		return (float)GetServerTimePrecise();
	}

	// Measures how well the network time is synchronized (under the assumption that small variance in latency implies good accuracy)
	double NetTime::GetAccuracy() const
	{
		if (m_averageDeltaTime.GetDataSize() < NET_TIME_NUM_SYNCS || !m_bSyncEnabled)
		{
			return 0.0f;
		}
		const double fStdDev = sqrt(m_serverTimeDifference.Variance());
		const double fAvegDT = (double)m_averageDeltaTime.Get();
		return fStdDev <= 0 ? 1000.0f : fAvegDT / fStdDev * (1.0/4.0);
	}

	double NetTime::GetServerTimePrecise()
	{
		double currentTime = net_get_time();
		NetTime* netTime = NetTime::Inst();
		if(netTime != NULL) { currentTime += netTime->m_serverTimeDifference.Get(); }
		return currentTime;
	}

	void NetTime::Reset()
	{
		m_netStartTime = net_get_time();
		m_serverTimeDifference.Clear();
		m_nextTimeSync = 0;
	}

	void NetTime::Update(NetClient* host, float deltaTime)
	{
		m_averageDeltaTime.Add(deltaTime);
		if (!m_bSyncEnabled)
		{
			return;
		}
		// Do a time sync with the host
		if (host != nullptr && GetAccuracy() < m_fDesiredAccuracy)
		{
			m_nextTimeSync -= deltaTime;
			if (m_nextTimeSync <= 0)
			{
				m_nextTimeSync = NET_TIME_SYNC_WAIT_TIME_SECONDS;
				NetMessageTimeRequest request;
				request.currentTime = net_get_time();
				host->SendNetMessage(request, false);
			}
		}
	}

	float32 NetTime::GetDesiredAccuracy()
	{
		return m_fDesiredAccuracy;
	}

	void NetTime::SetDesiredAccuracy(const float32 fAccuracy)
	{
		m_fDesiredAccuracy = fAccuracy;
	}

	void NetTime::SetClockSynchronizationEnabled(bool bValue)
	{
		if (bValue == m_bSyncEnabled)
		{
			return;
		}
		Reset();
		m_bSyncEnabled = bValue;
	}

	// Sync request from clients
	void NetTime::OnTimeRequest(NetMessageTimeRequest* request, NetClient* client)
	{
		// Receive a time request from a client, resend it back after a time stamp
		NetMessageTimeReturn nmtr;
		nmtr.clientTime = request->currentTime;
		nmtr.serverTime = GetServerTimePrecise();
		client->SendNetMessage(nmtr, false);
	}

	// Sync response from server
	void NetTime::OnTimeResponse(NetMessageTimeReturn* timeReturn)
	{
		if (GetAccuracy() >= m_fDesiredAccuracy)
		{
			DEBUG_PRINT("Already synced... discard response..\n");
			return;
		}

		// Calculate server time offset
		const double curTime = net_get_time();

		// Half the time difference between send and received time
		const double halfLatency = (curTime - timeReturn->clientTime) / 2;

		// Calculate delta from my time and server time
		const double timeNowOnServer = timeReturn->serverTime + halfLatency;
		const double fDifference = timeNowOnServer - curTime;

		// Add difference to moving average
		DEBUG_PRINT("Estimated diff to host: %f\n", fDifference);
		m_serverTimeDifference.Add(fDifference);
	}

}


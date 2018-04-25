/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef MESSENGER_H
#define MESSENGER_H

#include "Engine/Framework/MessageDispatch.h"
#include "Engine/Network/NetManager.h"
#include "Engine/Network/NetClient.h"

namespace usg
{

class Messenger
{
public:
	Messenger(MessageDispatch& dispatch, NetManager* netManager = NULL);
	~Messenger();

	void SetNetManager(NetManager* netManager);

	template <typename T> void SendLocal(T& message);
	template <typename T> void SendToClient(long long uid, T& message, bool bReliable);
	template <typename T> void SendToAll(T& message, bool bReliable, bool bIncludeSelf = false);

private:
	MessageDispatch& m_messageDispatch;
	NetManager* m_netManager;
};

// Send a message locally (don't transmit it over the network)
template<typename T>
void Messenger::SendLocal(T& message)
{
	m_messageDispatch.FireMessage(message);
}

// Send a message to a specific client
template<typename T>
void Messenger::SendToClient(long long uid, T& message, bool bReliable)
{
	ASSERT(m_netManager != NULL);

	NetClient* pClient = NULL;
	for (int i = 0; i < USAGI_NET_MAX_CLIENTS; i++)
	{
		pClient = m_netManager->GetClient(i);
		ASSERT(pClient != NULL);

		if(pClient->GetIsActive() && (pClient->GetUID() == uid))
		{
			pClient->SendPBMessage(message, bReliable);
			return;
		}
	}
}

// Send a message to all connected clients (potentially including ourselves)
template<typename T>
void Messenger::SendToAll(T& message, bool bReliable, bool bIncludeSelf)
{
	if(m_netManager != NULL)
	{
		NetClient* pClient = NULL;
		for(int i = 0; i < USAGI_NET_MAX_CLIENTS; i++)
		{
			pClient = m_netManager->GetClient(i);
			ASSERT(pClient != NULL);

			if(pClient->GetIsActive())
			{
				pClient->SendPBMessage(message, bReliable);
			}
		}
	}

	if(bIncludeSelf)
	{
		SendLocal(message);
	}
}

}

#endif //MESSENGER_H


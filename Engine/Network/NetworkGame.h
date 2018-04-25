/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef USAGI_NETWORK_GAME_H
#define USAGI_NETWORK_GAME_H

#include "NetPlatform.h"
#include "Sock/SockNetworkManager.h"

namespace usg{

class NetworkGame
{
public: 
	virtual NetMessage* InitialiseMessage(NetMessage* msg);

	virtual void Update(float deltaTime)=0;
	virtual void OnAddNetworkPlayer(int playerIndex, NetClient* client, uint8 color)=0;
	virtual void OnRemovePlayer(int playerIndex)=0;
	virtual void OnDisconnectedFromHost() {}
	virtual void OnMessageReceived(usg::NetMessage* message, class usg::NetClient* client)=0;
	virtual void OnMessageReceived(usg::NetMessage* message)=0;

	virtual void OnConnectionEstablished(const bool bAsHost) {}
	virtual void OnHostChanged(const bool bNewHostIsMe) {}
#ifdef DEBUG_BUILD
	virtual void DrawDebug() {}
#endif
};

}

#endif

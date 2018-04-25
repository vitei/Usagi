/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef SOCK_NETWORK_GAME_H
#define SOCK_NETWORK_GAME_H

#include "Engine/Network/NetworkGame.h"

namespace usg {
class SockNetworkGame : public NetworkGame
{
protected:
	SockNetworkGame();
	virtual ~SockNetworkGame() {}

	bool AttemptConnection(float deltaTime);
	void UpdateConnected();
	bool CheckConnection();

private:
	enum NetworkState
	{
		LOOKING_FOR_GAME,
		TRYING_TO_CONNECT,
		CONNECTED
	};
	NetworkState m_state;
	float m_stateTimer;
	sint64 m_tryingToConnectID;

	bool UpdateLookingForGame(float deltaTime);
	bool UpdateTryingToConnect(float deltaTime);

	void CreateNewLobby();
};
}
#endif



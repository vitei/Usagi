/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef NET_GAME_CONNECTION_H
#define NET_GAME_CONNECTION_H


#include "Engine/Network/NetAvailableGames.h"

namespace usg
{

class NetGameConnection
{
public:
	NetGameConnection();
	virtual ~NetGameConnection() { }

	static NetGameConnection* Inst() { return Instance; }
	virtual void AttemptToJoinGame(struct NetAvailableGame* game) = 0;
	virtual void ConnectToGame(uint32 uGameId) = 0;
	virtual bool IsConnectedToGame() const { return false; }

	virtual void Update(float deltaTime) = 0;
	virtual void LateUpdate() {}
	virtual void HostGame() = 0;
	virtual	void LeaveGame() = 0;

	virtual struct NetAvailableGame* GetAvailableGame(uint32 idx) = 0;
	virtual uint32 GetNumAvailableGames() = 0;
	virtual bool IsInErrorState() = 0;
	virtual void ForceDisconnect() = 0;
	virtual bool IsDisconnected() = 0;
	const AvailableGameInfoContainer& GetAvailableGames() const;
protected:
	AvailableGameInfoContainer m_availableGames;
private:
	static NetGameConnection* Instance;
};

}

#endif

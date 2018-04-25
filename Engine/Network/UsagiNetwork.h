/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef USAGI_NETWORK_H
#define USAGI_NETWORK_H

#include "NetCommon.h"
#include "NetPlatform.h"
#include "Sock/SockNetworkManager.h"

namespace usg {

	class UsagiInetCore;
	class NetManager;
	class MessageDispatch;
	struct MessageDispatchShutdown;
	class NetworkGame;

	class UsagiNet
	{
	public: 

		UsagiNet(UsagiInetCore* pInetCore); 
		~UsagiNet();

		void SetUninitialized();

		void InitLocalHost(const InitLocalHostRequest& req);
		void InitLocalClient(const InitLocalClientRequest& req);
		void InitInetAutoMatch(const InitInetAutoMatchRequest& req);

		void CloseSession();
	

		void Update(float deltaTime);

		void SetContext(NetworkGame* game, MessageDispatch* dispatch);
		NetManager& GetNetworkManager();

		// Internal callbacks
		static void OnJoinGameUnavailable(){}
	private:

		SockNetworkManager m_networkManager;

		NetworkGame* m_networkGame;
		UsagiInetCore* m_pInetCore;

		void OnDispatchShutdown(const MessageDispatchShutdown& evt);
	};

}

#endif

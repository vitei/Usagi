/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Network/NetCommon.h"
#include "UsagiNetwork.h"
#include "NetMessage.h"
#include <Engine/HID/Input.h>
#include "Engine/Network/UsagiInetCore.h"
#include "Engine/Framework/MessageDispatch.h"
#include "Engine/Network/NetworkGame.h"

namespace usg
{

	UsagiNet::UsagiNet(UsagiInetCore* pInetCore) :
		m_networkManager(pInetCore),
		m_networkGame(NULL),
		m_pInetCore(pInetCore)
	{
	
	}

	UsagiNet::~UsagiNet()
	{

	}

	void UsagiNet::SetUninitialized()
	{
		m_networkManager.SetUninitialized();
	}

	void UsagiNet::InitInetAutoMatch(const InitInetAutoMatchRequest& req)
	{

	}

	void UsagiNet::InitLocalClient(const InitLocalClientRequest& req) {
		m_networkManager.InitLocalClient(req);
	}

	void UsagiNet::InitLocalHost(const InitLocalHostRequest& req) {
		m_networkManager.InitLocalHost(req);
	}

	void UsagiNet::CloseSession()
	{
		m_networkManager.CloseSession();
	}

	void UsagiNet::Update(float deltaTime)
	{
		m_networkManager.Update(deltaTime);
		if(m_networkGame != NULL)
		{
			m_networkGame->Update(deltaTime);
		}
	}

	void UsagiNet::SetContext(NetworkGame* game, MessageDispatch* dispatch)
	{
		//The game should have been cleared by OnDispatchShutdown by the time
		//we get here...
		ASSERT(m_networkGame == NULL);
		m_networkGame = game;
		m_networkManager.SetContext(game, dispatch);

		if(dispatch != NULL)
		{
			dispatch->RegisterCallback(this, &UsagiNet::OnDispatchShutdown);
		}
	}

	NetManager& UsagiNet::GetNetworkManager()
	{
		return m_networkManager;
	}

	void UsagiNet::OnDispatchShutdown(const MessageDispatchShutdown& evt)
	{
		m_networkGame = NULL;
	}

}

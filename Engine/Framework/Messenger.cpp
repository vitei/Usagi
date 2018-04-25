/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Messenger.h"

using namespace usg;

Messenger::Messenger(MessageDispatch& dispatch, NetManager* netManager)
	: m_messageDispatch(dispatch)
	, m_netManager(netManager)
{
}

Messenger::~Messenger()
{
}

void Messenger::SetNetManager(NetManager* netManager)
{
	m_netManager = netManager;
}

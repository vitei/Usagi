/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "NetCommon.h"
#include "NetGameConnection.h"

namespace usg
{

NetGameConnection* NetGameConnection::Instance = 0;

NetGameConnection::NetGameConnection()
{
	Instance = this;
}

const AvailableGameInfoContainer& NetGameConnection::GetAvailableGames() const
{
	return m_availableGames;
}
}


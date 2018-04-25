/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "NetworkGame.h"

namespace usg {

NetMessage* NetworkGame::InitialiseMessage(NetMessage* msg)
{
	return new (msg) NetMessage(0);
}

}

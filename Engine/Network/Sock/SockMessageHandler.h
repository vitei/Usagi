/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef SOCK_MESSAGE_HANDLER_H
#define SOCK_MESSAGE_HANDLER_H

#include <Engine/Network/NetMessageHandler.h>

namespace usg
{

class SockMessageHandler : public NetMessageHandler
{
public:
	SockMessageHandler() : NetMessageHandler() { }
	virtual ~SockMessageHandler() { }

	virtual bool IsPacketValid(struct NetPacket* inPacket);
	virtual void ReadSessionMessage(struct NetMessage* message, NetPacket* inPacket);
	virtual void SendPacketToClients(NetPacket* packet);

};

}

#endif

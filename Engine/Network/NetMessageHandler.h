/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef NET_MESSAGE_HANDLER_H
#define NET_MESSAGE_HANDLER_H

namespace usg
{

struct NetPacket;
struct NetMessage;

class NetMessageHandler
{
public:
	NetMessageHandler() { }
	~NetMessageHandler() { }

	void HandleIncomingPacket(NetPacket* inPacket);

protected:
	virtual bool IsPacketValid(NetPacket* inPacket) = 0;
	virtual void SendPacketToClients(NetPacket* packet) = 0;
	virtual void ReadSessionMessage(NetMessage* message, NetPacket* inPacket);

private:
	void ReadSessionPacket(NetPacket* inPacket);

};

}

#endif

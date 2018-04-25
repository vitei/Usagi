/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef NET_MULTI_LINK_H
#define NET_MULTI_LINK_H

namespace usg
{

// LAN Multicast transportation layer
class NetMultiLink
{
public:
	NetMultiLink(){}
	virtual ~NetMultiLink(){}

	virtual bool Initialize(){ return false; }
	virtual bool Shutdown(){ return false; }

	virtual bool Send(struct NetPacket*){ return false; }
	virtual bool Recv(struct NetPacket*){ return false; }
protected:
	bool m_isInitialized;
};

}

#endif


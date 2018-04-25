/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef NET_CLIENT_LINK_H
#define NET_CLIENT_LINK_H

#include "NetPlatform.h"

namespace usg
{

	// Transportation layer for Windows Client Link
	class NetClientLink
	{
	public:
		NetClientLink(){}
		virtual ~NetClientLink(){}

		virtual bool Initialize() = 0;
		virtual bool Shutdown() = 0;

		virtual bool Send(struct NetPacket* outPacket) = 0;
		virtual bool Recv(struct NetPacket* inPacket) = 0;

	protected:

		bool m_isInitialized;
	};

}

#endif

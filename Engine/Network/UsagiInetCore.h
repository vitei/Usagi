/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2017
//  This class is a facade for game server connection.
****************************************************************************/
#ifndef USAGI_INET_CORE_H
#define USAGI_INET_CORE_H


#include "Engine/Network/NetCommon.h"
#include <string>
#include <Engine/Memory/StdAllocator.h>

namespace usg
{

	class UsagiInetCore
	{
		friend struct Initializer;

		bool m_bACDisconnectionDetected;
		bool m_bConnectionEstablished;
		class Initializer;
		Initializer* m_pInitializers[7];

#ifndef FINAL_BUILD
		usg::basic_string<char > m_log;
#endif
	public:
		UsagiInetCore();
		~UsagiInetCore();

		void Update(float32 fDeltaTime);

		// Establish Internet connection and log in to Game Server. Returns true on success.
		bool Connect();

		// Log out from Game Server and disconnect from the Internet
		void Disconnect();

		// Test if connection is established
		bool IsConnected();

#ifndef FINAL_BUILD
		const char* GetLog();
		void Log(const char* szLog, ...);
#endif

	};

}

#endif

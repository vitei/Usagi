/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//  Please include this file in all network related translation units/headers
//  right after the Usagi engine common header.
****************************************************************************/
#ifndef NET_COMMON_H
#define NET_COMMON_H



#define ENABLE_INET_GAME


namespace usg
{
	struct InitLocalHostRequest
	{
		uint32 uAppDataSize;
		const void* pAppData;
		uint32 uMaxParticipants;
		bool bUseKeycode;
		uint32 uKeycode;
		uint32 uLocale;
		InitLocalHostRequest() :
			uAppDataSize(0),
			pAppData(NULL),
			uMaxParticipants(8),
			bUseKeycode(false),
			uKeycode(0),
			uLocale(0)
		{

		}
	};

	struct InitLocalClientRequest
	{
		bool bUseKeycode;
		uint32 uKeycode;
		uint32 uLocale;

		InitLocalClientRequest() :
			bUseKeycode(false),
			uKeycode(0),
			uLocale(0)
		{

		}
	};

	struct InitInetAutoMatchRequest
	{
		uint32 uLocale;
		uint32 uMaxParticipants;

		InitInetAutoMatchRequest() : uLocale(0), uMaxParticipants(8)
		{

		}
	};

	enum SessionOpenForParticipationStatus
	{
		SESSION_OPEN,
		SESSION_CLOSING,
		SESSION_CLOSED,
		SESSION_OPENING
	};
}

#endif

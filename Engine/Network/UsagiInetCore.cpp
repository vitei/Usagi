/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2017
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Network/NetCommon.h"
#include "Engine/Network/UsagiInetCore.h"
#include "Engine/Core/String/String_Util.h"

#define NEX_REQUIRED_MEMORY_SIZE (1 * 1024 * 1024)
#define NEX_EMERGENCY_MEMORY_SIZE ( 64 * 1024)

namespace usg
{
	class UsagiInetCore::Initializer
	{
	protected:
		bool m_bInitialized;
	public:
		Initializer()
		{
			m_bInitialized = false;
		}

		bool IsInitialized()
		{
			return m_bInitialized;
		}

		virtual bool Initialize(UsagiInetCore* pInetCore)
		{
			m_bInitialized = true;
			return true;
		}

		virtual void Finalize(UsagiInetCore* pInetCore)
		{
			m_bInitialized = false;
		}
	};

	UsagiInetCore::UsagiInetCore() :
		m_bACDisconnectionDetected(false),
		m_bConnectionEstablished(false)
	{

	}

	UsagiInetCore::~UsagiInetCore()
	{

	}

	void UsagiInetCore::Disconnect()
	{

	}

	void UsagiInetCore::Update(float32 fDeltaTime)
	{

	}

	bool UsagiInetCore::Connect()
	{

		return false;
	}

#ifndef FINAL_BUILD
	const char* UsagiInetCore::GetLog()
	{
		return m_log.c_str();
	}

	void UsagiInetCore::Log(const char* szLog, ...)
	{
		char szTemp[512];
		va_list vlist;
		va_start(vlist, szLog);
		str::ParseVariableArgs(szTemp, sizeof(szTemp), szLog, vlist);
		va_end(vlist);

		m_log += szTemp;
		m_log += "\n";

		DEBUG_PRINT("%s\n",szTemp);

		memsize uLineCount = std::count(m_log.begin(), m_log.end(), '\n');
		static const memsize MaxLines = 24;
		while (uLineCount > MaxLines)
		{
			const size_t it = m_log.find('\n');
			if (it != m_log.npos)
			{
				m_log = m_log.substr(it + 1);
				uLineCount--;
			}
			else
			{
				break;
			}
		}
	}
#endif

	bool UsagiInetCore::IsConnected()
	{
		return false;
	}

}

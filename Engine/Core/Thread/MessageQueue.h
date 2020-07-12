/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: A class for passing messages to a worker thread
****************************************************************************/
#ifndef _USG_ENGINE_THREAD_MESSAGE_QUEUE_H_
#define	_USG_ENGINE_THREAD_MESSAGE_QUEUE_H_

#include "Engine/Core/Thread/CriticalSection.h"

namespace usg
{

	template <class MessageType>
	class MessageQueue
	{
	public:

		MessageQueue()
		{
			m_pBuffer = NULL;
			m_uHead = 0;
			m_uTail = 0;
			m_uMessageCount = 0;
		}

		~MessageQueue()
		{
			if (m_pBuffer)
			{
				vdelete m_pBuffer;
			}
		}

		void Init(uint32 uMessageCount)
		{
			m_criticalSection.Initialize();
			m_pBuffer = vnew(ALLOC_OBJECT) MessageType;
			m_uMessageCount = uMessageCount;
		}


		bool TryEnqueue( MessageType& type )
		{
			CriticalSection::ScopedLock lock(m_criticalSection);
			if(CanQueueInt())
			{
				EnqueueInt(type);
				return true;
			}
			return false;
		}

		void Enqueue( MessageType& type )
		{
			// Force it to queue the message, wait until we can if there is no space
			while(true)
			{
				{
					CriticalSection::ScopedLock lock(m_criticalSection);
					if( CanQueueInt())
					{
						EnqueueInt(type);
						break;
					}
					Thread::Sleep(1);
				}
			}
		}

		bool TryDequeue( MessageType& type )
		{
			CriticalSection::ScopedLock lock(m_criticalSection);
			if(m_uHead == m_uTail)
			{
				return false;
			}
			type = m_pBuffer[m_uTail];
			m_uTail = (m_uTail + 1) % m_uMessageCount;
			return true;
		}

		bool CanQueue()
		{
			CriticalSection::ScopedLock lock(m_criticalSection);
			return ((m_uHead + 1)%m_uMessageCount)!=m_uTail;
		}

		bool IsEmpty()
		{
			CriticalSection::ScopedLock lock(m_criticalSection);
			return(m_uHead == m_uTail);
		}

	private:

		void EnqueueInt(MessageType& type)
		{
			m_pBuffer[m_uHead] = type;
			m_uHead = (m_uHead + 1) % m_uMessageCount;
		}


		bool CanQueueInt()
		{
			return ((m_uHead + 1)%m_uMessageCount)!=m_uTail;
		}

		CriticalSection 	m_criticalSection;
		MessageType*		m_pBuffer;
		uint32				m_uMessageCount;
		uint32				m_uHead;
		uint32				m_uTail;
	};

}

#endif

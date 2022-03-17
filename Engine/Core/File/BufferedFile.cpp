/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "BufferedFile.h"


namespace usg {
	static const memsize WRITE_BUFFER_SIZE = 2048; //2kb

	BufferedFile::BufferedFile(const char* szFileName, FILE_ACCESS_MODE eMode, FILE_TYPE eFileType)
	{
		m_pCurrPointer = NULL;
		m_pBasePointer = NULL;
		m_uSize = 0;
		Open(szFileName, eMode, eFileType);
	}

	BufferedFile::BufferedFile(void* pData, memsize uSize)
	{
		m_pCurrPointer = NULL;
		m_pBasePointer = NULL;
		m_uSize = 0;

		m_uSize = uSize;

		m_pCurrPointer = m_pBasePointer = (uint8*)pData;

	}

	BufferedFile::BufferedFile(void)
	{
		m_pCurrPointer = NULL;
		m_pBasePointer = NULL;
		m_uSize = 0;
	}


	bool BufferedFile::Open(const char* szFileName, FILE_ACCESS_MODE eMode, FILE_TYPE eFileType)
	{
		if(!Inherited::Open(szFileName, eMode, eFileType) )
		{
			return false;
		}

		m_uSize = Inherited::GetSize();

		if(eMode == FILE_ACCESS_WRITE)
		{
			m_scratch.Init(WRITE_BUFFER_SIZE, FILE_READ_ALIGN);
			m_pBasePointer = m_pCurrPointer = (uint8*)m_scratch.GetRawData();
		}
		else
		{
			m_scratch.Init((uint32)m_uSize, FILE_READ_ALIGN);
			m_pBasePointer = m_pCurrPointer = (uint8*)m_scratch.GetRawData();
			Inherited::Read(m_uSize, m_pCurrPointer);
		}

		if(eMode == FILE_ACCESS_READ)
		{
			// Don't close if we have writing available
			Inherited::Close();
		}

		return true;
	}

	void BufferedFile::Close()
	{
		if(GetAccessMode() == FILE_ACCESS_WRITE && m_pCurrPointer > m_pBasePointer)
		{
			Flush(); //TODO What if this returns false?
		}

		Inherited::Close();
		if(m_pBasePointer)
		{
			m_pCurrPointer = NULL;
			m_pBasePointer = NULL;
			m_scratch.Free();
		}
	}

	bool BufferedFile::Flush()
	{
		if(GetAccessMode() == FILE_ACCESS_WRITE && m_pCurrPointer > m_pBasePointer)
		{
			const memsize uBufUsed = m_pCurrPointer - m_pBasePointer;
			const memsize uBytesWritten = Inherited::Write((uint32)uBufUsed, m_pBasePointer);
			if(uBytesWritten != uBufUsed) { return false; }
			m_pCurrPointer = m_pBasePointer;
		}

		return true;
	}

	bool BufferedFile::IsOpen()
	{
		if(GetAccessMode() == FILE_ACCESS_WRITE)
		{
			return Inherited::IsOpen();
		}
		return (m_pCurrPointer != NULL);
	}

	memsize BufferedFile::Read(memsize uSize, void* pDst)
	{
		ASSERT(m_pCurrPointer!=NULL);
		ASSERT((m_pCurrPointer + uSize) <= (m_pBasePointer + m_uSize));
		MemCpy(pDst, m_pCurrPointer, uSize);
		m_pCurrPointer += uSize;

		return uSize;
	}

	memsize BufferedFile::Write(memsize uSize, const void* pSrc)
	{
		ASSERT(GetAccessMode() == FILE_ACCESS_WRITE && "Must have WRITE access to write files");

		const memsize uBufUsed = m_pCurrPointer - m_pBasePointer;
		ASSERT(uBufUsed <= WRITE_BUFFER_SIZE);

		const memsize uBufRemaining = WRITE_BUFFER_SIZE - uBufUsed;

		if(uBufRemaining > uSize)
		{
			MemCpy(m_pCurrPointer, pSrc, uSize);
			m_pCurrPointer += uSize;
		}
		else
		{
			if(uBufRemaining > 0)
			{
				MemCpy(m_pCurrPointer, pSrc, uBufRemaining);
				m_pCurrPointer += uBufRemaining;
			}

			ASSERT(m_pCurrPointer - m_pBasePointer == WRITE_BUFFER_SIZE);

			{
				const memsize uBytesWritten = Inherited::Write(WRITE_BUFFER_SIZE, m_pBasePointer);
				if(uBytesWritten != WRITE_BUFFER_SIZE) { DEBUG_PRINT("Write error; returning 0\n"); return 0; }
				m_pCurrPointer = m_pBasePointer;
			}

			const memsize uUnwrittenBytes = uSize - uBufRemaining;
			if(uUnwrittenBytes > 0)
			{
				uint8* const pSrcOffset = (uint8*)pSrc + uBufRemaining;
				if(uUnwrittenBytes < WRITE_BUFFER_SIZE)
				{
					MemCpy(m_pCurrPointer, pSrcOffset, uUnwrittenBytes);
					m_pCurrPointer += uUnwrittenBytes;
				}
				else
				{
					const memsize uBytesWritten = Inherited::Write(uUnwrittenBytes, pSrcOffset);
					if(uBytesWritten != uUnwrittenBytes) { DEBUG_PRINT("Write error; returning 0\n"); return 0; }
				}
			}
		}

		return uSize;
	}

	uint8 BufferedFile::ReadByte()
	{
		ASSERT(m_pCurrPointer!=NULL);
		uint8 uData = *m_pCurrPointer;
		m_pCurrPointer++;
		ASSERT((m_pCurrPointer) <= (m_pBasePointer + m_uSize));
		return uData;
	}

	void BufferedFile::SeekPos(memsize uPos)
	{
		ASSERT(GetAccessMode() != FILE_ACCESS_WRITE && "Can't SeekPos on buffered write files!");
		if(GetAccessMode() == FILE_ACCESS_WRITE)
		{
			Inherited::SeekPos(uPos);
		}
		else
		{
			ASSERT(m_pCurrPointer!=NULL);
			m_pCurrPointer = (m_pBasePointer+uPos);
		}
	}

	void BufferedFile::AdvanceBytes(memsize uPos)
	{
		ASSERT(GetAccessMode() != FILE_ACCESS_WRITE && "Can't AdvanceBytes on buffered write files!");
		if(GetAccessMode() == FILE_ACCESS_WRITE)
		{
			Inherited::AdvanceBytes(uPos);
		}
		else
		{
			ASSERT(m_pCurrPointer!=NULL);
			m_pCurrPointer += uPos;
			ASSERT((m_pCurrPointer) <= (m_pBasePointer + m_uSize));
		}
	}


	memsize BufferedFile::GetPos()
	{
		if(GetAccessMode() == FILE_ACCESS_WRITE)
		{
			return Inherited::GetPos();
		}
		else
		{
			ASSERT(m_pCurrPointer!=NULL);
			return (uint32)(m_pCurrPointer - m_pBasePointer);
		}
	}

	memsize BufferedFile::GetSize()
	{
		return m_uSize;
	}

}


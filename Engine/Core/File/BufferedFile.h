/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Pre-loads all of the file into scratch memory to improve
//	the speed of small file read
*****************************************************************************/
#pragma once

#ifndef _USG_BUFFERED_FILE_H_
#define _USG_BUFFERED_FILE_H_

#include "Engine/Memory/ScratchRaw.h"
#include "Engine/Memory/MemUtil.h"
#include "File.h"


namespace usg {

class BufferedFile : protected File
{
	typedef File Inherited;
public:
	BufferedFile(void);
	explicit BufferedFile(const char* szFileName, FILE_ACCESS_MODE eMode = FILE_ACCESS_READ, FILE_TYPE eFileType = FILE_TYPE_RESOURCE);
	~BufferedFile(void) { Close(); }

	virtual bool Open(const char* szFileName, FILE_ACCESS_MODE eMode = FILE_ACCESS_READ, FILE_TYPE eFileType = FILE_TYPE_RESOURCE);
	virtual void Close();
	virtual bool Flush();

	virtual bool IsOpen();
	virtual memsize Read(memsize uSize, void* pDst);
	virtual memsize Write(memsize uSize, const void* pSrc);
	virtual uint8 ReadByte();
	virtual void SeekPos(memsize uPos);
	virtual void AdvanceBytes(memsize uPos);
	virtual memsize GetPos();
	virtual memsize GetSize();

private:
	PRIVATIZE_COPY(BufferedFile)

	ScratchRaw	m_scratch;
	uint8*		m_pCurrPointer;
	uint8*		m_pBasePointer;
	memsize		m_uSize;
};



}

#endif

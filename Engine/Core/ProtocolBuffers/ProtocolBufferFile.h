/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#pragma once

#ifndef _USG_PROTOCOL_BUFFER_FILE_H__
#define _USG_PROTOCOL_BUFFER_FILE_H__

#include <pb.h>
#include <pb_decode.h>
#include <pb_encode.h>

#include "Engine/Common/Common.h"
#include "Engine/Core/File/BufferedFile.h"
#include "Engine/Core/ProtocolBuffers/ProtocolBufferFields.h"

#include OS_HEADER(Engine/Memory, Mem_ps.h)

#include "Engine/Resource/ResourceBase.h"

//#define PB_DEBUG
namespace usg {

//Helper function to generate a protocol buffer stream from a File handle
pb_istream_t pb_istream_from_file(File* f);
pb_ostream_t pb_ostream_from_file(File* f);

// The ProtocolBufferFile class provides a convenient way to read data from an
// ASCII NUL delimited list of protocol buffers.  You need to know the types of
// the data you want to read, so you can pass a pointer to them to the Read()
// function, but it will figure out the rest.
// This class derives off File to make use of its functionality, but because I
// am a bit scared about what would happen if you started fiddling about with
// the underlying File representation when m_stream thinks it has full control
// of the File handle I've elected to derive it privately.
class ProtocolBufferFile : protected BufferedFile, public ResourceBase
{
	typedef BufferedFile Inherited;
public:
	explicit ProtocolBufferFile(const char* szFileName, FILE_ACCESS_MODE eMode = FILE_ACCESS_READ, FILE_TYPE eFileType = FILE_TYPE_RESOURCE)
		: BufferedFile(szFileName, eMode, eFileType)
	{
		if(IsOpen())
		{
			m_istream = pb_istream_from_file(this);
			m_ostream = pb_ostream_from_file(this);
		}
	}
	virtual ~ProtocolBufferFile(void) {}

	void SetupHash(const char* szFileName)
	{
		ResourceBase::SetupHash(szFileName);
	}

	template<typename T>
	bool Read(T* pDst)
	{
		ProtocolBufferFields<T>::PreLoad(pDst);
		bool result = pb_decode(&m_istream, ProtocolBufferFields<T>::Spec(), pDst);
		ProtocolBufferFields<T>::PostLoad(pDst);
		if(!result)
		{
#if defined(DEBUG_BUILD) && defined(PB_DEBUG)
			DEBUG_PRINT("Read error: %s\n", m_istream.errmsg ? m_istream.errmsg : "(none)");
#endif
			m_istream.errmsg = NULL;
		}
		return result;
	}

	template<typename T>
	bool Write(T* pSrc)
	{
		ProtocolBufferFields<T>::PreSave(pSrc);
		bool result = pb_encode(&m_ostream, ProtocolBufferFields<T>::Spec(), pSrc);
		ProtocolBufferFields<T>::PostSave(pSrc);
		if(result) { uint8_t zero = 0; BufferedFile::Write(1, &zero); }
		else       { DEBUG_PRINT("Write error: %s\n", m_ostream.errmsg); }
		return result;
	}

	template<typename T>
	size_t ReadMany(size_t count, T* pDst)
	{
		T* dst = pDst;
		int i = 0;

		for(i = 0; i < count; i++, dst++)
		{
			if(!Read(pDst))
			{
				break;
			}
		}

		return i;
	}

	template<typename T>
	size_t WriteMany(size_t count, T* pSrc)
	{
		T* src = pSrc;
		int i = 0;

		for(i = 0; i < count; i++, src++)
		{
			if(!Write(pSrc))
			{
				break;
			}
		}

		return i;
	}

	virtual memsize GetPos();
	virtual memsize GetSize();
	virtual bool IsOpen();
	virtual void AdvanceBytes(memsize uPos);

	void Reset()
	{ 
		Inherited::SeekPos(0);
		m_istream = pb_istream_from_file(this);
		m_ostream = pb_ostream_from_file(this);
	}

	// Read raw data out of a file.  Useful for files containing Protocol Buffer headers
	// interspersed with raw data.  Note that bytes is an exact count, not a maximum!
	// Use (GetSize() - GetPos()) if you just want to read till the end of the file.
	bool ReadRaw(uint8* pBuf, size_t bytes);

	// Write raw data to a file.  You can use this for chunks of data, like textures, or
	// for simple delimiters, sizes, etc.
	bool WriteRaw(uint8* pBuf, size_t bytes);


private:
	pb_istream_t m_istream;
	pb_ostream_t m_ostream;
};

}

#endif //_USG_PROTOCOL_BUFFER_FILE_H__

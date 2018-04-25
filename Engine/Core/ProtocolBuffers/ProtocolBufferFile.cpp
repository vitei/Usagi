/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "ProtocolBufferFile.h"

namespace usg {

bool pb_istream_from_file_cb(pb_istream_t *stream, uint8_t *buf, size_t count)
{
	File* f = (File*)stream->state;
	size_t bytes_read = f->Read((uint32)count, buf);
	return bytes_read == count;
}

bool pb_ostream_from_file_cb(pb_ostream_t *stream, const uint8_t *buf, size_t count)
{
	File* f = (File*)stream->state;
	size_t bytes_written = f->Write((uint32)count, (void*)buf);
	return bytes_written == count;
}

pb_istream_t pb_istream_from_file(File* f)
{
	size_t bytes_left = f->GetSize() - f->GetPos();
	pb_istream_t out = { pb_istream_from_file_cb, f, bytes_left, NULL };
	return out;
}

pb_ostream_t pb_ostream_from_file(File* f)
{
	//size_t bytes_left = f->GetSize() - f->GetPos();
	pb_ostream_t out = { pb_ostream_from_file_cb, f, PB_SIZE_MAX, NULL };
	return out;
}

memsize ProtocolBufferFile::GetPos()
{
	return BufferedFile::GetPos();
}

memsize ProtocolBufferFile::GetSize()
{
	return BufferedFile::GetSize();
}

bool ProtocolBufferFile::IsOpen()
{
	return BufferedFile::IsOpen();
}

void ProtocolBufferFile::AdvanceBytes(memsize uPos)
{ 
	Inherited::AdvanceBytes(uPos);
	m_istream = pb_istream_from_file(this);
	m_ostream = pb_ostream_from_file(this);
}

bool ProtocolBufferFile::ReadRaw(uint8* pBuf, memsize bytes)
{
	return pb_read(&m_istream, pBuf, bytes);
}

bool ProtocolBufferFile::WriteRaw(uint8* pBuf, memsize bytes)
{
	return pb_write(&m_ostream, pBuf, bytes);
}


}

/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Standard char* strings, which have fast comparison and
//	dynamically resize
*****************************************************************************/
#ifndef _USG_STRING_H
#define _USG_STRING_H


const int STRING_CHUNK_SIZE = 32;
const int STRING_CMP_SIZE = STRING_CHUNK_SIZE/8;

namespace usg{

// TODO: Move me into my own clas
class U8Char
{
public:
	U8Char(size_t data) {
		m_data = data;
	}
	U8Char(const char* dataloc, size_t bytes)
	{
		this->m_data = 0;
		memcpy(&this->m_data, dataloc, bytes);
	}
	const char* GetBytes() const {
		return (const char*)m_data;
	};
	uint32 GetAsUInt32() const {
		return (uint32)m_data;
	}
	bool operator==(const U8Char &rhs) const;

	static uint32 GetByteCount(const char* szText);
private:
	size_t m_data;
};

inline bool U8Char::operator==(const U8Char &rhs) const
{
	return this->m_data == rhs.GetAsUInt32();
}


}

#endif

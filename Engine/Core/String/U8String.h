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


class U8String
{
public:
	U8String();
	U8String(const char* szInitText);
	U8String(const char16* szInitU16);
	U8String(const U8String &rhs);
	~U8String();

	const U8String& operator +=( const U8String &rhs );
	const U8String operator+( const U8String &rhs ) const;
	const U8String& operator=( const U8String &rhs );
	const U8String& operator=( const char* szText );
	bool operator==( const U8String &rhs ) const;
	bool operator!=( const U8String &rhs ) const;
	bool operator==( const char* szText ) const;

	const char* CStr() const { return m_pszString != NULL ? m_pszString : ""; }
	uint16 Length() const { return m_uStringLen; }
	uint16 CharCount() const;
	U8Char GetUTF8CharAtIdx(uint32 p_idx) const;
	bool HasExtension(const char* szExt);
	void ParseString(const char* szString, ...);
	void CopySingleLine( const char* szText );
	void CopyLength(const char* szText, uint32 uLength);
	void TruncateToPath();
	void TruncateExtension();
	const char* GetExtension() const;
	void RemovePath();
	void RemoveBasePath();
	void ToLower();

	static void InitPool();
	static void CleanupPool();

	struct StringChunk
	{
		union
		{
			char		szData[STRING_CHUNK_SIZE];
			uint64		uCmpData[STRING_CMP_SIZE];
		};
	};

private:
	
	union
	{
		char*			m_pszString;
		StringChunk*	m_pChunks;
	};

	void Init();
	void Resize(uint16 uBlockCount, bool copy=false);
	void CopyString( const char* szText );
	void CopyString( const char16* szText );
	uint32 GetBlockCount(uint32 uStringLen) const { return ((uStringLen+1)+(STRING_CHUNK_SIZE-1))/STRING_CHUNK_SIZE; };
	inline void ClearBlock( uint16 uBlock );
	inline void ChunkCopy( const StringChunk &src, StringChunk &dst );
	inline bool ChunkCmp( const StringChunk &src, const StringChunk &dst ) const;
	
	uint16	m_uBlockCount;
	uint16	m_uStringLen;
	
	mutable uint32	m_uHashValue;
	mutable bool	m_bHashDirty;

};


inline bool U8String::operator!=(const U8String &rhs) const
{
	return !(*this == rhs);
}



}

#endif

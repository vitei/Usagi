/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Maths/MathUtil.h"
#include "Engine/Core/String/U8String.h"
#include "Engine/Core/Thread/CriticalSection.h"
#include "Engine/Core/String/String_Util.h"
#include "Engine/Memory/ArrayPool.h"

namespace usg{

static ArrayPool<U8String::StringChunk>*	g_sChunkPool = NULL;
static CriticalSection s_criticalSection;

const int STRING_WORKSPACE_SIZE = 1024;
static char g_tempString[STRING_WORKSPACE_SIZE];

// Private functions inlined
inline void U8String::ClearBlock(uint16 uBlock)
{
	StringChunk* pChunk = &m_pChunks[uBlock];
	for (uint32 i = 0; i < STRING_CMP_SIZE; i++)
	{
		pChunk->uCmpData[i] = 0;
	}
}

inline void U8String::ChunkCopy(const StringChunk &src, StringChunk &dst)
{
	for (uint32 i = 0; i < STRING_CMP_SIZE; i++)
	{
		dst.uCmpData[i] = src.uCmpData[i];
	}
}

inline bool U8String::ChunkCmp(const StringChunk &src, const StringChunk &dst) const
{
	ASSERT(STRING_CMP_SIZE == 4);
	return(src.uCmpData[0] == dst.uCmpData[0]
		&& src.uCmpData[1] == dst.uCmpData[1]
		&& src.uCmpData[2] == dst.uCmpData[2]
		&& src.uCmpData[3] == dst.uCmpData[3]);
}


void U8String::InitPool()
{
	if(!g_sChunkPool)
	{
		g_sChunkPool = vnew(ALLOC_STRING) ArrayPool<StringChunk>(8192, true);
		s_criticalSection.Initialize();
	}

}

void U8String::CleanupPool()
{
	vdelete g_sChunkPool;
}

U8String::U8String()
{
	Init();
}


U8String::U8String(const char* szInitText)
{
	Init();
	if(szInitText)
		CopyString( szInitText );
}

U8String::U8String(const char16* szInitText16)
{
	Init();
	CopyString( szInitText16 );
}

const U8String& U8String::operator=( const char* szText )
{
	CopyString( szText );
	return *this;
}

void U8String::CopyString( const char* szText )
{
	uint32 uStringLen = str::StringLength(szText);
	Resize( GetBlockCount( uStringLen ) );
	m_uStringLen = (uint16)uStringLen;
	// Make sure that the last block is zeroed for fast comparison
	if(m_uBlockCount > 0)
	{
		ClearBlock( m_uBlockCount - 1 );
		str::Copy( m_pszString, szText, m_uBlockCount*STRING_CHUNK_SIZE + 1 );
	}
}

void U8String::CopyString( const char16* szText )
{
	uint32 uStringLen = str::StringLength(szText);
	Resize( GetBlockCount( uStringLen ) );
	m_uStringLen = (uint16)uStringLen;
	// Make sure that the last block is zeroed for fast comparison
	if(m_uBlockCount > 0)
	{
		ClearBlock( m_uBlockCount - 1 );	
		char* szDest = m_pszString;
		while(*szText)
		{
			*szDest++ = (char)*szText++;
		}
		*szDest = '\0';
	}
}

const U8String& U8String::operator=( const U8String &rhs )
{
	Resize( rhs.m_uBlockCount, false );
	for(int i=0; i<m_uBlockCount; i++)
	{
		ChunkCopy( rhs.m_pChunks[i], m_pChunks[i] );
	}
	m_uStringLen = rhs.m_uStringLen;

	return *this;
}


U8String::U8String(const U8String &rhs)
{
	Init();
	Resize( rhs.m_uBlockCount, false );
	for(int i=0; i<m_uBlockCount; i++)
	{
		ChunkCopy( rhs.m_pChunks[i], m_pChunks[i] );
	}
	m_uStringLen = rhs.m_uStringLen;
}

U8String::~U8String()
{
	if(m_uBlockCount)
	{
		{
			CriticalSection::ScopedLock lock(s_criticalSection);
			g_sChunkPool->FreeArray(m_pChunks, m_uBlockCount);
		}
		m_pChunks		= NULL;
		m_uBlockCount	= 0;
	}
}

void U8String::Init()
{
	m_pChunks		= NULL;
	m_uBlockCount	= 0;
	m_bHashDirty	= true;
	m_uStringLen	= 0;
}

void U8String::Resize(uint16 uBlockCount, bool copy)
{
	if(m_uBlockCount == uBlockCount)
		return;	// We don't bother resizing to save space, only to increase

	uint32 uCopyBlocks = Math::Min(uBlockCount, m_uBlockCount);

	StringChunk* pPrevChunk = m_pChunks;
	m_pChunks = NULL;

	if(uBlockCount > 0)
	{
		CriticalSection::ScopedLock lock(s_criticalSection);
		ASSERT(g_sChunkPool != NULL);
		m_pChunks = g_sChunkPool->AllocArray(uBlockCount);
	}

	if( copy && pPrevChunk )
	{
		uint32 i=0;
		for(i=0; i<uCopyBlocks; i++)
		{
			ChunkCopy( pPrevChunk[i], m_pChunks[i] );
		}
		for(;i<uBlockCount; i++)
		{
			ClearBlock(i);
		}
	}
	else
	{
		for(uint16 i=0; i<uBlockCount; i++)
		{
			ClearBlock(i);
		}
	}

	if (pPrevChunk && m_uBlockCount > 0)
	{
		CriticalSection::ScopedLock lock(s_criticalSection);
		g_sChunkPool->FreeArray(pPrevChunk, m_uBlockCount);
	}
	

	m_uBlockCount = uBlockCount;

}

// return the actual number of utf-8 characters in this string
uint16 U8String::CharCount() const
{
	uint16 uCount = 0;

	for (uint32 i = 0; i < m_uStringLen; ++i)
	{
		if ((m_pszString[i] & 0x80) == 0 || (m_pszString[i] & 0xc0) == 0xc0)
			++uCount;
	}

	return uCount;
}

U8Char U8String::GetUTF8CharAtIdx(uint32 p_idx) const
{
	//uint32 uChar = 0;

	void* pStart = NULL;

	uint16 uCount = 0;
	size_t countBytes = 0;
	for (uint32 i = 0; i < m_uStringLen; ++i)
	{
		if ((m_pszString[i] & 0x80) == 0 || (m_pszString[i] & 0xc0) == 0xc0)
		{
			if (uCount == p_idx)
			{
				pStart = &m_pszString[i];
				countBytes = 1;
			}
			else if (pStart != NULL)
			{
				break;
			}
			++uCount;
		}
		else {
			if (pStart != NULL)
				countBytes++;
		}
	}

	// there has to be a better way than this, surely.
	//memcpy((char*)&uChar, (char*)pStart, countBytes);
	U8Char returnChar((char*)pStart, countBytes);

	return returnChar;
}

bool U8String::HasExtension(const char* szExt)
{
	uint32 len = str::StringLength(szExt);
	if( len > m_uStringLen )
	{
		ASSERT(false);
		return false;
	}

	char* cmp = &m_pszString[m_uStringLen-len];
	while(len--)
	{
		if(str::ToUpper(*cmp) != str::ToUpper(*szExt))
		{
			return false;
		}
		cmp++;
		szExt++;
	}

	return true;
}

const char* U8String::GetExtension() const
{
	for( uint32 i=m_uStringLen; i>0; i--)
	{
		if(m_pszString[i] == '.')
		{
			return &m_pszString[i+1];
		}
	}
	return NULL;
}

void U8String::ParseString(const char* szString, ...)
{
	va_list va;
	va_start(va, szString);
	str::ParseVariableArgs(g_tempString, STRING_WORKSPACE_SIZE, szString, va);
	va_end(va);
	CopyString(g_tempString);
}


void U8String::CopyLength(const char* szText, uint32 uLength)
{
	uint32 uStringLen = str::StringLength(szText);
	if(uStringLen<uLength)
		uLength = uStringLen;

	Resize( GetBlockCount( uLength ) );
	m_uStringLen = (uint16)uLength;
	// Make sure that the last block is zeroed for fast comparison
	ClearBlock( m_uBlockCount - 1 );
	str::Copy( m_pszString, szText, uLength+1 );
}

void U8String::CopySingleLine( const char* szText )
{
	uint32 uStringLen = str::StringLengthSingleLine(szText);
	Resize( GetBlockCount( uStringLen ) );
	m_uStringLen = (uint16)uStringLen;
	// Make sure that the last block is zeroed for fast comparison
	ClearBlock( m_uBlockCount - 1 );
	str::Copy( m_pszString, szText, uStringLen+1 );
}

void U8String::TruncateToPath()
{
	for( sint32 i=(sint32)(m_uStringLen)-2; i>0; i--)
	{
		if(m_pszString[i] == '\\' || m_pszString[i] == '/' )
		{
			m_pszString[i+1] = '\0';
			break;
		}
	}
	
	uint32 uStringLen = str::StringLength(m_pszString);
	Resize( GetBlockCount( uStringLen ), true );
	m_uStringLen = (uint16)uStringLen;
}

void U8String::TruncateExtension()
{
	for( uint32 i=m_uStringLen; i>0; i--)
	{
		if(m_pszString[i] == '.')
		{
			m_pszString[i] = '\0';
			break;
		}
	}

	uint32 uStringLen = str::StringLength(m_pszString);
	Resize( GetBlockCount( uStringLen ), true );
	m_uStringLen = (uint16)uStringLen;
}

void U8String::RemovePath()
{
	U8String tmp;
	for( uint32 i=m_uStringLen; i>0; i--)
	{
		if(m_pszString[i] == '\\' || m_pszString[i] == '/' )
		{
			tmp = &m_pszString[i+1];
			break;
		}
	}

	CopyString(tmp.CStr());
}

void U8String::RemoveBasePath()
{
	U8String tmp;
	for( uint32 i=0; i<m_uStringLen; i++)
	{
		if(m_pszString[i] == '\\' || m_pszString[i] == '/' )
		{
			if(i<(uint32)(m_uStringLen-1))
			{
				tmp = &m_pszString[i+1];
			}
			else
			{
				tmp = &m_pszString[i];
			}
			break;
		}
	}

	CopyString(tmp.CStr());
}

bool U8String::operator==(const char* szText) const
{
	return str::Compare(szText, m_pszString);
}

void U8String::ToLower()
{
	if(!m_pszString)
		return;

	for(uint32 i=0; i<m_uStringLen; i++)
	{
		m_pszString[i] = str::ToLower(m_pszString[i]);
	}
}


const U8String& U8String::operator+=(const U8String& rhs)
{
	if (m_pszString)
	{
		if (rhs.m_uStringLen != 0)
		{
			uint32 uNewBlockCount = GetBlockCount(m_uStringLen + rhs.m_uStringLen);
			Resize(uNewBlockCount, true);
			str::StringCat(m_pszString, rhs.m_pszString, m_uBlockCount*STRING_CHUNK_SIZE);
			m_uStringLen += rhs.m_uStringLen;
		}
	}
	else
	{
		*this = rhs;
	}
	return *this;
}

bool U8String::operator==(const U8String &rhs) const
{
	if (rhs.Length() != m_uStringLen)
		return false;

	if (m_uStringLen == 0)
		return rhs.Length() == 0;

	uint32 uCmpBlocks = GetBlockCount(m_uStringLen);
	for (uint32 i = 0; i < uCmpBlocks; i++)
	{
		if (!ChunkCmp(m_pChunks[i], rhs.m_pChunks[i]))
			return false;
	}

	return true;
}

const U8String U8String::operator+(const U8String &rhs) const
{
	U8String ret = *this;
	ret += rhs;
	return ret;
}

uint32 U8Char::GetByteCount(const char* szText)
{
	// Assume the first character is a starter
	szText++;
	uint32 uCount = 1;
	// Add characters where it isn't
	while ((*szText & 0x80) != 0 && (*szText & 0xc0) != 0xc0 && *szText != '\0')
	{
		szText++;
		uCount++;
	}
	return uCount;
}


}

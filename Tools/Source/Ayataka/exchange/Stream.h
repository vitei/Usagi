#ifndef STREAM_H__
#define STREAM_H__

#include "OwnSTLDecl.h"

#include "Engine/Scene/Model/Shape.pb.h"

using namespace usg;

namespace exchange
{
typedef std::vector< uint32_t, aya::Allocator<uint32_t> > VectorIndexStream;

class Stream {
public:
	Stream( void ) {
		clear();
	}
	virtual ~Stream( void ) {
		Free();
	}

	template <typename T> void allocate( uint32_t length, uint32_t colNum ) {
		ASSERT_MSG( m_pStreamArray == NULL, "Stream is already allocated." );
		//mAttrType = attrType;
		m_uLength = length;
		m_uColumnNum = colNum;
		m_uElementSize = sizeof(T);
		m_uSize = sizeof(T)*length;
		m_pStreamArray = reinterpret_cast<char*>( malloc(m_uSize) );
		ASSERT_MSG(m_pStreamArray != NULL, "Allocation failed.");
	}
	void clear( void ) {
		m_pStreamArray = NULL;
		//mAttrType = VertexAttribute_NONE;
		m_uSize = 0;
		m_uLength = 0;
		m_uElementSize = 0;
		m_uColumnNum = 0;
	}
	void Free( void ) {
		//TODO: Fix this code properly.  The if statement below wasn't there
		//      originally, causing the code to crash when given an empty stream
		//      (such as one skipped because it's not supported).
		//      Regardless, calling "delete[]" on a void* is pretty dodgy anyway,
		//      so that might need some thinking through.
		if(m_pStreamArray != NULL && m_uSize > 0)
			::free(m_pStreamArray);
		clear();
	}

	void* GetStreamArrayPtr( void ) { return m_pStreamArray; }
	const void* GetStreamArrayPtr( void ) const { return m_pStreamArray; }

	//VertexAttribute getAttrType( void ) const { return mAttrType; }
	uint32_t GetSize( void ) const { return m_uSize; }
	uint32_t GetLength( void ) const { return m_uLength; }
	uint32_t GetColumnNum( void ) const { return m_uColumnNum; }
	uint32_t GetSingleByteSize() const { return m_uColumnNum * m_uElementSize; }
	uint32_t GetElementSize() const { return m_uElementSize; }
	bool IsAllocated( void ) const { return m_pStreamArray != NULL; }

private:
	char* m_pStreamArray;
	//VertexAttribute mAttrType;
	uint32_t m_uElementSize; // 1 element (4 for float)
	uint32_t m_uSize;		// byte size
	uint32_t m_uLength;	// the numbers of elements
	uint32_t m_uColumnNum;// the numbers of columns
};
typedef std::vector< Stream*, aya::Allocator<Stream*> > VectorVertexStream;

}

#endif // STREAM_H__

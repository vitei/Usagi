#ifndef CMDLBINARYSTORE_H
#define CMDLBINARYSTORE_H

#include <stdint.h>

#include "Cmdl.h"
#include "exchange/Shape.h"

class CmdlBinaryStore
{
public:
	CmdlBinaryStore( bool bSwapEndian = false );

	size_t calcStoredBinarySize( const Cmdl& cmdl, size_t alignment );
	void store( void* pBuffer, size_t size, const Cmdl& cmdl, size_t alignment );

private:
	void* StoreShapes( void* p, const Cmdl& cmdl, size_t alignment );
	void* StoreShape( void* p, const Cmdl& cmdl, const ::exchange::Shape* pShape, size_t alignment );

	void* StoreMaterials( void* p, const Cmdl& cmdl );
	void* StoreMaterial( void* p, const ::exchange::Material* pMaterial );

	void* StoreMeshes( void* p, const Cmdl& cmdl );
	void* StoreMesh( void* p, const ::exchange::Mesh* pMesh );

	void* StoreSkeleton( void* p, const Cmdl& cmdl );

	template <typename T> void CopyIndexStream( void* p, const ::exchange::Stream* pIndexStream );
	template <typename T> void CopyVertexStream( uint8* pDest, size_t destStep, const uint8* pSrc, size_t vertexElementSize, int vertexNum );

	bool mbSwapEndian;
};

#endif // CMDLBINARYSTORE_H

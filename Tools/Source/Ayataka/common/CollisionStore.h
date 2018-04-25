#ifndef COLLISIONSTORE_H
#define COLLISIONSTORE_H

#include <stdint.h>
#include "cmdl/Cmdl.h"

class CollisionStore
{
public:
	CollisionStore( bool bSwapEndian = false );
	virtual ~CollisionStore() {}

	size_t CalcStoredBinarySize( const Cmdl& cmdl );
	void Store( void* pBuffer, size_t size, const Cmdl& cmdl );

	size_t CalcNamesSectionBinarySize( const Cmdl& cmdl );
	void StoreNamesSection( void* p, size_t size, const Cmdl& cmdl );
private:
	size_t CalcShapeBinarySize( const Cmdl& cmdl, const ::exchange::Shape* pShape );

	typedef void* ( CollisionStore::*MeshProcessFunc )( void*, const Cmdl&, uint32_t );
	void* TraverseMesh( MeshProcessFunc func, void* p, const Cmdl& cmdl );

	typedef void* ( CollisionStore::*MaterialProcessFunc )( void*, const ::exchange::Material* );
	void* TraverseMaterial( MaterialProcessFunc func , void* p, const Cmdl& cmdl );

	void* StoreIndexStream( void* p, const ::exchange::Stream* pIndexStream, uint32_t indexOffset );
	void* StoreAdjacencyStream( void* p, const Cmdl& cmdl, uint32_t meshIndex );
	void* StorePositionStream( void* p, const Cmdl& cmdl, uint32_t meshIndex );
	void* StoreNormalStreamPerVertex( void* p, const Cmdl& cmdl, uint32_t meshIndex );
	void* StoreNormalStreamPerTriangle( void* p, const Cmdl& cmdl, uint32_t meshIndex );
	void* StoreMaterialIndexStream( void* p, const Cmdl& cmdl, uint32_t meshIndex );

	void* StoreMaterialAttributes( void* p, const ::exchange::Material* pMaterial );

	bool m_bSwapEndian;
	uint32_t m_contextOffset;
};

#endif // COLLISIONSTORE_H

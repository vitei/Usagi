#include "Mesh.h"

#include <string.h>

#include "common.h"

namespace exchange {

void copyString( char* pDest, size_t destLen, const char* pSrc, size_t srcLen )
{
	if( srcLen < destLen ) {
		strncpy( pDest, pSrc, srcLen );
		pDest[srcLen] = '\0';
	}
}

Mesh::Mesh()
{
	usg::exchange::Mesh_init( &m_mesh );
}

void Mesh::SetName( const char* p )
{
	copyString( m_mesh.name, ARRAY_SIZE( m_mesh.name ), p, strlen( p ) + 1 );
}


const char* Mesh::GetName( void )
{
	return m_mesh.name;
}

void Mesh::SetShapeRefIndex( uint32_t i )
{
	m_mesh.shapeRefIndex = i;
}

uint32_t Mesh::GetShapeRefIndex() const
{
	return m_mesh.shapeRefIndex;
}

void Mesh::SetMaterialRefName(const char* p)
{
	copyString( m_mesh.materialRefName, ARRAY_SIZE( m_mesh.materialRefName ), p, strlen( p ) );
}

const char*Mesh::GetMaterialRefName() const
{
	return m_mesh.materialRefName;
}

void Mesh::SetMaterialRefIndex(uint32_t value)
{
	m_mesh.materialRefIndex = value;
}

uint32_t Mesh::GetMaterialRefIndex() const
{
	return m_mesh.materialRefIndex;
}

}

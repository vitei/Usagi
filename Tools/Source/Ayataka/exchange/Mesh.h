#ifndef MESH_H
#define MESH_H

#include <stdio.h>
#include <math.h>

#include "common.h"

#include "Engine/Scene/Model/Mesh.pb.h"

namespace exchange {

static const int NAME_LENGTH = 32;

class Mesh
{
public:
	Mesh();
	virtual ~Mesh() {}

	void SetName( const char* p );
	const char* GetName( void );

	void SetShapeRefIndex( uint32_t i );
	uint32_t  GetShapeRefIndex( void ) const;

	void SetMaterialRefName( const char* p );
	const char* GetMaterialRefName( void ) const;

	void SetMaterialRefIndex(uint32_t value);
	uint32_t GetMaterialRefIndex() const;

    usg::exchange::Mesh& pb( void ) { return m_mesh; }
	const usg::exchange::Mesh& pb( void ) const { return m_mesh; }

private:
    usg::exchange::Mesh m_mesh;
};

}

#endif // MESH_H

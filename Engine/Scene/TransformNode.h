/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: A transformation and bounding box to be used (and ideally shared)
//	by graphics and physics objects for quadtree placement and frustum culling
*****************************************************************************/
#ifndef _USG_GRAPHICS_SCENE_TRANSFORM_NODE_H_
#define _USG_GRAPHICS_SCENE_TRANSFORM_NODE_H_

#include "Engine/Maths/Matrix4x4.h"
#include "Engine/Maths/Sphere.h"


namespace usg{

class TransformNode
{
public:
	TransformNode();
	~TransformNode() {}

	virtual uint32 GetId() = 0;
	void SetMatrix(const Matrix4x4 &mat);
	void SetBoundingSphere(const usg::Sphere &sphere);
	const Matrix4x4 &GetMatrix() const { return m_worldMat; }
	const usg::Sphere& GetWorldSphere() const;
	const usg::Sphere& GetSphere() const { return m_relSphere; }
    const Matrix4x4& GetMatrix() { return m_worldMat; }
	const Matrix4x4& GetLocalMatrix() { return m_worldMat; }
    
protected:
	void UpdateWorldSphere();

	Matrix4x4		m_worldMat;
	usg::Sphere		m_relSphere;
	usg::Sphere		m_worldSphere;

};

inline void TransformNode::UpdateWorldSphere()
{
	Vector3f vPos = m_worldMat.TransformVec3(m_relSphere.GetPos(), 1.0f);
	m_worldSphere.SetPos( vPos );
}


inline void TransformNode::SetBoundingSphere(const usg::Sphere &sphere)
{
	m_relSphere = sphere;
	m_worldSphere.SetRadius( sphere.GetRadius() );
	UpdateWorldSphere();
}

inline const usg::Sphere& TransformNode::GetWorldSphere() const
{
	return m_worldSphere;
}

}

#endif


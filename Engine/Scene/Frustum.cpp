/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Maths/AABB.h"
#include "Frustum.h"

namespace usg{

Frustum::Frustum(void)
{
}

Frustum::~Frustum(void)
{
}


void Frustum::SetUp(const Matrix4x4 &proj, const Matrix4x4 &view)
{
	Matrix4x4	viewProj = view * proj;
	InitFromMatrix(viewProj);
}

void Frustum::SetUp(const Matrix4x4 &viewProj)
{
	InitFromMatrix(viewProj);
}

void Frustum::SetUpLR(const Matrix4x4 &projL, const Matrix4x4 &viewL, const Matrix4x4 &projR, const Matrix4x4 &viewR)
{
	Matrix4x4	viewProjR = viewR * projR;

	SetUp(viewProjR);

	Matrix4x4	viewProjL = viewL * projL;
	float a, b, c, d;

	// TODO: Confirm this code, hasn't been tested since our old 3D stereo matrix. Not been confirmed in VR yet
	a = viewProjL[0][3] + viewProjL[0][0];
	b = viewProjL[1][3] + viewProjL[1][0];
	c = viewProjL[2][3] + viewProjL[2][0];
	d = viewProjL[3][3] + viewProjL[3][0];
	m_planes[PLANE_LEFT].Set( a, b, c, d );
	
	m_absNormals[PLANE_LEFT].AssignAbsolute(m_planes[PLANE_LEFT].GetNormal());
}

void Frustum::InitFromMatrix(const Matrix4x4 &viewProj)
{
	float a, b, c, d;
	
	// Right
	a = viewProj[0][3] - viewProj[0][0];
	b = viewProj[1][3] - viewProj[1][0];
	c = viewProj[2][3] - viewProj[2][0];
	d = viewProj[3][3] - viewProj[3][0];
	m_planes[0].Set( a, b, c, d );

	// Left
	a = viewProj[0][3] + viewProj[0][0];
	b = viewProj[1][3] + viewProj[1][0];
	c = viewProj[2][3] + viewProj[2][0];
	d = viewProj[3][3] + viewProj[3][0];
	m_planes[1].Set( a, b, c, d );

	// Bottom
	a = viewProj[0][3] + viewProj[0][1];
	b = viewProj[1][3] + viewProj[1][1];
	c = viewProj[2][3] + viewProj[2][1];
	d = viewProj[3][3] + viewProj[3][1];
	m_planes[2].Set( a, b, c, d );

	// Top
	a = viewProj[0][3] - viewProj[0][1];
	b = viewProj[1][3] - viewProj[1][1];
	c = viewProj[2][3] - viewProj[2][1];
	d = viewProj[3][3] - viewProj[3][1];
	m_planes[3].Set( a, b, c, d );

	// Far plane
	a = viewProj[0][3] - viewProj[0][2];
	b = viewProj[1][3] - viewProj[1][2];
	c = viewProj[2][3] - viewProj[2][2];
	d = viewProj[3][3] - viewProj[3][2];
	m_planes[4].Set( a, b, c, d );


	// Near plane	
#if Z_RANGE_0_TO_1
	a = viewProj[0][3];
	b = viewProj[1][3];
	c = viewProj[2][3];
	d = viewProj[3][3];
	m_planes[5].Set( a, b, c, d );
#else
	a = viewProj[0][3] + viewProj[0][2];
	b = viewProj[1][3] + viewProj[1][2];
	c = viewProj[2][3] + viewProj[2][2];
	d = viewProj[3][3] + viewProj[3][2];
	m_planes[5].Set( a, b, c, d );
#endif

	for(int i=0; i<PLANE_COUNT; i++)
	{
		m_absNormals[i].AssignAbsolute(m_planes[i].GetNormal());
	}
}

const Plane& Frustum::GetPlane(uint32 uIndex) const
{
	ASSERT(uIndex < PLANE_COUNT);
	return m_planes[uIndex];
}


PlaneClass Frustum::ClassifyBox(const AABB &box) const
{
	const Vector3f& vExtent = box.GetRadii();
	const Vector3f& vCenter = box.GetPos();

    PlaneClass result = PC_IN_FRONT;
	const Plane*	pPlane = &m_planes[0];

	// For every plane of frustum
	for( uint32 planeId = 0; planeId < PLANE_COUNT; planeId++, pPlane++ )
	{
        float d = DotProduct(vCenter, pPlane->GetNormal() );
        float r = DotProduct(vExtent, m_absNormals[planeId] );

        if(d + r < -pPlane->GetDistance())
        {
            // outside
            return PC_BEHIND;
        }
        if(d - r < -pPlane->GetDistance())
        {
        	// Intersecting
        	result = PC_ON_PLANE;
        }
	}

	return result;
}


bool Frustum::ArePointsInFrustum(const Vector4f* ppvPoints, uint32 uCount) const
{
	// For every plane of frustum
	const Plane*	pPlane = &m_planes[0];
	uint32 uInside = uCount;

	for(uint32 uPoint=0; uPoint < uCount; uPoint++)
	{
		uint32 uPointsInView = 0;
		for (uint32 planeId = 0; planeId < PLANE_COUNT; planeId++, pPlane++)
		{
			PlaneClass eClass =  pPlane->GetPointPlaneClass(ppvPoints[uPoint].v3());
			if(eClass != PC_BEHIND)
			{
				uPointsInView++;
			}
		}
		if (uPointsInView == 0)
		{
			return false;
		}
	}	

	return true;
}

}

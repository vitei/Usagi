/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Physics/CollisionDetection.h"
#include "Engine/Maths/Sphere.h"

namespace usg
{
	bool Sphere::LineIntersect(const usg::Vector3f& vStart, const usg::Vector3f& vEnd, usg::Vector3f& vPosOut)
	{
		real cx = m_pos.x;
		real cy = m_pos.y;
		real cz = m_pos.z;

		real px = vStart.x;
		real py = vStart.y;
		real pz = vStart.z;

		usg::Vector3f vDir = vEnd - vStart;

		real fA = DotProduct(vDir, vDir);
		real fB = 2.0f * (vStart.x * vDir.x + py * vDir.y + pz * vDir.z - vDir.x * cx - vDir.y * cy - vDir.z * cz);
		real fC = vStart.x * vStart.x - 2 * vStart.x * cx + cx * cx + py * py - 2 * py * cy + cy * cy +
			pz * pz - 2 * pz * cz + cz * cz - m_radius * m_radius;

		real fD = fB * fB - 4 * fA * fC;
		real fDSqrt = usg::Math::SqrtSafe(fD);

		real t1 = (-fB - fDSqrt) / (2.0f * fA);

		Vector3f vSolution1(vStart.x * (1 - t1) + t1 * vEnd.x,
			vStart.y * (1 - t1) + t1 * vEnd.y,
			vStart.z * (1 - t1) + t1 * vEnd.z);

		real t2 = (-fB + fDSqrt) / (2.0f * fA);
		Vector3f vSolution2(vStart.x * (1 - t2) + t2 * vEnd.x,
			vEnd.y * (1 - t2) + t2 * vEnd.y,
			vEnd.z * (1 - t2) + t2 * vEnd.z);

		if (fD < 0 || t1 > 1)// || t2 > 1)
		{
			return false;
		}

		if (t1 < 0.0f)
		{
			// Behind the start
			return false;
		}
		
		vPosOut = vSolution1;
		return true;
	}

}
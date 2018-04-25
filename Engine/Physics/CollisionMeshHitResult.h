/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: A quad tree for collisions, code adapted from nsub
*****************************************************************************/
#ifndef _USG_PHYSICS_COLLISION_MATERIAL_HIT_RESULT_H_
#define _USG_PHYSICS_COLLISION_MATERIAL_HIT_RESULT_H_
#include "Engine/Common/Common.h"
#include "Engine/Maths/Vector3f.h"

namespace usg
{

	struct CollisionMeshHitResult
	{
        CollisionMeshHitResult()
        {
			uMaterialFlags = 0;
			pMaterialName = NULL;
			uTriIndex = 0;
			pNode = NULL;
			uMaterialNameHash = 0;
            vNorm = Vector3f(0,1,0);
            vPoints[0] = Vector3f(0,0,0);
            vPoints[1] = Vector3f(0,0,0);
            vPoints[2] = Vector3f(0,0,0);
			fDistance = 10000;
			bHit = false;			
        }
		uint32			uMaterialNameHash;
		const char*		pMaterialName;
		uint32			uMaterialFlags;
		Vector3f		vNorm;
		Vector3f		vIntersectPoint;
		float			fDistance;
		bool			bHit;
		Vector3f		vPoints[3];
		int				vIndices[3];
		uint32			uTriIndex;
		// Using a void pointer to avoid unnecessary includes
		const void*		pNode;		
		
	};
}

#endif
	
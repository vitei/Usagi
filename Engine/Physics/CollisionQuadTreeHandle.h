/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: A handle to the quad tree
*****************************************************************************/
#ifndef _USG_PHYSICS_COLLISION_QUADTREE_HANDLE_H_
#define _USG_PHYSICS_COLLISION_QUADTREE_HANDLE_H_

namespace usg
{

class CollisionQuadTree;
namespace Components
{
struct QuadTreeHandle
{
	union {
		uint64_t quadtreeID;           //Unused at present; could be used to choose between multiple quadtrees
		CollisionQuadTree* m_quadtree;
	};
};
}

}

#include "Engine/Physics/CollisionData.pb.h"

#endif //_USG_PHYSICS_COLLISION_QUADTREE_HANDLE_H_

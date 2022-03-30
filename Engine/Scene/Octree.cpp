/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Scene/Frustum.h"
#include "Engine/Scene/Octree.h"
#include <float.h>

namespace usg {

Octree::Octree():
m_nodePool(MAX_BLOCKS),
m_componentPool(512)
{
	m_pParentNode = NULL;
	Reset();
}

Octree::~Octree()
{

}

void Octree::Init(const AABB& worldBounds, uint32 uMaxPerLevel, float fLooseness)
{
	Reset();
	// It's a multiplier so it needs to be bigger than the main box but smaller than the parent to have any meaning
	ASSERT(fLooseness > 1.0f && fLooseness < 2.0f);	
	// Need atleast two per node as we don't resize until we've fallen below half
	ASSERT(uMaxPerLevel >= 2);

	m_fLooseness	= fLooseness;
	m_uMaxPerLevel	= uMaxPerLevel;

	m_pParentNode = m_nodePool.Alloc();
	m_pParentNode->Init(this, worldBounds, NULL, 0);

}

void Octree::Reset()
{
	if(m_pParentNode)
	{
		m_pParentNode->FreeChildren();
		m_nodePool.Free(m_pParentNode);
		m_pParentNode = NULL;
	}

	m_fLooseness	= 0.0f;
	m_uMaxPerLevel	= 0;
}


void Octree::InsertObject(const TransformNode* pNode, void* pUserData, uint32 uMask)
{
	OctreeComponent* pComponent = m_componentPool.Alloc();
	pComponent->pNode 		= pNode;
	pComponent->pUserData	= pUserData;
	pComponent->pNext		= NULL;
	pComponent->uTestMask	= uMask;

	m_pParentNode->InsertObject(pComponent);
}

void Octree::UpdateMask(const TransformNode* pNode, uint32 uMask)
{
	OctreeComponent* pComponent = NULL;
	for(FastPool<OctreeComponent>::Iterator it = m_componentPool.Begin(); !it.IsEnd(); ++it)
	{
		if( (*it)->pNode == pNode )
		{
			pComponent = *it;
			pComponent->uTestMask = uMask;
			break;
		}
	}
}


void Octree::RemoveObject(const TransformNode* pNode)
{
	OctreeComponent* pComponent = NULL;
	uint32 uCount = 0;
	for(FastPool<OctreeComponent>::Iterator it = m_componentPool.Begin(); !it.IsEnd(); ++it)
	{
		if( (*it)->pNode == pNode )
		{
			pComponent = *it;
			uCount++;
		}
	}

	ASSERT(uCount == 1);

	if(pComponent)
	{
		bool bFound = pComponent->pParent->RemoveObject(pComponent);
		ASSERT(bFound);
		m_componentPool.Free(pComponent);	
	}
	else
	{
		ASSERT(false);	// Couldn't find the component
	}
	
}


void Octree::UpdateTransforms()
{
	for(FastPool<OctreeComponent>::Iterator it = m_componentPool.Begin(); !it.IsEnd(); ++it)
	{
		OctreeComponent* pComponent = *it;
		const TransformNode* pTransform	= pComponent->pNode;
		Node* pCurrentNode			= pComponent->pParent;
		if( !pCurrentNode->GetLooseBox().ContainedInBox(pTransform->GetWorldSphere() ) )
		{
			// Remove the object and try re-inserting it into the parent, only once they've totally left the loose box
			pCurrentNode->RemoveObject(pComponent);
			pComponent->pParent = NULL;
			pComponent->pNext	= NULL;
			m_pParentNode->InsertObject(pComponent);
		}
	}
}


void Octree::GetVisibleList(SearchObject& object) const
{
	object.GetVisibleList(this);
}

void Octree::GetVisibleList( SearchFrustum& searchObj ) const
{
	m_pParentNode->GetVisibleList(searchObj);
}

void Octree::GetVisibleList(SearchSphere& searchObj) const
{
	m_pParentNode->GetVisibleList(searchObj);
}

// **********************************************************************
//
// Octree::Node functions
//
// **********************************************************************

void Octree::Node::Init(Octree* pOctree, AABB tightAABB, Node* pParent, uint32 uDepth)
{
	m_uEntryCount = 0;
	m_uDepth = uDepth;

	m_aabbTight = tightAABB;
	CalculateLooseBounds(pOctree->GetLooseness());

	m_pParent = pParent;
	m_pOctree = pOctree;

	for(int i=0; i<NUM_CHILDREN; i++)
	{
		m_pChildren[i] = NULL;
	}

	m_pGeomList = NULL;
}


Octree::Node* Octree::Node::FindContainingChild(OctreeComponent* pComponent)
{
	Sphere worldBounds = pComponent->pNode->GetWorldSphere();
	Vector3f vTransPos = worldBounds.GetPos();
	Node* pChild = NULL;
	if( HasChildren() )
	{
		// TODO: Good place for optimization
		for(int i=0; i<NUM_CHILDREN; i++)
		{
			if( m_pChildren[i]->GetTightBox().InBox(worldBounds.GetPos()) )
			{
				pChild = m_pChildren[i];
				break;
			}
		}
		if(pChild && pChild->GetLooseBox().ContainedInBox(worldBounds))
		{
			return pChild;
		}
	}
	return NULL;
}

void Octree::Node::GetChildBox(uint32 uId, AABB &boxOut)
{
	Vector3f vBoundsMin = m_aabbTight.GetMin();
	Vector3f vBoundsMax = m_aabbTight.GetMax();
	Vector3f vHalfRange = (vBoundsMax - vBoundsMin)/2.f;

	if(uId > 3)
	{
		vBoundsMin.x += vHalfRange.x;
	}
	else
	{
		vBoundsMax.x -= vHalfRange.x;
	}

	switch(uId)
	{
		case 0:
		case 1:
		case 4:
		case 5:
		{
			vBoundsMin.y += vHalfRange.y;
		}
		break;
		default:
		{
			vBoundsMax.y -= vHalfRange.y;
		}
	}

	switch(uId)
	{
		case 0:
		case 2:
		case 4:
		case 6:
		{
			vBoundsMin.z += vHalfRange.z;
		}
		break;
		default:
		{
			vBoundsMax.z -= vHalfRange.z;
		}
	}

	boxOut.SetMinMax(vBoundsMin, vBoundsMax);
}

void Octree::Node::AllocateChildren()
{
	for(uint32 i=0; i<NUM_CHILDREN; i++)
	{
		m_pChildren[i] = m_pOctree->GetNodePool().Alloc();
		AABB childBox;
		GetChildBox(i, childBox);
		m_pChildren[i]->Init(m_pOctree, childBox, this, m_uDepth+1);
	}

	// Try putting all of our current components into a lower level of the octree
	OctreeComponent* pComponent = m_pGeomList;
	OctreeComponent* pPrev = NULL;
	while(pComponent)
	{
		OctreeComponent* pNext = pComponent->pNext;

		Node* pChild = FindContainingChild(pComponent);
		if(pChild)
		{
			if(pComponent == m_pGeomList)
			{
				m_pGeomList = pComponent->pNext;
			}
			else
			{
				if(pPrev)
				{
					pPrev->pNext = pComponent->pNext;
				}
				if(pComponent->pNext)
				{
					pComponent->pNext = pPrev;
				}
			}
			pChild->InsertObject(pComponent);
		}
		else
		{
			pPrev = pComponent;
		}
		pComponent = pNext;
	}
}

bool Octree::Node::InsertObject(OctreeComponent* pComponent)
{
	
	bool bInBox =  m_aabbLoose.ContainedInBox( pComponent->pNode->GetWorldSphere() );
	if(!bInBox)
	{
		//DEBUG_PRINT("Object out of world bounds, adding to parent\n");
		ASSERT(m_uDepth == 0);
	}

	// Check to see if we need to create a new set of nodes to 
	if(bInBox && !HasChildren() && m_uEntryCount > m_pOctree->GetMaxPerLevel() && m_uDepth < Octree::MAX_DEPTH)
	{
		// Create the children for this node
		AllocateChildren();
	}

	if( HasChildren() )
	{
		Node* pChild = FindContainingChild(pComponent);
		if(pChild)
		{
			if( pChild->InsertObject(pComponent) )
			{
				// Our entry count is that of our children as well as our own
				m_uEntryCount++;
				return true;
			}
		}
	}

	// Couldn't fit into the children so we'll put it into our own node
	//ASSERT( m_aabbLoose.InBox( pComponent->pNode->GetWorldSphere() ) );

	if( m_pGeomList )
	{
		pComponent->pNext = m_pGeomList;
		m_pGeomList = pComponent;
	}
	else
	{
		m_pGeomList = pComponent;
		pComponent->pNext = NULL;
	}

	pComponent->pParent = this;
	m_uEntryCount++;

	return true;	
}



void Octree::Node::CalculateLooseBounds(float fLooseness)
{
	Vector3f vRadii = m_aabbTight.GetRadii();

	vRadii *= fLooseness;

	m_aabbLoose.SetPos(m_aabbTight.GetPos());
	m_aabbLoose.SetRadii(vRadii);
}

void Octree::Node::FreeChildren()
{
	for(int i=0; i<NUM_CHILDREN; i++)
	{
		if(m_pChildren[i])
		{
			m_pChildren[i]->FreeChildren();
			m_pOctree->GetNodePool().Free(m_pChildren[i]);
			m_pChildren[i] = NULL;
		}
	}
}


void Octree::Node::AbsorbChildren()
{
	if (HasChildren())
	{
		for (uint32 uChild = 0; uChild < NUM_CHILDREN; uChild++)
		{
			m_pChildren[uChild]->AbsorbChildren();
		}
	}

	for (uint32 uChild = 0; uChild < NUM_CHILDREN; uChild++)
	{
		if (m_pChildren[uChild])
		{
			OctreeComponent* pComponent = m_pChildren[uChild]->GetComponentList();
			m_pChildren[uChild]->m_pGeomList = NULL;
			while (pComponent)
			{
				OctreeComponent* pNext = pComponent->pNext;

				if (m_pGeomList)
				{
					pComponent->pNext = m_pGeomList;
					m_pGeomList = pComponent;
				}
				else
				{
					m_pGeomList = pComponent;
					pComponent->pNext = NULL;
				}

				pComponent->pParent = this;
				pComponent = pNext;
			}
		}
	}
	FreeChildren();

}


void Octree::Node::NotifyRemoval()
{
	m_uEntryCount--;

	// TODO: Is this the best place to perform this, could immediately insert a new node
	if(m_uEntryCount < m_pOctree->GetMinPerLevel() )
	{
		AbsorbChildren();
	}
}

bool Octree::Node::RemoveObject(OctreeComponent* pComponent)
{
	OctreeComponent* pCompare = m_pGeomList;
	OctreeComponent* pPrev = NULL;

	while(pCompare)
	{
		if( pCompare == pComponent )
		{
			if(pPrev)
			{
				pPrev->pNext = pCompare->pNext;
			}
			else
			{
				m_pGeomList = pCompare->pNext;
			}
			m_uEntryCount--;
			if(m_pParent)
			{
				m_pParent->NotifyRemoval();
			}
			return true;
		}
		pPrev = pCompare;
		pCompare = pCompare->pNext;
	}
	
	return false;
}


void Octree::Node::GetVisibleList(SearchFrustum& object)
{
	// No point testing this box, we don't have anything
	if (m_uEntryCount == 0)
		return;

	const Frustum* pFrustum = object.GetFrustum();

	if (pFrustum->ClassifyBox(m_aabbLoose) != PC_BEHIND)
	{
		OctreeComponent* pComponent = m_pGeomList;
		uint32 uMask = object.GetMask();
		uint32 uReq = object.GetReqFlags();
		uint32 uExcl = object.GetExclFlags();

		while (pComponent)
		{
			if ((uMask & pComponent->uTestMask) != 0 && ((uReq & pComponent->uTestMask) == uReq)
				&& ((uExcl & pComponent->uTestMask) == 0) )
			{
				if (pFrustum->IsSphereInFrustum(pComponent->pNode->GetWorldSphere()))
				{
					object.Callback(pComponent->pUserData);
				}
			}
			pComponent = pComponent->pNext;
		}

		if (HasChildren())
		{
			for (int i = 0; i < NUM_CHILDREN; i++)
			{
				m_pChildren[i]->GetVisibleList(object);
			}
		}
	}
}

void Octree::Node::GetVisibleList(SearchSphere& object)
{
	// No point testing this box, we don't have anything
	if (m_uEntryCount == 0)
		return;

	const Sphere*  pSphere = object.GetSphere();

	if (m_aabbLoose.ContainedInBox(*pSphere))
	{
		OctreeComponent* pComponent = m_pGeomList;
		uint32 uMask = object.GetMask();

		while (pComponent)
		{
			if ((uMask & pComponent->uTestMask) != 0)
			{
				if (pComponent->pNode->GetWorldSphere().Intersect(*pSphere))
				{
					object.Callback(pComponent->pUserData);
				}
			}
			pComponent = pComponent->pNext;
		}

		if (HasChildren())
		{
			for (int i = 0; i < NUM_CHILDREN; i++)
			{
				m_pChildren[i]->GetVisibleList(object);
			}
		}
	}
}


const AABB& Octree::GetWorldBounds() const
{
	return m_pParentNode->GetTightBox();
}


Octree::SearchObject::SearchObject()
{
	m_uExclFlags = 0;
	m_uReqFlags = 0;
	m_uMask = 0xffffffff;
}

Octree::SearchObject::~SearchObject()
{

}

void Octree::SearchObject::InitInt(uint32 uMask, uint32 uReqFlags)
{
	m_uMask		= uMask;
	m_uReqFlags = uReqFlags;
}

}

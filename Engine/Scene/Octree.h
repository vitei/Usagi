/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Encloses world geometry based by their bounding volume
//	into a lose octree
*****************************************************************************/
#ifndef _USG_GRAPHICS_SCENE_OCTREE_H_
#define _USG_GRAPHICS_SCENE_OCTREE_H_
#include "Engine/Common/Common.h"
#include "Engine/Maths/AABB.h"
#include "Engine/Scene/TransformNode.h"
#include "Engine/Memory/FastPool.h"

namespace usg{

class Frustum;

class Octree
{
public:
	Octree();
	~Octree();

	void	Init(const AABB& worldBounds, uint32 uMaxPerLevel, float fLooseness = 1.2f);
	void	Reset();

	void	InsertObject(const TransformNode* pNode, void* pUserData, uint32 uMask = 0xFFFFFFFF);
	void	UpdateMask(const TransformNode* pNode, uint32 uMask);
	void	RemoveObject(const TransformNode* pNode);
	uint32	GetMaxPerLevel() const { return m_uMaxPerLevel; }
	// Don't resize a given block unless it's got half the number we consider necessary
	uint32  GetMinPerLevel() const { return m_uMaxPerLevel/2; }
	float	GetLooseness() const { return m_fLooseness; }
	// Call after the world update to account for the motion of any of the transforms
	void	UpdateTransforms();

	const AABB& GetWorldBounds() const;

	class SearchObject
	{
	public:
		SearchObject();
		~SearchObject();

		void InitInt(uint32 uMask = 0xFFFFFFFF);
		uint32 GetMask() const { return m_uMask; }
		void SetMask(uint32 uMask) { m_uMask = uMask; }
		virtual void Callback(void* pUserData) = 0;
		virtual void GetVisibleList(Octree* pOctree) = 0;

	protected:
		uint32			m_uMask;

	};

	class SearchFrustum : public SearchObject
	{
	public:
		const Frustum* GetFrustum() const { return m_pFrustum; }
		void SetFrustum(const Frustum* pFrustum) { m_pFrustum = pFrustum; }
		void GetVisibleList(Octree* pOctree) override { pOctree->GetVisibleList(*this); }

	protected:
		const Frustum*	m_pFrustum;
	};

	class SearchSphere : public SearchObject
	{
	public:
		const usg::Sphere* GetSphere() const { return m_pSphere; }
		void SetSphere(const usg::Sphere* sphere) { m_pSphere = sphere; }
		void GetVisibleList(Octree* pOctree) override { pOctree->GetVisibleList(*this); }

	protected:
		const usg::Sphere*	m_pSphere;
	};

	void	GetVisibleList(SearchObject& object);
	void	GetVisibleList(SearchFrustum& object);
	void	GetVisibleList(SearchSphere& object);

private:
	enum
	{
		NUM_CHILDREN	= 8,
		NUM_NEIGHBOURS	= 6,
		MAX_DEPTH		= 6,
		MAX_BLOCKS		= ((((MAX_DEPTH-1)*NUM_CHILDREN)*MAX_DEPTH) + 1)
	};

	class Node;

	struct OctreeComponent
	{
		Node*					pParent;
		const TransformNode*	pNode;
		void*					pUserData;
		OctreeComponent*		pNext;
		uint32					uTestMask;
	};

	class Node
	{
	public:
		Node() {}
		~Node() {};

		void	Init(Octree* pOctree, AABB tightAABB, Node* pParent, uint32 uDepth);
		const Vector3f& GetCentrePos() { return m_aabbTight.GetPos(); }
		uint32	GetDepth() { return m_uDepth; }
		void 	AllocateChildren();
		void 	FreeChildren();
		bool	HasChildren() { return m_pChildren[0]!=NULL; }
		bool	InsertObject(OctreeComponent* pComponent);	
		bool	RemoveObject(OctreeComponent* pComponent);	
		Node*	FindContainingChild(OctreeComponent* pComponent);

		void	GetVisibleList( SearchFrustum& object );
		void	GetVisibleList(SearchSphere& object);
		
		OctreeComponent* GetComponentList() { return m_pGeomList; }
		// Called when an object is removed from a child so we can let the parent know that it may have to resize
		void	NotifyRemoval();
		void	AbsorbChildren();

		const AABB&	GetTightBox() { return m_aabbTight; }
		const AABB& GetLooseBox() { return m_aabbLoose; }


	private:
		void CalculateLooseBounds(float fLooseness);
		void GetChildBox(uint32 uId, AABB &boxOut);

		AABB				m_aabbTight;
		AABB				m_aabbLoose;

		Octree*				m_pOctree;
		Octree::Node*		m_pParent;
		Octree::Node*		m_pChildren[NUM_CHILDREN];
		// Useful for collision detection
		OctreeComponent*	m_pGeomList;
		uint32				m_uEntryCount;
		uint32				m_uDepth;
	};

	Node* AllocateNode(Node* pParent, const AABB& bounds);
	void FreeNode(Node* pNode);
	FastPool<Node>&	GetNodePool() { return m_nodePool; }

	Node*			m_pParentNode;
	float			m_fLooseness;
	uint32			m_uMaxPerLevel;

	// Create a pool large enough to contain all of the nodes we may need
	FastPool<Node>				m_nodePool;
	FastPool<OctreeComponent>	m_componentPool;
};

}

#endif


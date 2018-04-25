/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: A quad tree for collisions, code adapted from nsub
*****************************************************************************/
#ifndef _USG_PHYSICS_COLLISION_QUADTREE_H_
#define _USG_PHYSICS_COLLISION_QUADTREE_H_
#include "Engine/Common/Common.h"
#include "Engine/Maths/AABB.h"
#include "Engine/Maths/Vector3f.h"
#include "Engine/Maths/Vector4f.h"
#include "Engine/Physics/CollisionQuadTree.pb.h"
#include "Engine/Physics/CollisionMeshHitResult.h"

#ifdef _DEBUG
//#define DRAW_VISIBLE_MODEL
#endif

#ifdef DRAW_VISIBLE_MODEL
#include "Engine/Scene/TransformNode.h"
#include "Engine/Scene/Common/Mesh.h"
#endif

namespace usg{

struct PositionNormalUVVertex;
class Scene;
class VertexBuffer;
class GFXDevice;

class CollisionQuadTree
{
public:

	enum {
		MF_GROUND		= 1<<0,
		MF_WALL			= 1<<1,
		MF_HARD			= 1<<2,
		MF_NOCOLLISION	= 1<<3,
		MF_WATER		= 1<<4,
		MF_NOFIRE		= 1<<5,
		MF_SLOW			= 1<<6,
		MF_FAST			= 1<<7,

		MF_ALL			= 0xffff
	};

	static const uint32 MaxHits = 8;

	
	class Node;

	

	CollisionQuadTree();
	~CollisionQuadTree();

	void		Load(const char* szFileName, const AABB& worldBounds);
	int			CheckInsert(Node* pNode, uint32 uIndex, uint32 uDepth);
	void		Dump(const Node* pNode, uint32,int);
	const Node*	FindNode(const Node* pNode, const Vector3f& vPoint);
	const Node*	FindNode(const Vector3f& vPoint);
	const Node* GetRootNode() const { return m_pRoot; }
	uint32		FindNodes(const Node* pNode, const Vector3f& vPoint, float fRadius, const Node** ppOut, uint32 uMax, uint32 uCnt);

	bool		InBounds(const Vector3f &p);
	bool		InBounds(const Vector3f &p, float r);
	bool		InBounds(const Vector3f &min, const Vector3f &max);

	int			ClipLine(const Node* pNode, const Vector3f &vFrom, const Vector3f &vTo, const Vector3f &vDir, float fClosest, bool bSmooth, CollisionMeshHitResult *pHits, uint32 uMaxHits, uint32 uNumHits, uint32 uIncludeFlags) const;
	float		ClipLine(const Node* pNode, const Vector3f &vFrom, const Vector3f &vTo, const Vector3f &vDir, float fClosest, bool bSmooth, CollisionMeshHitResult& hit, uint32 uIncludeFlags, uint32 uMaxHits = MaxHits) const;

	bool		ClipLine(const Vector3f &vFrom, const Vector3f &vTo, CollisionMeshHitResult& hit, uint32 uMaxHits = MaxHits) const;

	int			ClipSphere(const Node* pNode, const Vector3f &pos, float radius, CollisionMeshHitResult *pHits, uint32 uMaxHits, uint32 uNumHits, uint32 uIncludeFlags) const;

	void		UpdateBoundingBoxes(Node *pNode);


	float		GetMaxHeightAt(float x, float z, Vector3f* pNormOut) const;
    bool        FindLocation(float fDotCmpVal, const Vector3f& vPos, float radius, Vector3f& vOut, uint32 uNumChecks = 20);

	float		getDistanceQT(const Vector3f &pos, CollisionMeshHitResult *, float radius);
	
	void		SmoothNormal(CollisionMeshHitResult &);
	
	uint32 searchAdjTriangleRecursively( uint32 triangleIndices[], uint32 triangleNum, const uint32 triangleMax,
									   const uint32 tgtIndex, const uint32 rootIndex, const usg::Sphere& sphere ) const;
	uint32 setupGroundPatchIndices( uint16 indices[], uint32 indicesMax, float radius, const CollisionMeshHitResult& hitResult ) const;
	void initVertexBuffer( GFXDevice* pDevice, VertexBuffer& vertexBuffer ) const;

#ifdef DRAW_VISIBLE_MODEL
	void SetupVisibleModel( GFXDevice* pDevice, Scene* pScene );
#endif	

	class Node
	{
	public:
		Node(const Vector3f& min, const Vector3f& max, Node* parent);
		~Node();

		void		CreateChildren();
		Vector3f	GetMin() const { return m_boundingBox.GetMin(); }
		Vector3f	GetMax() const { return m_boundingBox.GetMax(); } 
		const Node* GetChild(uint32 uChild) const { ASSERT(uChild < 4); return m_pChildren[uChild]; }
		Node*		GetChild(uint32 uChild) { ASSERT(uChild < 4); return m_pChildren[uChild]; }
		Node*		GetParent() { return m_pParent; }
		uint32		GetNumIndices() const { return m_uIndexCnt; }
		void		FlushIndices() { m_uIndexCnt = 0;  m_bHasLeaves = true;  }
		uint32		GetIndex(uint32 uIndex) const { ASSERT(uIndex < m_uIndexCnt); return m_uIndices[uIndex]; }
		void		AddIndex(uint32 uIndex);
		const		AABB& GetAABB() const { return m_boundingBox; }
		AABB&		GetAABB() { return m_boundingBox; }
		
		bool		InBounds(const Vector3f &) const;
		bool		InBounds(const Vector3f &p, float r) const;
		bool		InBounds(const Vector3f &min, const Vector3f &max) const;
		uint8		GetDepth() const { return m_depth; }
		bool		HasLeaves() const { return m_bHasLeaves; }
		bool		IsBranch() const { return m_bHasLeaves || m_uIndexCnt > 0;  }

	private:

		enum
		{
			MAX_INDICES = 100
		};
	private:

		uint32		m_uIndexCnt;
		uint16		m_uIndices[MAX_INDICES];
		uint8		m_depth;
		AABB		m_boundingBox;
		Node*		m_pChildren[4];
		Node*		m_pParent;
		bool		m_bHasLeaves;
	};
	
	
private:

	bool InTriangle(const Vector3f &P, const Vector3f &A, const Vector3f &B, const Vector3f &C) const;
	uint32 GetClipCodes(const Vector3f& p, const Vector3f& min, const Vector3f& max) const;
	uint32 GetClipCodesXZ(const Vector3f& p, const Vector3f& min, const Vector3f& max) const;
	bool InBox(const Vector3f &p1,const Vector3f &p2,const Vector3f &p3, const AABB& aabb);
	bool InBoxXZ(const Vector3f &p1, const Vector3f &p2, const Vector3f &p3, const AABB& aabb);
	float GetMaxHeightAt(const Node* pNode, float x, float z, Vector3f* pNormOut) const;

	struct TriangleIndices
	{
		uint32 i1;
		uint32 i2;
		uint32 i3;
	};

	struct MaterialAttribute
	{
		char name[usg::CollisionQuadTreeConstants_MATERIAL_NAME_LENGTH];
	};


	Node*						m_pRoot;
    TriangleIndices*            m_pTriangles;
	TriangleIndices*            m_pAdjacentTriangles;
	Vector3f*                   m_pVertices;
	Vector3f*					m_pVertexNormals;
	Vector3f*                   m_pTriangleNormals;
	uint32*						m_pMaterialIndices;
	MaterialAttribute*			m_pMaterialAttributes;
	uint32*						m_pMaterialNameHash;
	uint8*						m_pMaterialFlags;
	uint32						m_uTriangles;
	uint32						m_uVertices;
	uint32						m_uNumNodes;


	const Node*					m_pLastFound;

#ifdef DRAW_VISIBLE_MODEL
	const EffectBinding*	m_pVisibleEffect;
	Mesh					m_VisibleMesh;
	TransformNode*			m_pVisibleTransform;
	RenderGroup*			m_pVisibleRenderGroup;
#endif
};

}


#endif

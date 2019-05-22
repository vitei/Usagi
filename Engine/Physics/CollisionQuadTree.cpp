/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Core/File/File.h"
#include "Engine/Graphics/StandardVertDecl.h"
#include "CollisionQuadTree.h"
#include "Engine/Maths/MathUtil.h"
#include "Engine/Maths/Plane.h"
#include "Engine/Physics/CollisionDetection.h"
#include "Engine/Debug/Rendering/DebugRender.h"

#include <float.h>
#include <string.h>

#ifdef DRAW_VISIBLE_MODEL
#include "Engine/Scene/Scene.h"
#include "Engine/Resource/ResourceMgr.h"
#endif

namespace usg{
	


void CollisionQuadTree::Node::CreateChildren()
{
	Vector3f vCent = m_boundingBox.GetPos();

	const float minY = 100000.0f;
	const float maxY = -100000.0f;

	m_pChildren[0] = vnew(ALLOC_COLLISION) Node(Vector3f(GetMin().x, minY, GetMin().z), Vector3f(vCent.x,maxY, vCent.z), this);
	m_pChildren[1] = vnew(ALLOC_COLLISION) Node(Vector3f( vCent.x, minY, GetMin().z),	Vector3f( GetMax().x, maxY, vCent.z ), this );
	m_pChildren[2] = vnew(ALLOC_COLLISION) Node(Vector3f( GetMin().x, minY, vCent.z),	Vector3f( vCent.x, maxY, GetMax().z ), this );
	m_pChildren[3] = vnew(ALLOC_COLLISION) Node(Vector3f(vCent.x, minY, vCent.z),		Vector3f(GetMax().x, maxY, GetMax().z), this );
}

CollisionQuadTree::CollisionQuadTree()
{
	m_pRoot			= NULL;
	m_pTriangles	= NULL;
	m_uVertices		= 0;
	m_pVertices		= NULL;
	m_pVertexNormals = NULL;
	m_pTriangleNormals= NULL;
	m_pMaterialIndices = NULL;
	m_pMaterialNameHash = NULL;
	m_pMaterialAttributes = NULL;
	m_pMaterialFlags = NULL;
	m_uTriangles	= 0;
	m_pAdjacentTriangles = NULL;
	m_pLastFound	= NULL;

	m_uNumNodes = 0;

#ifdef DRAW_VISIBLE_MODEL
	m_pVisibleEffect = NULL;
	m_pVisibleTransform = NULL;
	m_pVisibleRenderGroup = NULL;
#endif
}

CollisionQuadTree::~CollisionQuadTree()
{
	if(m_pRoot)
	{
		vdelete m_pRoot;
	}

	if(m_pTriangles)
		mem::Free(MEMTYPE_STANDARD, (void*)m_pTriangles);
	
	if(m_pVertices)
		mem::Free(MEMTYPE_STANDARD, (void*)m_pVertices);

	if( m_pVertexNormals )
		mem::Free( MEMTYPE_STANDARD, (void*)m_pVertexNormals );
	
	if(m_pTriangleNormals)
		mem::Free(MEMTYPE_STANDARD, (void*)m_pTriangleNormals);

	if( m_pMaterialIndices ) {
		mem::Free( MEMTYPE_STANDARD, (void*)m_pMaterialIndices );
	}

	if( m_pMaterialAttributes ) {
		mem::Free( MEMTYPE_STANDARD, (void*)m_pMaterialAttributes );
	}

	if (m_pMaterialFlags) {
		mem::Free(MEMTYPE_STANDARD, (void*)m_pMaterialFlags);
	}

	if( m_pAdjacentTriangles )
	{
		mem::Free( MEMTYPE_STANDARD, (void*)m_pAdjacentTriangles );
	}

	if (m_pMaterialNameHash)
	{
		mem::Free(MEMTYPE_STANDARD, (void*)m_pMaterialNameHash);
	}

}

CollisionQuadTree::Node::~Node()
{
	for(int i=0; i<4; i++)
	{
		if(m_pChildren[i])
		{
			delete m_pChildren[i];
			m_pChildren[i] = NULL;
		}
	}
}



#if 0
Vector3 GetBounds(usg::CollisionQuadTree::Node *pNode, Vector3f &min, Vector3)
{
if (pNode->GetChild(0))
{
	for (int i = 0; i<4; i++)
	{
		uNumHits = ClipSphere(pNode->GetChild(i), pos, radius, pHits, uMaxHits, uNumHits, uIncludeFlags);
	}
}
else
{
	AABB::BoxCorners corners;

	pNode->GetAABB().GetCorners(corners);a

	for (int i = 0; i < 8; i++)
		DebugRender::GetRenderer()->AddSphere(corners.verts[i], 0.5, Color(0.3f, 0.3f, 1.0f, 0.8f));



	CollisionMeshHitResult chit;

	for (uint32 i = 0; i < pNode->GetNumIndices(); i++)
	{
		int index = pNode->GetIndex(i);


		if (!(m_pMaterialFlags[m_pMaterialIndices[index]] & uIncludeFlags))
			continue;

		const TriangleIndices* tris = &m_pTriangles[index];

		const Vector3f &p1 = m_pVertices[tris->i1];
		const Vector3f &p2 = m_pVertices[tris->i2];
		const Vector3f &p3 = m_pVertices[tris->i3];

#endif




void CollisionQuadTree::Node::AddIndex(uint32 uIndex)
{
	if(m_uIndexCnt < MAX_INDICES)
	{
		m_uIndices[m_uIndexCnt++] = uIndex;
	}
	else
	{
		ASSERT("Dropping a collision tree index\n");
	}
}


void CollisionQuadTree::Load(const char* szFileName, const AABB& worldBounds)
{
	static const uint32 AyatakaMagicNumber = utl::CRC32("AyatakaCollisionModel");

	File dataFile(szFileName);

	uint32 uMagicNumber, uFileVersion;
	dataFile.Read(sizeof(uMagicNumber), &uMagicNumber);
	ASSERT(uMagicNumber == AyatakaMagicNumber && "Not a proper Ayataka-generated collision model file");
	dataFile.Read(sizeof(uMagicNumber), &uFileVersion);
	ASSERT(uFileVersion >= 1);

	// Skip submesh data
	uint32 uSubmeshCount;
	dataFile.Read(sizeof(uSubmeshCount), &uSubmeshCount);
	for (uint32 i = 0; i < uSubmeshCount; i++)
	{
		uint32 uDiscard;
		dataFile.Read(sizeof(uDiscard), &uDiscard);
		dataFile.Read(sizeof(uDiscard), &uDiscard);
	}

	ALIGNED_VAR(CollisionQuadTreeHeader, FILE_READ_ALIGN, header);
	dataFile.Read(sizeof(CollisionQuadTreeHeader), (void*)(&header));

	const Vector3f vWorldMin = worldBounds.GetMin();
	const Vector3f vWorldMax = worldBounds.GetMax();

	m_pRoot = vnew(ALLOC_COLLISION) Node(vWorldMin, vWorldMax, NULL);
	m_uTriangles = header.uTriangles;
	m_uVertices = header.uVertices;

	ASSERT(header.uVertices < 65535);

	m_pTriangles = (TriangleIndices*)mem::Alloc(MEMTYPE_STANDARD, ALLOC_COLLISION, sizeof(TriangleIndices)*header.uTriangles, FILE_READ_ALIGN);
	m_pAdjacentTriangles = (TriangleIndices*)mem::Alloc(MEMTYPE_STANDARD, ALLOC_COLLISION, sizeof(TriangleIndices)*header.uTriangles, FILE_READ_ALIGN);
	m_pVertices = (Vector3f*)mem::Alloc(MEMTYPE_STANDARD, ALLOC_COLLISION, sizeof(Vector3f)*header.uVertices, FILE_READ_ALIGN);
	m_pVertexNormals = (Vector3f*)mem::Alloc(MEMTYPE_STANDARD, ALLOC_COLLISION, sizeof(Vector3f)*header.uVertices, FILE_READ_ALIGN);
	m_pTriangleNormals = (Vector3f*)mem::Alloc(MEMTYPE_STANDARD, ALLOC_COLLISION, sizeof(Vector3f)*header.uTriangles, FILE_READ_ALIGN);
	m_pMaterialIndices = (uint32*)mem::Alloc(MEMTYPE_STANDARD, ALLOC_COLLISION, sizeof(uint32) * header.uTriangles, FILE_READ_ALIGN);

	m_pMaterialAttributes = (MaterialAttribute*)mem::Alloc(MEMTYPE_STANDARD, ALLOC_COLLISION, sizeof(MaterialAttribute) * header.uMaterials, FILE_READ_ALIGN);
	m_pMaterialFlags = (uint8*)mem::Alloc(MEMTYPE_STANDARD, ALLOC_COLLISION, sizeof(uint8) * header.uMaterials, FILE_READ_ALIGN);
	m_pMaterialNameHash = (uint32*)mem::Alloc(MEMTYPE_STANDARD, ALLOC_COLLISION, sizeof(uint32) * header.uMaterials, FILE_READ_ALIGN);

	dataFile.Read(sizeof(m_pTriangles[0]) * header.uTriangles, (void*)m_pTriangles);
	dataFile.Read(sizeof(m_pAdjacentTriangles[0]) * header.uTriangles, (void*)m_pAdjacentTriangles);
	dataFile.Read(sizeof(m_pVertices[0]) * header.uVertices, (void*)m_pVertices);
	dataFile.Read(sizeof(m_pVertexNormals[0]) * header.uVertices, (void*)m_pVertexNormals);
	dataFile.Read(sizeof(m_pTriangleNormals[0]) * header.uTriangles, (void*)m_pTriangleNormals);

	dataFile.Read(sizeof(m_pMaterialIndices[0]) * header.uTriangles, (void*)m_pMaterialIndices);
	dataFile.Read(sizeof(m_pMaterialAttributes[0]) * header.uMaterials, (void*)m_pMaterialAttributes);


	for (uint32 m = 0; m < header.uMaterials; m++)
	{
		uint32 flags = 0;

		if (strstr(m_pMaterialAttributes[m].name, "_wall_"))
			flags |= MF_WALL;

		if (strstr(m_pMaterialAttributes[m].name, "nocollision"))
			flags |= MF_NOCOLLISION;

		if (strstr(m_pMaterialAttributes[m].name, "_water_"))
			flags |= MF_WATER;

		if (strstr(m_pMaterialAttributes[m].name, "_nofire_"))
			flags |= MF_NOFIRE;



		if (strstr(m_pMaterialAttributes[m].name, "_hard_"))
			flags |= MF_HARD | MF_GROUND;

		if (strstr(m_pMaterialAttributes[m].name, "_fast_"))
			flags |= MF_FAST | MF_GROUND;

		if (strstr(m_pMaterialAttributes[m].name, "_slow_"))
			flags |= MF_SLOW | MF_GROUND;

		if (flags == 0)
			flags |= MF_GROUND;

		m_pMaterialFlags[m] = flags;

		m_pMaterialNameHash[m] = utl::CRC32(m_pMaterialAttributes[m].name);
	}

	for (uint32 t = 0; t < m_uTriangles; t++)
	{
		CheckInsert(m_pRoot, t, 0);
	}

	DEBUG_PRINT("\nCreated QuadTree with %d nodes\n", m_uNumNodes);

	m_pLastFound = NULL;
	UpdateBoundingBoxes(m_pRoot);
}

// Computes the square distance between a point p and an AABB b
inline float SqDistPointAABB(const Vector3f &p, const Vector3f &min, const Vector3f &max)
{
	float sqDist = 0.0f;
	for (int i = 0; i < 3; i++)
	{
		// For each axis count any excess distance outside box extents
		float v = p[i];
		if (v < min[i]) sqDist += (min[i] - v) * (min[i] - v);
		if (v > max[i]) sqDist += (v - max[i]) * (v - max[i]);
	}
	return sqDist;
}

bool	CollisionQuadTree::Node::InBounds(const Vector3f &min, const Vector3f &max) const
{
	if (GetMax().x < min.x || GetMin().x > max.x) return false;
	if (GetMax().y < min.y || GetMin().y > max.y) return false;
	if (GetMax().z < min.z || GetMin().z > max.z) return false;
	
	return true;
}

bool	CollisionQuadTree::Node::InBounds(const Vector3f &p) const
{
	return  (p.y >= GetMin().y && p.y <= GetMax().y && p.x >= GetMin().x && p.x <= GetMax().x && p.z >= GetMin().z && p.z <= GetMax().z);
}
	
bool CollisionQuadTree::Node::InBounds(const Vector3f &p, float r) const
{
	return SqDistPointAABB(p, GetMin(), GetMax()) <= r * r;	
}

bool CollisionQuadTree::InBounds(const Vector3f &p)
{
	return  m_pRoot->InBounds(p);
}

bool CollisionQuadTree::InBounds(const Vector3f &p, float r)
{
	return  m_pRoot->InBounds(p,r);
}

bool CollisionQuadTree::InBounds(const Vector3f &min, const Vector3f &max)
{
	return  m_pRoot->InBounds(min,max);
}
	

const CollisionQuadTree::Node *CollisionQuadTree::FindNode(const Vector3f &p)
{
	const Node *n = FindNode(m_pRoot,p);

	if (n)
	{
		m_pLastFound = n;
	}

	return n;
}




const CollisionQuadTree::Node *CollisionQuadTree::FindNode(const Node* pNode, const Vector3f &p)
{
	if (p.x < pNode->GetMin().x || p.x > pNode->GetMax().x || p.z < pNode->GetMin().z || p.z > pNode->GetMax().z)
		return NULL;

	if (!pNode->GetChild(0))
		return pNode;

	for(int i=0; i<4; i++)
	{
		const Node *pRet = FindNode(pNode->GetChild(i),p);
		if (pRet)
		{
			return pRet;
		}
	}
	return NULL;
}


uint32 CollisionQuadTree::FindNodes(const Node* pNode, const Vector3f& vPoint, float fRadius, const Node** ppOut, uint32 uMax, uint32 uCnt)
{
	if (vPoint.x < (pNode->GetMin().x-fRadius) || vPoint.x > (pNode->GetMax().x+fRadius) || vPoint.z < (pNode->GetMin().z-fRadius) || vPoint.z > (pNode->GetMax().z + fRadius))
		return uCnt;

	if (!pNode->GetChild(0))
	{
		if (uCnt < uMax)
		{
			ppOut[uCnt++] = pNode;
		}
		return uCnt;
	}

	for(int i=0; i<4; i++)
	{
		uCnt = FindNodes(pNode->GetChild(i), vPoint, fRadius, ppOut, uMax, uCnt);
	}

	return uCnt;
}



void CollisionQuadTree::Dump(const Node* pNode, uint32 level, int child=0)
{
#ifdef DEBUG_BUILD
	char str[64];

	uint32 i;
	for(i=0; i<level; i++)
	{
		str[i] = ' ';
	}
	str[i] = 0;


	DEBUG_PRINT("%s %d(%d) : %d indices (%f,%f)\n", str, level, child, pNode->GetNumIndices(), pNode->GetMin().y, pNode->GetMax().y);

	if (pNode->GetChild(0))
	{
		for(i=0; i<4; i++)
		{
			if (pNode->GetChild(i)->IsBranch())
				Dump(pNode->GetChild(i),level+1,i);
		}
	}

#endif
}

uint32 CollisionQuadTree::GetClipCodes(const Vector3f& p, const Vector3f& min, const Vector3f& max) const
{
	uint32 clip=0;

	if (p.x > max.x) 
		clip |= 1<<0;
	else if (p.x < min.x)
		clip |= 1<<1;
		
	if (p.z > max.z) 
		clip |= 1<<2;
	else if (p.z < min.z)
		clip |= 1<<3;

	if (p.y > max.y)
		clip |= 1 << 4;
	else if (p.y < min.y)
		clip |= 1 << 5;

	return clip;
}


uint32 CollisionQuadTree::GetClipCodesXZ(const Vector3f& p, const Vector3f& min, const Vector3f& max) const
{
	uint32 clip = 0;

	if (p.x > max.x)
		clip |= 1 << 0;
	else if (p.x < min.x)
		clip |= 1 << 1;

	if (p.z > max.z)
		clip |= 1 << 2;
	else if (p.z < min.z)
		clip |= 1 << 3;


	return clip;
}
bool CollisionQuadTree::InBoxXZ(const Vector3f &p1,const Vector3f &p2,const Vector3f &p3, const AABB& aabb)
{
	int clip;

	Vector3f vMin = aabb.GetMin();
	Vector3f vMax = aabb.GetMax();

	clip = GetClipCodesXZ(p1,vMin,vMax);
	clip &= GetClipCodesXZ(p2,vMin,vMax);
	clip &= GetClipCodesXZ(p3,vMin,vMax);

	return clip==0;
}


bool CollisionQuadTree::InBox(const Vector3f &p1, const Vector3f &p2, const Vector3f &p3, const AABB& aabb)
{
	int clip;

	Vector3f vMin = aabb.GetMin();
	Vector3f vMax = aabb.GetMax();

	clip = GetClipCodes(p1, vMin, vMax);
	clip &= GetClipCodes(p2, vMin, vMax);
	clip &= GetClipCodes(p3, vMin, vMax);

	return clip == 0;
}


void CollisionQuadTree::UpdateBoundingBoxes(CollisionQuadTree::Node *pNode)
{
	if (pNode->GetChild(0))
	{
		for (int i = 0; i < 4; i++)
			if (pNode->GetChild(i)->IsBranch())
				UpdateBoundingBoxes(pNode->GetChild(i));
	}
	else
	{
		for (uint32 i = 0; i < pNode->GetNumIndices(); i++)
		{
			int index = pNode->GetIndex(i);

			const TriangleIndices* tris = &m_pTriangles[index];

			const Vector3f& p1 = m_pVertices[tris->i1];
			const Vector3f& p2 = m_pVertices[tris->i2];
			const Vector3f& p3 = m_pVertices[tris->i3];

			Node *p = pNode;

			while (p)
			{
				Vector3f vMin = p->GetAABB().GetMin();
				Vector3f vMax = p->GetAABB().GetMax();

				if (p1.y < vMin.y)
					vMin.y = p1.y;
				if (p1.y > vMax.y)
					vMax.y = p1.y;
				if (p2.y < vMin.y)
					vMin.y = p2.y;
				if (p2.y > vMax.y)
					vMax.y = p2.y;
				if (p3.y < vMin.y)
					vMin.y = p3.y;
				if (p3.y > vMax.y)
					vMax.y = p3.y;

				p->GetAABB().SetMinMax(vMin, vMax);

				p = p->GetParent();
			}

		}

	}
}



int CollisionQuadTree::CheckInsert(CollisionQuadTree::Node* pNode, uint32 index, uint32 depth)
{
	if (m_pMaterialFlags[m_pMaterialIndices[index]] & MF_NOCOLLISION)
		return 0;

	const int MAX_DEPTH = 8;
	const int MAX_TRIS_PER_NODE = 30;

	const TriangleIndices* tris = &m_pTriangles[index];

	const Vector3f& p1 = m_pVertices[tris->i1];
	const Vector3f& p2 = m_pVertices[tris->i2];
	const Vector3f& p3 = m_pVertices[tris->i3];

	const AABB& aabb = pNode->GetAABB();

	if ( InBoxXZ( p1, p2, p3, aabb) )
	{
		if ( (!pNode->HasLeaves() && pNode->GetNumIndices() < MAX_TRIS_PER_NODE) || depth >= MAX_DEPTH)
		{
			pNode->AddIndex(index);
			return 1;
		}
		else
		{

			if (!pNode->GetChild(0))
			{
				pNode->CreateChildren();

				m_uNumNodes += 4;

				for (uint32 i = 0; i < pNode->GetNumIndices(); i++)
				{
					int num = 0;
					for (uint32 j = 0; j < 4; j++)
						num += CheckInsert(pNode->GetChild(j), pNode->GetIndex(i), depth + 1);

					ASSERT(num > 0);
				}

				pNode->FlushIndices();
			}

			int num = 0;
			for(uint32 i=0; i<4; i++)
				num += CheckInsert(pNode->GetChild(i), index, depth+1);
			
			ASSERT(num > 0);

			return num;
		}
	}

	return 0;
}

bool CollisionQuadTree::InTriangle(const Vector3f &P, const Vector3f &A, const Vector3f &B, const Vector3f &C) const
{
	Vector3f v0 = C - A;
	Vector3f v1 = B - A;
	Vector3f v2 = P - A;

	float dot00 = DotProduct(v0, v0);
	float dot01 = DotProduct(v0, v1);
	float dot02 = DotProduct(v0, v2);
	float dot11 = DotProduct(v1, v1);
	float dot12 = DotProduct(v1, v2);

	float invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
	float u;
	float v;
	return ((u = (dot11 * dot02 - dot01 * dot12) * invDenom) >= 0.0f) && ((v = (dot00 * dot12 - dot01 * dot02) * invDenom) >= 0.0f) && (u + v < 1.0f);
}

static float TriArea(const Vector3f &p1, const Vector3f &p2, const Vector3f &p3)
{
	
	float a = p1.GetDistanceFrom(p2);
	float b = p1.GetDistanceFrom(p3);
	float c = p2.GetDistanceFrom(p3);
	
	float s = (a+b+c) / 2.0f;
	
	float d = s*(s-a)*(s-b)*(s-c);
	
	if (d > 0)
		return sqrtf(d);
	else
		return 0;
}



float	CollisionQuadTree::getDistanceQT(const Vector3f &pos, CollisionMeshHitResult *hit, float radius)
{
	float closestD = radius*2.0f;
	
	if (InBounds(pos))
	{
		const Node *nodes[64];
		
		int cnt = FindNodes(m_pRoot, pos, radius, nodes, 64, 0);
		
		for(int n=0; n<cnt; n++)
		{
			const Node *node = nodes[n];
			
			
			if ((pos.y >= (node->GetMin().y-radius)) && (pos.y <= (node->GetMax().y+radius)))
			{
				
				for(uint32 i=0; i<node->GetNumIndices(); i++)
				{
					
					int index = node->GetIndex(i);
					
					
					const TriangleIndices *tris = &m_pTriangles[index];
					
					
					const Vector3f &p1 = (const Vector3f &)(m_pVertices[tris->i1]);
					const Vector3f &p2 = (const Vector3f &)(m_pVertices[tris->i2]);
					const Vector3f &p3 = (const Vector3f &)(m_pVertices[tris->i3]);
					
					Vector3f norm = (const Vector3f &)(m_pTriangleNormals[index]);
					
					Vector3f rp = pos - p1;
					
					float d = DotProduct(rp,norm);
					
					
					Vector3f tip = pos - (norm * d);
					
					if (InTriangle(tip,p1,p2,p3))
					{
						if (fabsf(d) < fabsf(closestD))
						{
							closestD = d;
							
							hit->uMaterialNameHash = m_pMaterialNameHash[m_pMaterialIndices[index]];
							hit->pMaterialName = m_pMaterialAttributes[m_pMaterialIndices[index]].name;
							hit->uMaterialFlags = m_pMaterialFlags[m_pMaterialIndices[index]];
							hit->vIntersectPoint = tip;
							hit->vNorm = norm;
							hit->uTriIndex = index;
							hit->vPoints[0] = p1;
							hit->vPoints[1] = p2;
							hit->vPoints[2] = p3;

						}
						
					}
				}
			}
		}
	}
	
	
	return closestD;
	
}
	

int	CollisionQuadTree::ClipSphere(const Node* pNode, const Vector3f &pos, float radius, CollisionMeshHitResult *pHits, uint32 uMaxHits, uint32 uNumHits, uint32 uIncludeFlags) const
{
	if (!pNode->InBounds(pos, radius))
		return uNumHits;

	if (pNode->GetChild(0))
	{
		for (int i = 0; i<4; i++)
		{
			if (pNode->GetChild(i)->IsBranch())
			{
				uNumHits = ClipSphere(pNode->GetChild(i), pos, radius, pHits, uMaxHits, uNumHits, uIncludeFlags);
				if (uNumHits == uMaxHits)
				{
					return uNumHits;
				}
			}
		}
	}
	else
	{
		/*
		AABB::BoxCorners corners;

		pNode->GetAABB().GetCorners(corners);

		for (int i = 0; i < 8; i++)
			DebugRender::GetRenderer()->AddSphere(corners.verts[i], 0.5, Color(0.3f, 0.3f, 1.0f, 0.8f));
			*/

		const float fRadiusSquared = radius * radius;
		const uint32 uIndexCount = pNode->GetNumIndices();
		for (uint32 i = 0; i < uIndexCount; i++)
		{
			int index = pNode->GetIndex(i);
			
			if (!(m_pMaterialFlags[m_pMaterialIndices[index]] & uIncludeFlags))
				continue;

			const TriangleIndices* tris = &m_pTriangles[index];
			const Vector3f &p1 = m_pVertices[tris->i1];
			const Vector3f &p2 = m_pVertices[tris->i2];
			const Vector3f &p3 = m_pVertices[tris->i3];

			Vector3f closest = ClosestPointOnTriangleToPoint(pos, p1, p2, p3);
			const float distanceSquared = closest.GetSquaredDistanceFrom(pos);
			if (distanceSquared <= fRadiusSquared)
			{
				const float fDistance = sqrtf(distanceSquared);
				CollisionMeshHitResult& chit = pHits[uNumHits];

				chit.vNorm.x = (pos.x - closest.x) /fDistance;
				chit.vNorm.y = (pos.y - closest.y) / fDistance;
				chit.vNorm.z = (pos.z - closest.z) / fDistance;

				const Vector3f &vTriangleNormal = m_pTriangleNormals[index];
				if (DotProduct(chit.vNorm, vTriangleNormal) < 0)
				{
					chit.vNorm.x *= -1;
					chit.vNorm.y *= -1;
					chit.vNorm.z *= -1;
				}

				chit.uMaterialNameHash = m_pMaterialNameHash[m_pMaterialIndices[index]];
				chit.pMaterialName = m_pMaterialAttributes[m_pMaterialIndices[index]].name;
				chit.uMaterialFlags = m_pMaterialFlags[m_pMaterialIndices[index]];

				chit.vPoints[0] = p1;
				chit.vPoints[1] = p2;
				chit.vPoints[2] = p3;

				chit.vIndices[0] = tris->i1;
				chit.vIndices[1] = tris->i2;
				chit.vIndices[2] = tris->i3;

				chit.vIntersectPoint = closest;

				chit.fDistance = fDistance - radius;

				chit.uTriIndex = index;

				chit.bHit = true;
				chit.pNode = pNode;

				uNumHits++;
				if (uNumHits == uMaxHits)
				{
					return uNumHits;
				}
			}
		}
	}	
	return uNumHits;	
}


	
bool	CollisionQuadTree::ClipLine(const Vector3f &vFrom, const Vector3f &vTo, CollisionMeshHitResult& hit, uint32 uMaxHits) const
{
	Vector3f vDir = vTo - vFrom;
	
	float fClosest = vDir.Magnitude();
	
	vDir *= 1.0f / fClosest;
	
	const Node* pRoot = GetRootNode();
	if (ClipLine(pRoot, vFrom, vTo, vDir, fClosest, false, hit, MF_ALL, uMaxHits) < fClosest)
	{
		//DEBUG_PRINT("hit %f,%f,%f\n",hit.vIntersectPoint.x,hit.vIntersectPoint.y,hit.vIntersectPoint.z);
		//DEBUG_PRINT("to %f,%f,%f\n",vTo.x,vTo.y,vTo.z);
		//DEBUG_PRINT("dir %f,%f,%f\n",vDir.x,vDir.y,vDir.z);
		
		return true;
	}
	
	return false;
}


float		CollisionQuadTree::ClipLine(const Node* pNode, const Vector3f &vFrom, const Vector3f &vTo, const Vector3f &vDir, float fClosest, bool bSmooth, CollisionMeshHitResult& pHit, uint32 uIncludeFlags, uint32 uMaxHits) const
{
	CollisionMeshHitResult hits[MaxHits];
	
	uint32 uNumHits;
	
	if (pHit.bHit)
	{
		uNumHits = ClipLine((Node*)pHit.pNode, vFrom, vTo, vDir, fClosest, bSmooth, hits, uMaxHits, 0, uIncludeFlags);
		if (uNumHits==0)
			uNumHits = ClipLine(pNode, vFrom, vTo, vDir, fClosest, bSmooth, hits, uMaxHits, 0, uIncludeFlags);
	}
	else
		uNumHits = ClipLine(pNode, vFrom, vTo, vDir, fClosest, bSmooth, hits, uMaxHits, 0, uIncludeFlags);



	if (uNumHits>0)
	{
		pHit = hits[0];

		for(uint32 i=1; i<uNumHits; i++)
		{
			if (hits[i].fDistance < pHit.fDistance)
			{
				pHit = hits[i];
			}
		}
		
		return pHit.fDistance;
	}else
		return fClosest;
}

void	CollisionQuadTree::SmoothNormal(CollisionMeshHitResult &hit)
{
	float areaT = TriArea(hit.vPoints[0],hit.vPoints[1],hit.vPoints[2]);
	if (areaT > 0.0001f)
	{
		float areaB = TriArea(hit.vPoints[0],hit.vIntersectPoint,hit.vPoints[2]);
		float areaC = TriArea(hit.vPoints[0],hit.vIntersectPoint,hit.vPoints[1]);
		float areaA = areaT - areaB - areaC;
		
		float c1 = areaA / areaT;
		float c2 = areaB / areaT;
		float c3 = areaC / areaT;
		
		const TriangleIndices* tris = &m_pTriangles[hit.uTriIndex];
		
		Vector3f snorm = m_pVertexNormals[tris->i1] * c1 + m_pVertexNormals[tris->i2] * c2 + m_pVertexNormals[tris->i3] * c3;
		
		if (snorm.TryNormalise())
			hit.vNorm = snorm;
		else
			printf("TRI NORM FAIL! %f,%f,%f,%f\n",areaT,areaA,areaB,areaC);
	}else
		printf("CANNOT SMOOTH NORMAL\n");
	
}



int	CollisionQuadTree::ClipLine(const Node* pNode, const Vector3f &vFrom, const Vector3f &vTo, const Vector3f &vDir, float fClosest, bool bSmooth, CollisionMeshHitResult *pHits, uint32 uMaxHits,  uint32 uNumHits, uint32 uIncludeFlags) const
{
	const Vector3f& vMin = pNode->GetAABB().GetMin();
	const Vector3f& vMax = pNode->GetAABB().GetMax();
	uint32 uClip = GetClipCodes(vFrom, vMin, vMax);
	uClip		&= GetClipCodes(vTo, vMin, vMax);

	if( uClip == 0 )
	{
		if (pNode->GetChild(0))
		{
			for(int i=0; i<4; i++)
			{
				if (pNode->GetChild(i)->IsBranch())
				{
					const Node* pChild = pNode->GetChild(i);
					uNumHits = ClipLine(pChild, vFrom, vTo, vDir, fClosest, bSmooth, pHits, uMaxHits, uNumHits, uIncludeFlags);
					if (uNumHits >= uMaxHits)
					{
						return uNumHits;
					}
				}
			}
		}
		else
		{
			
			/*
			AABB::BoxCorners corners;

			pNode->GetAABB().GetCorners(corners);

			for (int i = 0; i < 8; i++)
				DebugRender::GetRenderer()->AddSphere(corners.verts[i], 0.5, Color(0.3f, 0.3f, 1.0f, 0.8f));
				*/

			for(uint32 i=0; i<pNode->GetNumIndices(); i++)
			{
				int index = pNode->GetIndex(i);

				if (!(m_pMaterialFlags[m_pMaterialIndices[index]] & uIncludeFlags))
					continue;

				const TriangleIndices* tris = &m_pTriangles[index];

				const Vector3f &p1 = m_pVertices[tris->i1];
				const Vector3f &p2 = m_pVertices[tris->i2];
				const Vector3f &p3 = m_pVertices[tris->i3];

				const Vector3f &norm = m_pTriangleNormals[index];
				

				float Vd = DotProduct(norm, vDir);

				if (Vd < 0.0f)
				{
					float D = -DotProduct(norm, p1);

					float Vo = -(DotProduct(norm, vFrom)+D);

					float t = Vo / Vd;

					if (t >= 0.0f && t < fClosest)
					{
						Vector3f tip = vFrom+(vDir*t);

						if (InTriangle(tip,p1,p2,p3))
						{						
							//addMarker(tip);

							CollisionMeshHitResult chit;
							
							if (bSmooth && !(m_pMaterialFlags[m_pMaterialIndices[index]] & MF_HARD))
							{
								float areaT = TriArea(p1,p2,p3);
								if (areaT > 0.0001f)
								{
									float areaB = TriArea(p1,tip,p3);
									float areaC = TriArea(p1,tip,p2);
									float areaA = areaT - areaB - areaC;
									
									float c1 = areaA / areaT;
									float c2 = areaB / areaT;
									float c3 = areaC / areaT;
									
									chit.vNorm = m_pVertexNormals[tris->i1] * c1 + m_pVertexNormals[tris->i2] * c2 + m_pVertexNormals[tris->i3] * c3;
									
									if (!chit.vNorm.TryNormalise())
									{
										printf("TRI NORM FAIL! %f,%f,%f,%f\n",areaT,areaA,areaB,areaC);
										chit.vNorm = norm;
									}

								}else
									chit.vNorm = norm;
								
							}else
							{
								chit.vNorm = norm;
							}
							
							chit.uMaterialNameHash = m_pMaterialNameHash[m_pMaterialIndices[index]];
							chit.pMaterialName = m_pMaterialAttributes[m_pMaterialIndices[index]].name;
							chit.uMaterialFlags = m_pMaterialFlags[m_pMaterialIndices[index]];

							
							chit.vPoints[0] = p1;
							chit.vPoints[1] = p2;
							chit.vPoints[2] = p3;
							
							chit.vIndices[0] = tris->i1;
							chit.vIndices[1] = tris->i2;
							chit.vIndices[2] = tris->i3;
							
							chit.vIntersectPoint = tip;
							
							chit.fDistance = t;
							
							chit.uTriIndex = index;
							
							chit.bHit = true;
							chit.pNode = pNode;
							
							if (uNumHits < uMaxHits)
							{
								pHits[uNumHits++] = chit;
							}							
							if (uNumHits == uMaxHits)
							{
								return uNumHits;
							}
						}
					}
				}
			}
		}
	}

	return uNumHits;
}


uint32 CollisionQuadTree::searchAdjTriangleRecursively(
	uint32 triangleIndices[], uint32 triangleNum, const uint32 triangleMax,
	const uint32 tgtIndex, const uint32 rootIndex, const Sphere& sphere ) const
{
	const Vector3f* pVertexStream = m_pVertices;
	const TriangleIndices* pIndexStream = m_pTriangles;
	const TriangleIndices* pAdjacencyStream = m_pAdjacentTriangles;

	for( int triNo = 0; triNo < 3; ++triNo ) {
		uint32 adjTriangle = ( &pAdjacencyStream[tgtIndex].i1 )[triNo];
		if( adjTriangle == UINT_MAX || adjTriangle == rootIndex ) {
			continue; // Skip
		}

		// Check already added?
		bool added = false;
		for( uint32 addedTriNo = 0; addedTriNo < triangleNum; ++addedTriNo ) {
			if( triangleIndices[addedTriNo] == adjTriangle ) {
				added = true;
				break;
			}
		}
		if( added ) { continue; } // Skip

		float dp = DotProduct( m_pTriangleNormals[adjTriangle], m_pTriangleNormals[rootIndex] );
		if( dp < 0.5f/*cos 60deg*/ ) { continue; } // Skip

		// TODO: use more accurate method as necessary
		const TriangleIndices& triIndices = pIndexStream[adjTriangle];
		Vector3f vClosest;
		bool bInside = TestSphereTriangle(sphere, pVertexStream[triIndices.i1], pVertexStream[triIndices.i2], pVertexStream[triIndices.i3], vClosest);
		

		// Add if it's inside
		if( bInside ) {
			ASSERT( triangleNum < triangleMax );
			triangleIndices[triangleNum] = adjTriangle;
			triangleNum += 1;

			if( triangleNum < triangleMax ) {
				// still going
				triangleNum = searchAdjTriangleRecursively( triangleIndices, triangleNum, triangleMax,
												adjTriangle, rootIndex, sphere );
			}

			if( triangleNum >= triangleMax ) {
				// container is full. dismiss.
				break;
			}
		}
	}
	return triangleNum;
}

uint32 CollisionQuadTree::setupGroundPatchIndices( uint16 indices[], uint32 indicesMax, float radius, const CollisionMeshHitResult& hitResult ) const
{
	ASSERT( indicesMax % 3 == 0 ); // must be a triangle
	const uint32 triangleMax = indicesMax / 3;

	uint32 rootIndex = hitResult.uTriIndex;
	Sphere boundingSphere;
	boundingSphere.SetPos( hitResult.vIntersectPoint );
	boundingSphere.SetRadius( radius );

	uint32* triangleIndices;
	ScratchObj<uint32> scratchIndices( triangleIndices, triangleMax );

	triangleIndices[0] = rootIndex;
	uint32 triangleNum = 1;
	triangleNum = searchAdjTriangleRecursively( triangleIndices, triangleNum, triangleMax, rootIndex, rootIndex, boundingSphere );

	const TriangleIndices* pIndexStream = m_pTriangles;
	for( uint32 i = 0; i < triangleNum; ++i ) {
		indices[i * 3] = pIndexStream[triangleIndices[i]].i1;
		indices[i * 3 + 1] = pIndexStream[triangleIndices[i]].i2;
		indices[i * 3 + 2] = pIndexStream[triangleIndices[i]].i3;
	}

	return triangleNum * 3;
}

void CollisionQuadTree::initVertexBuffer( GFXDevice* pDevice, VertexBuffer& vertexBuffer ) const
{
	vertexBuffer.Init( pDevice, m_pVertices, sizeof(PositionVertex), m_uVertices, "CollisionQuadTree_Position" );
}

#ifdef DRAW_VISIBLE_MODEL
void CollisionQuadTree::SetupVisibleModel( GFXDevice* pDevice, Scene* pScene )
{
	ASSERT( pDevice != NULL && pScene != NULL );

	PositionVertex* pVertexStream = reinterpret_cast<PositionVertex*>( const_cast<Vector3f*>( m_pVertices ) );
	//m_VisibleMesh.InitVertexBuffer( GetStandardDeclarationId( VT_POSITION ), pVertexStream, m_uVertices );
	

	m_VisibleMesh.GetVertexBuffer().Init(pDevice, GetStandardDeclarationId(VT_POSITION), pVertexStream, m_uVertices, "CollisionQuadTree_visualize" );
	uint32* pIndexStream = reinterpret_cast<uint32*>( const_cast<TriangleIndices*>( m_pTriangles ) );
	m_VisibleMesh.GetIndexBuffer().Init(pDevice, pIndexStream, m_uTriangles * 3, PT_TRIANGLES, true);
	m_pVisibleEffect = ResourceMgr::Inst()->GetEffectBinding(pDevice, "solid", GetStandardDeclarationId( VT_POSITION ));


	DepthStencilStateDecl depthDecl;
	depthDecl.bDepthWrite	= true;
	depthDecl.bDepthEnable	= true;
	depthDecl.eDepthFunc	= DEPTH_TEST_LEQUAL;
	depthDecl.bStencilEnable= true;
	depthDecl.ePassOp		= STENCIL_OP_REPLACE;
	depthDecl.eStencilTest	= STENCIL_TEST_ALWAYS;

	//AlphaStateDecl alphaDecl;
	//alphaDecl.bWire = true;
	//alphaDecl.SetDeferredTarget(true);

	//StencilRefMask stencilRef(0x0, STENCIL_MASK_GEOMETRY, STENCIL_GEOMETRY, 0x0, STENCIL_MASK_GEOMETRY, STENCIL_GEOMETRY);

	RasterizerStateDecl rasterizerDecl;
	rasterizerDecl.bWireframe = true;

	Material &mat = m_VisibleMesh.GetMaterial();
	mat.Init(pDevice, m_pVisibleEffect);
	mat.SetDepthStencil(pDevice->GetDepthStencilState(&depthDecl));
	//mat.SetStencilRefMask(stencilRef);
	//mat.SetAlpha(pDevice->GetAlphaState(&alphaDecl));
	mat.SetRasterizer( pDevice->GetRasterizerState( &rasterizerDecl ) );

	m_pVisibleTransform = pScene->CreateTransformNode();
	m_pVisibleRenderGroup = pScene->CreateRenderGroup(m_pVisibleTransform);

	RenderNode* pNode = &m_VisibleMesh;
	m_pVisibleRenderGroup->AddRenderNodes( &pNode, 1, 0 );
	//pNode->SetLayer( RenderLayer::LAYER_MODELS_OPAQUE );
	pNode->SetPriority(0);

	Matrix4x4 matTmp;
	matTmp.LoadIdentity();
	m_pVisibleTransform->SetMatrix( matTmp );
	Sphere boundingSphere;
	boundingSphere.SetPos( Vector3f( 0.0f, 0.0f, 0.0f ) );
	boundingSphere.SetRadius( 10000.0f );
	m_pVisibleTransform->SetBoundingSphere( boundingSphere );
}
#endif


CollisionQuadTree::Node::Node(const Vector3f& vMin, const Vector3f& vMax, Node *parent)
{
	m_boundingBox.SetMinMax(vMin, vMax);

	m_uIndexCnt = 0;

	m_pParent = parent;

	m_bHasLeaves = false;

	m_depth = parent == NULL ? 0 : parent->GetDepth() + 1;

	for(int i=0; i<4; i++)
	{
		m_pChildren[i] = NULL;
	}

}


float CollisionQuadTree::GetMaxHeightAt(float x, float z, Vector3f* pNormOut) const
{
	return GetMaxHeightAt(m_pRoot, x, z, pNormOut);
}


float CollisionQuadTree::GetMaxHeightAt(const Node* pNode, float x, float z, Vector3f* pNormOut) const
{
	const Vector3f vPos(x,pNode->GetAABB().GetPos().y,z);
	int clip = GetClipCodes(vPos, pNode->GetMin(), pNode->GetMax());

	float maxHeight = -100000.0;

	if (clip==0)
	{
		//if ((from.y >= node->min.y || to.y >= node->min.y) && (from.y <= node->max.y || to.y <= node->max.y))
		{
			if (pNode->GetChild(0))
			{
				for(int i=0; i<4; i++)
				{
					if (pNode->GetChild(i)->IsBranch())
					{
						float h = GetMaxHeightAt(pNode->GetChild(i), x, z, pNormOut);
						if (h > maxHeight)
							return h;
					}
				}
			}else
			{
				for(uint32 i=0; i<pNode->GetNumIndices(); i++)
				{
					int index = pNode->GetIndex(i);


					const TriangleIndices*tris = &m_pTriangles[index];

					const Vector3f &p1 = (const Vector3f&)(m_pVertices[tris->i1]);
					const Vector3f &p2 = (const Vector3f&)(m_pVertices[tris->i2]);
					const Vector3f &p3 = (const Vector3f&)(m_pVertices[tris->i3]);

					const Vector3f &norm = (const Vector3f&)(m_pTriangleNormals[index]);

					float D = -DotProduct(norm,p1);

					Vector3f tip;

					tip.x = x;
					tip.y = (norm.x*x + norm.z*z + D) / -norm.y;
					tip.z = z;


					if (tip.y > maxHeight)
					{							
						if (InTriangle(tip,p1,p2,p3))
						{						
							maxHeight = tip.y;

							if(pNormOut)					
								*pNormOut = norm;
						}

					}
				}

			}
		}
	}

	return maxHeight;
}

bool CollisionQuadTree::FindLocation(float fDotCmpVal, const Vector3f& vPos, float radius, Vector3f& vOut, uint32 uNumChecks)
{
	Vector3f vNormal(0.0f, 1.0f, 0.0f);
	Vector3f vUp(0.0f, 1.0f, 0.0f);
	Vector3f dir;
    
	for (uint32 i = 0; i < uNumChecks; i++)
	{
		float fDist = Math::RangedRandom(-radius, radius);
		Vector3f vDir;
		vDir = Vector3f::RandomRange(Vector3f(0.00001f, 0.0f, 0.00001f), Vector3f(100.0f, 0.0f, 100.0f));
		vDir.Normalise();
		vDir *= fDist;
		vOut = vPos + vDir;
		
		// Ok we have 2D position, how go to the height map to find out where to plonk it
		vOut.y = this->GetMaxHeightAt(vOut.x, vOut.z, &vNormal);
		if( DotProduct(vNormal, vUp) >= fDotCmpVal )
		{
			return true;	// This ground level is fine
		}
	}
    
	return false;
}
	
	
float distanceFrom(const Vector3f &min, const Vector3f &max, const Vector3f &point)
{
	Vector3f cpos;
	
	cpos.AssignMax(min, point);
	cpos.AssignMin(max, cpos);
	
	return (cpos - point).Magnitude();
}

	
	
	
	
#define FINDMINMAX(x0,x1,x2,min,max) \
min = max = x0;   \
if(x1<min) min=x1;\
if(x1>max) max=x1;\
if(x2<min) min=x2;\
if(x2>max) max=x2;
	
int planeBoxOverlap(Vector3f normal,float d, Vector3f maxbox)
{
	int q;
	Vector3f vmin,vmax;
	for(q=0;q<=2;q++)
	{
  if(normal[q]>0.0f)
  {
	  vmin[q]=-maxbox[q];
	  vmax[q]=maxbox[q];
  }
  else
  {
	  vmin[q]=maxbox[q];
	  vmax[q]=-maxbox[q];
  }
	}
	if(DotProduct(normal,vmin)+d>0.0f) return 0;
	if(DotProduct(normal,vmax)+d>=0.0f) return 1;
	
	return 0;
}


}


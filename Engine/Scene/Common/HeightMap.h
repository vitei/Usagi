/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Simple terrain heightmap
*****************************************************************************/
#ifndef _USG_GRAPHICS_SCENE_HEIGHTMAP_H_
#define _USG_GRAPHICS_SCENE_HEIGHTMAP_H_

#include "Engine/Maths/Matrix4x4.h"
#include "Engine/Scene/TransformNode.h"
#include "Engine/Scene/Common/Mesh.h"
#include "Engine/Graphics/Materials/Material.h"

namespace usg{

class Camera;
class CollisionQuadTree;
class Scene;

class HeightMap
{
public:
	HeightMap(void);
	~HeightMap(void);

	bool Load(GFXDevice* pDevice, Scene* pScene, ResourceMgr* pResMgr, const char* szFileName, uint32 uWidth, uint32 uHeight, uint32 uBpp, float scale, float offset, CollisionQuadTree& pCollisionQuadTree);

private:
	enum
	{
		TERRAIN_TEX_COUNT = 3
	};

	Mesh                    m_mesh;
	TransformNode*			m_pTransformNode;
	RenderGroup*			m_pRenderGroup;

	float					m_fScale;
	float					m_fOffset;
};

}

#endif

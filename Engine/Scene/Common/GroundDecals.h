/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_ENGINE_SCENE_GROUND_DECALS_
#define _USG_ENGINE_SCENE_GROUND_DECALS_

#include "Engine/Graphics/Primitives/VertexBuffer.h"
#include "Engine/Graphics/Primitives/IndexBuffer.h"
#include "Engine/Scene/RenderNode.h"
#include "Engine/Physics/CollisionQuadTree.h"
#include "Decal.h"

namespace usg {

class GroundDecals
{
public:
	static const uint32 DECALS_NUM = 32;
	static const uint32 DAMAGE_TEXTURE_TYPES_NUM = 5;
	static const uint32 MAX_SPECIAL_TEXTURES = 5;

	GroundDecals(uint32 uShadowDecals);
	~GroundDecals();

	void Init( GFXDevice* pDevice, Scene* pScene, CollisionQuadTree* p, const char* szShadowTexName );
	void AddDecal(GFXDevice* pDevice, const Vector3f& vFrom, const Vector3f& vTo, float fScale = 1.0f, uint32 uType = 0);
	void AddDecal( GFXDevice* pDevice, const CollisionMeshHitResult& hit, float fScale = 1.0f, uint32 uType = 0 );
	void Update( float fDelta );
	void UpdateBuffers(GFXDevice* pDevice);
	void AddSpecialDecalTexture(usg::GFXDevice* pDevice, uint32 uType, const char* szTexName);
	const VertexBuffer* GetVertexBuffer() { return &m_vertexBuffer; }

	Decal* GetShadowDecal(const char* szTexName);
	void FreeShadowDecal(Decal* pDecal);

private:

	struct DamageDecal
	{
		float		fExisitingTime;
		Decal		decal;
	};

	void UpdateDecal( DamageDecal& decal, float fDelta );
	TextureHndl			m_pStandardTex[DAMAGE_TEXTURE_TYPES_NUM];
	TextureHndl			m_pSpecialTex[MAX_SPECIAL_TEXTURES];

	GFXDevice*				m_pDevice;
	Scene*					m_pScene;
	CollisionQuadTree*		m_pQuadTree;

	TransformNode*			m_pTransform;
	RenderGroup*			m_pRenderGroup;
	
	DamageDecal				m_decals[DECALS_NUM];
	uint32					m_decalsCount;
	VertexBuffer			m_vertexBuffer;

	FastPool<Decal>			m_shadowDecals;

	sint32					m_cameraRollDeg;
};

namespace Components
{
struct GroundDecalsHandle
{
	GroundDecals* pGroundDecals;
};
}

}

#endif

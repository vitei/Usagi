/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Resource/ResourceMgr.h"
#include "GroundDecals.h"
#include "Engine/Framework/SystemCoordinator.h"

namespace usg {

	template<>
	void RegisterComponent<usg::Components::GroundDecalsHandle>(SystemCoordinator& systemCoordinator)
	{
		systemCoordinator.RegisterComponent<::usg::Components::GroundDecalsHandle>();
	}

static const float LIFE_TIME_SEC = 10;
static const float DISSOLVE_TIME_SEC = 5;
static const float TOTAL_LIFE =  LIFE_TIME_SEC + DISSOLVE_TIME_SEC;

const char* g_szDecalNames[GroundDecals::DAMAGE_TEXTURE_TYPES_NUM] =
{
	"fx_damage00",
	"fx_damage01",
	"fx_damage02",
	"fx_damage03",
	"fx_damage04",
};

GroundDecals::GroundDecals(uint32 uShadowDecals) :
	m_shadowDecals(36, false)
{
	m_pDevice = NULL;
	m_pQuadTree = NULL;
	m_decalsCount = 0;
}

GroundDecals::~GroundDecals()
{

}

void GroundDecals::Init( GFXDevice* pDevice, Scene* pScene, CollisionQuadTree* p, const char* szShadowTexName )
{
	ASSERT_MSG( pDevice != NULL && p != NULL, "nullpo" );
	m_pDevice = pDevice;
	m_pScene = pScene;
	m_pQuadTree = p;

	m_pQuadTree->initVertexBuffer( pDevice, m_vertexBuffer );

	for (int i = 0; i < DAMAGE_TEXTURE_TYPES_NUM; i++)
	{
		m_pStandardTex[i] = ResourceMgr::Inst()->GetTexture(m_pDevice, g_szDecalNames[i]);
	}

	for(int i=0; i<DECALS_NUM; i++)
	{
		// Don't initially set a texture as we're going to assign one randomly
		uint32 damageTextureIndex = i % DAMAGE_TEXTURE_TYPES_NUM;
		m_decals[i].decal.Init(pDevice, pScene, m_pStandardTex[damageTextureIndex], 30);
		m_decals[i].decal.SetPriority(126-(uint8)i);	// Draw before shadows, consistent ordering
		m_decals[i].fExisitingTime = TOTAL_LIFE;
	}

	for (FastPool<Decal>::Iterator it = m_shadowDecals.EmptyBegin(); !it.IsEnd(); ++it)
	{
		// FIXME: The decal shadow texture shouldn't be set here
		(*it)->Init(pDevice, pScene, usg::ResourceMgr::Inst()->GetTexture(pDevice, szShadowTexName), 20);
	}
}


void GroundDecals::AddSpecialDecalTexture(usg::GFXDevice* pDevice, uint32 uType, const char* szTexName)
{
	ASSERT(uType > 0 && uType <= MAX_SPECIAL_TEXTURES);
	m_pSpecialTex[uType-1] = usg::ResourceMgr::Inst()->GetTexture(pDevice, szTexName);
}

Decal* GroundDecals::GetShadowDecal(const char* szTexName)
{
	Decal* pDecal = m_shadowDecals.Alloc();
	if (pDecal && szTexName[0] != '0')
	{
		TextureHndl pTexture = usg::ResourceMgr::Inst()->GetTexture(m_pDevice, szTexName);
		pDecal->SetTexture(m_pDevice, pTexture);
	}
	return pDecal;
}
void GroundDecals::FreeShadowDecal(Decal* pDecal)
{
	pDecal->RemoveFromScene(m_pScene);
	m_shadowDecals.Free(pDecal);
}

void GroundDecals::AddDecal( GFXDevice* pDevice, const CollisionMeshHitResult& hit, float fScale, uint32 uType )
{
	TextureHndl pTexture = NULL;
	if (uType == 0)
	{
		pTexture = m_pStandardTex[rand() % DAMAGE_TEXTURE_TYPES_NUM];
	}
	else
	{
		ASSERT(uType <= MAX_SPECIAL_TEXTURES);
		pTexture = m_pSpecialTex[uType - 1];
		if (!pTexture)
			ASSERT(false);
	}
	ASSERT( m_decalsCount < DECALS_NUM );
	Decal& decal = m_decals[m_decalsCount].decal;

	// Initialize values
	m_decals[m_decalsCount].fExisitingTime = 0.0f;

	// Index Buffer
	const uint32 TRIANGLES_NUM = 30;
	const uint32 indicesMax = TRIANGLES_NUM * 3;
	uint16 indices[indicesMax];

	float radius = 8.0f * fScale;
	uint32 indicesNum = m_pQuadTree->setupGroundPatchIndices( indices, indicesMax, radius, hit );

	for( int i = indicesNum; i < indicesMax; ++i ) {
		indices[i] = indices[indicesNum];
	}

	Sphere boundingSphere;
	boundingSphere.SetPos( hit.vIntersectPoint );
	boundingSphere.SetRadius( radius );

	//====================
	float32 fNear = 0.01f;
	float32 fFar = 10.0f;
	Matrix4x4 projMatrix, viewMatrix;

	projMatrix.Orthographic(-4.0f*fScale, 4.0f*fScale, -4.0f*fScale, 4.0f*fScale, fNear, fFar);

	// calc the camera cooridate
	Vector3f eye = hit.vIntersectPoint;
	Vector3f cam = eye + hit.vNorm;
	Vector3f viewDir = eye - cam;
	viewDir.Normalise();
	Vector3f vUp( 0.0f, 1.0f, 0.0f );

	// roll the camera
	Matrix4x4 rotMatrix;
	rotMatrix.LoadIdentity();
	rotMatrix.MakeRotAboutAxis( Vector4f( viewDir, 0.0f ), Math::DegToRad( (float)m_cameraRollDeg ) );
	m_cameraRollDeg = ( m_cameraRollDeg + 45 ) % 360;
	vUp = vUp * rotMatrix;

	Vector3f vRight = CrossProduct( vUp, viewDir );
	vRight = vRight.GetNormalisedIfNZero(Vector3f::X_AXIS);
	vUp = CrossProduct( vRight, viewDir );

	viewMatrix.CameraMatrix(
		Vector4f( vRight, 0.0f ),
		Vector4f( vUp, 0.0f ),
		Vector4f( viewDir, 0.0f ),
		Vector4f( cam, 0.0f )
		);

	
	// Init the renderable decal
	decal.AddToScene(pDevice, m_pScene);
	decal.SetContents(pDevice, &boundingSphere, &m_vertexBuffer, indices, indicesNum);
	decal.SetMatrix(projMatrix, viewMatrix);
	decal.SetTexture(m_pDevice, pTexture);
	decal.SetOpacity( 1.0f );

	m_decalsCount = ( ( m_decalsCount + 1 ) % DECALS_NUM );
}

void GroundDecals::AddDecal( GFXDevice* pDevice, const Vector3f& vFrom, const Vector3f& vTo, float fScale, uint32 uType )
{
	usg::Vector3f vDir;
	vDir = vTo - vFrom;
	float magn = vDir.Magnitude();
	vDir.Normalise();
	usg::CollisionMeshHitResult hitResult;
	float dist = m_pQuadTree->ClipLine(m_pQuadTree->GetRootNode(), vFrom, vTo, vDir, magn, false, hitResult, CollisionQuadTree::MF_ALL & ~(CollisionQuadTree::MF_WATER|CollisionQuadTree::MF_NOCOLLISION));

	if(dist < magn)
	{
		AddDecal( pDevice, hitResult, fScale, uType );
	}
	
}

void GroundDecals::Update( float fDelta )
{
	for( uint32 i = 0; i < DECALS_NUM; ++i )
	{
		if (m_decals[i].decal.IsVisible())
		{
			UpdateDecal(m_decals[i], fDelta);
		}
	}
}

void GroundDecals::UpdateBuffers(GFXDevice* pDevice)
{
	for (uint32 i = 0; i < DECALS_NUM; ++i)
	{
		if (m_decals[i].decal.IsVisible())
		{
			m_decals[i].decal.UpdateBuffers(pDevice);
		}
	}

	for (FastPool<Decal>::Iterator it = m_shadowDecals.Begin(); !it.IsEnd(); ++it)
	{
		(*it)->UpdateBuffers(pDevice);
	}
}


void GroundDecals::UpdateDecal( DamageDecal& decal, float fDelta )
{
	if( decal.fExisitingTime > TOTAL_LIFE  )
	{
		return;
	}

	decal.fExisitingTime += fDelta;

	if(decal.fExisitingTime > TOTAL_LIFE)
	{
		decal.decal.RemoveFromScene(m_pScene);
	}
	else
	{
		const float lifeTimeMS = LIFE_TIME_SEC * 1000.f;
		const float dissolveTimeMS = DISSOLVE_TIME_SEC * 1000.f;
		const float time = ( decal.fExisitingTime * 1000.0f ) - lifeTimeMS;
		if( time >= 0.0f )
		{
			float opacity = 1.0f - Math::Min( time / dissolveTimeMS, 1.0f );

			decal.decal.SetOpacity( opacity );
		}
	}
}



}
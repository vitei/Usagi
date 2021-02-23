/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Core/Utility.h"
#include "Engine/Core/File/File.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Memory/Mem.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/Resource/ModelResourceMesh.h"
#include "Engine/Graphics/Device/GFXContext.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Scene/Model/Bone.h"
#include "Engine/Scene/Model/ModelRenderNodes.h"
#include "Model.h"

//#define MODEL_DEBUG
namespace usg {

Model::Model()
{
	m_pTransformNode	= NULL;
	m_pSkeleton			= NULL;
	m_meshArray			= NULL;
	m_pOverrideMaterials = NULL;
	m_pRenderGroup		= NULL;
	m_pScene			= NULL;
	m_uMeshCount		= 0;
	m_fScale			= 1.0f;
	m_bDynamic			= false;
	m_bVisible			= false;
	m_bDepthPassMeshEnabled = false;
	m_pResource			= NULL;
	m_bFade				= false;
	m_fAlpha			= 1.0f;
	m_bShouldBeVisible	= true;
	m_depthRenderMask	= RenderMask::RENDER_MASK_NONE;
}


Model::~Model()
{
	SetInUse(false);
	AddToScene(false);
	if(m_pSkeleton)
	{
		vdelete m_pSkeleton;
		m_pSkeleton = NULL;
	}
	if(m_pOverrideMaterials)
	{
		vdelete[] m_pOverrideMaterials;
		m_pOverrideMaterials = NULL;
	}
	for (uint32 i = 0; i < m_uMeshCount; i++)
	{
		vdelete m_meshArray[i];
		vdelete m_depthMeshArray[i];
		m_meshArray[i] = NULL;
		m_depthMeshArray[i] = NULL;
	}
	if(m_meshArray)
	{
		vdelete[] m_meshArray;
		vdelete[] m_depthMeshArray;
		m_meshArray = NULL;
		m_depthMeshArray = NULL;
	}
}

Model::Model(GFXDevice* pDevice, Scene* pScene, ResourceMgr* pResMgr, const char* szFileName, bool bDynamic)
{
	bool bLoad = Load(pDevice, pScene, pResMgr, szFileName, bDynamic);
	ASSERT(bLoad);
}

bool Model::Load( GFXDevice* pDevice, Scene* pScene, ResourceMgr* pResMgr, const char* szFileName, bool bDynamic, bool bFastMem, bool bAutoTransform, bool bPerBoneCulling)
{
	uint32 uRenderMask = RenderMask::RENDER_MASK_ALL;
	// TODO: Support cleanup or reuse?
	ASSERT(m_pScene == NULL);

	ASSERT(pDevice && pScene);

	m_pScene	= pScene;
	// Relying on each instance of a model having it's own constant sets to override
	m_bDynamic	= bDynamic;
	m_bPerBoneCulling = bPerBoneCulling;

#if defined(DEBUG_BUILD) && defined(MODEL_DEBUG)
	DEBUG_PRINT( "Model::Load, %s\n", szFileName );
#endif
	m_pResource = pResMgr->GetModel( pDevice, szFileName, bFastMem );

	ASSERT(!m_pResource->IsInstanceModel());

	m_uMeshCount = m_pResource->GetMeshCount();
	m_meshArray = vnew(usg::ALLOC_GEOMETRY_DATA) RenderMesh*[m_pResource->GetMeshCount()];
	m_depthMeshArray = vnew(usg::ALLOC_GEOMETRY_DATA) RenderMesh*[m_pResource->GetMeshCount()];
	for (uint32 i = 0; i < m_pResource->GetMeshCount(); i++)
	{
		// TODO: Add depth specific variants of the render nodes
		const ModelResource::Mesh* pMesh = m_pResource->GetMesh(i);
		m_meshArray[i] = vnew(usg::ALLOC_GEOMETRY_DATA) RenderMesh;
		m_depthMeshArray[i] = vnew(usg::ALLOC_GEOMETRY_DATA) RenderMesh;
	}

	m_pSkeleton = vnew(usg::ALLOC_GEOMETRY_DATA) Skeleton;
	m_pSkeleton->Load(pDevice, pScene, this, bAutoTransform);
	
	if(m_bDynamic)
	{
		m_pOverrideMaterials = vnew(ALLOC_GEOMETRY_DATA) MaterialInfo[m_pResource->GetMeshCount()];
	}

	// Set up the constant sets for the bones
	if (!m_pResource->GetRigidBoneIndices().empty())
	{
		// Even through the bound constant buffer needs to have a constant size set, none of the API's thus far
		// seem to take issue with passing in a smaller size constant set
		const ShaderConstantDecl boneDecl[] =
		{
			{ "mBones", CT_MATRIX_43, (uint32)m_pResource->GetRigidBoneIndices().size(), 0, 0, NULL },
			SHADER_CONSTANT_END()
		};

		m_staticBones.Init(pDevice, boneDecl);
	}

	if (!m_pResource->GetSmoothBoneIndices().empty())
	{
		// Even through the bound constant buffer needs to have a constant size set, none of the API's thus far
		// seem to take issue with passing in a smaller size constant set
		const ShaderConstantDecl boneDecl[] =
		{
			{ "mBones", CT_MATRIX_43, (uint32)m_pResource->GetSmoothBoneIndices().size(), 0, 0, NULL },
			SHADER_CONSTANT_END()
		};

		m_skinnedBones.Init(pDevice, boneDecl);
	}

	for(uint32 i=0; i<m_pResource->GetMeshCount(); i++)
	{
		const ModelResource::Mesh* pMesh = m_pResource->GetMesh(i);
		m_meshArray[i]->Init(pDevice, pScene, pMesh, this, false);

		m_depthMeshArray[i]->Init(pDevice, pScene, pMesh, this, true);

		InitDynamics(pDevice, pScene, i);

	}

	SetRenderMask(uRenderMask);

	SetInUse(true);
	AddToScene(true);

	return true;
}


void Model::Cleanup(GFXDevice* pDevice)
{
	m_skinnedBones.Cleanup(pDevice);
	m_staticBones.Cleanup(pDevice);
	if (m_pSkeleton)
	{
		m_pSkeleton->Cleanup(pDevice);
	}

	for (uint32 i = 0; i < m_uMeshCount; i++)
	{
		m_meshArray[i]->Cleanup(pDevice);
		m_depthMeshArray[i]->Cleanup(pDevice);
	}

	if (m_pOverrideMaterials)
	{
		for (uint32 uMesh = 0; uMesh < m_pResource->GetMeshCount(); uMesh++)
		{
			m_pOverrideMaterials[uMesh].customFX.Cleanup(pDevice);
		}
	}

}

void Model::SetInUse(bool bInUse)
{
	if (!bInUse && m_pTransformNode)
	{
		ASSERT(!m_bVisible);
		m_pSkeleton->SetInUse(m_pScene, this, false);
		m_pScene->DeleteTransformNode(m_pTransformNode);
		m_pTransformNode = NULL;
	}
	else if (bInUse && !m_pTransformNode)
	{
		m_pTransformNode = m_pScene->CreateTransformNode();
		m_pTransformNode->SetBoundingSphere(m_pResource->GetBounds());
		m_pSkeleton->SetInUse(m_pScene, this, true);
	}
}


void Model::RemoveOverrides(GFXDevice* pDevice)
{
	SetFade(false, 1.0f);
	for (uint32 i = 0; i < m_pResource->GetMeshCount(); i++)
	{
		const ModelResource::Mesh* pMesh = m_pResource->GetMesh(i);
		DescriptorSet& descSet = m_meshArray[i]->GetDescriptorSet();
		// FIXME: Use the correct indicies
		descSet.SetConstantSetAtBinding(SHADER_CONSTANT_MATERIAL, pMesh->renderSets[0].effectRuntime.GetConstantSet(0), 0, SHADER_FLAG_VERTEX);
		descSet.SetConstantSetAtBinding(SHADER_CONSTANT_MATERIAL_1, pMesh->renderSets[0].effectRuntime.GetConstantSet(1), 0, SHADER_FLAG_PIXEL);
		for (uint32 i = 0; i < ModelResource::Mesh::MAX_UV_STAGES; i++)
		{
			if (pMesh->pTextures[i])
			{
				descSet.SetImageSamplerPairAtBinding(i, pMesh->pTextures[i], pMesh->samplers[i]);
			}
		}
		descSet.UpdateDescriptors(pDevice);
		for (uint32 uTex = 0; uTex < ModelResource::Mesh::MAX_UV_STAGES; uTex++)
		{
			TextureHndl pOrigTex = pMesh->pTextures[uTex];
			SamplerHndl origSamp = pMesh->samplers[uTex];
			if (pOrigTex.get() != NULL)
			{
				descSet.SetImageSamplerPairAtBinding(uTex, pOrigTex, origSamp);
			}
		}
		usg::Color white(1.0f, 1.0f, 1.0f, 1.0f);
		m_meshArray[i]->SetBlendColor(white);
		//m_meshArray[i]->SetPipelineState(pDevice->GetPipelineState(m_pScene->GetRenderPasses(0).GetRenderPass(*m_meshArray[i]), pMesh->pipelines.defaultPipeline));
		m_fScale = 1.0f;
		m_meshArray[i]->ResetOverrides();
		m_meshArray[i]->GetDescriptorSet().UpdateDescriptors(pDevice);
		if (m_depthMeshArray)
		{
			DescriptorSet& depthDesc = m_depthMeshArray[i]->GetDescriptorSet();
			depthDesc.SetConstantSetAtBinding(SHADER_CONSTANT_MATERIAL, pMesh->renderSets[0].effectRuntime.GetConstantSet(0), 0, SHADER_FLAG_VERTEX);
			m_depthMeshArray[i]->ResetOverrides();
			m_depthMeshArray[i]->GetDescriptorSet().UpdateDescriptors(pDevice);
		}
	}
}

void Model::SetRenderMask(uint32 uMask)
{
	m_uRenderMask = uMask;
	m_depthRenderMask &= (uMask|RenderMask::RENDER_MASK_SHADOW_CAST);
	UpdateRenderMaskInt();
}

void Model::UpdateRenderMaskInt()
{
	for (uint32 i = 0; i < m_pResource->GetMeshCount(); i++)
	{
		m_meshArray[i]->SetRenderMask(m_uRenderMask);
		m_depthMeshArray[i]->SetRenderMaskWithShadowCheck(m_depthRenderMask);
	}
}

void Model::EnableShadow(GFXDevice* pDevice, bool bEnable)
{
	if (bEnable)
	{
		m_depthRenderMask |= RenderMask::RENDER_MASK_SHADOW_CAST;
	}
	else
	{
		m_depthRenderMask &= ~RenderMask::RENDER_MASK_SHADOW_CAST;
	}

	UpdateRenderMaskInt();
}

void Model::SetDynamic(GFXDevice* pDevice, bool bDynamic)
{
	if (bDynamic == m_bDynamic)
		return;

	if (m_bDynamic && !bDynamic)
	{
		// Do nothing, just leave it dynamic
		//vdelete[] m_pOverrideMaterials;
		//m_pOverrideMaterials = NULL;
	}
	else if (!m_bDynamic && bDynamic)
	{
		m_bDynamic = true;
		m_pOverrideMaterials = vnew(ALLOC_GEOMETRY_DATA) MaterialInfo[m_pResource->GetMeshCount()];

		for (uint32 i = 0; i < m_pResource->GetMeshCount(); i++)
		{
			InitDynamics(pDevice, m_pScene, i);
		}
	}
}

void Model::InitDynamics(GFXDevice* pDevice, Scene* pScene, uint32 i)
{
	const ModelResource::Mesh* pMesh = m_pResource->GetMesh(i);
	//ASSERT(m_bDynamic);
	// Below this point is for dynamic overrides
	if (m_bDynamic)
	{
		// Copy the override constant sets (only doing material, not lighting atm)
		m_pOverrideMaterials[i].customFX.Init(pDevice, &pMesh->renderSets[0].effectRuntime);
		m_pOverrideMaterials[i].customFX.GPUUpdate(pDevice);

		// Add the UV sets to the model for updating of the texture matrices
		for (uint32 uUVSet = 0; uUVSet < pMesh->uUVCount; uUVSet++)
		{
			// FIXME: Remove the inheritance as all these allocations are wasteful
			const ModelResource::TextureCoordInfo* pUVInfo = &pMesh->uvMapping[uUVSet];
			UVMapper* pMapper = m_meshArray[i]->GetUVMapper(uUVSet);
			pMapper->Init(uUVSet, pUVInfo->method, &m_pOverrideMaterials[i].customFX, pUVInfo->translate, pUVInfo->scale, pUVInfo->rotate);
			pMapper->Update();
		}

		m_meshArray[i]->SetOverrideConstant(0, m_pOverrideMaterials[i].customFX.GetConstantSet(0));
		m_meshArray[i]->SetOverrideConstant(1, m_pOverrideMaterials[i].customFX.GetConstantSet(1));

		if (m_depthMeshArray)
		{
			m_depthMeshArray[i]->SetOverrideConstant(0, m_pOverrideMaterials[i].customFX.GetConstantSet(0));
		}
	}
}

bool Model::IsOnScreen()
{
	return true;

	// FIXME: Re-implemenet
#if 0
	if(m_pRenderGroup)
	{
		return m_pRenderGroup->DrawnLastFrame();
	}
	
	return false;
#endif
}

void Model::AddToSceneInt(GFXDevice* pDevice)
{
	Skeleton& skeleton = GetSkeleton();
	bool bVisible = m_fAlpha > Math::EPSILON && m_bShouldBeVisible;
	bool bAddDepth = m_bShouldBeVisible && (m_depthRenderMask != RenderMask::RENDER_MASK_NONE);

	if ((bVisible || bAddDepth) && !m_bPerBoneCulling && !m_pRenderGroup)
	{
		m_pRenderGroup = m_pScene->CreateRenderGroup(m_pTransformNode);
		m_pRenderGroup->UseVisibilityUpdate(true);
	}

	if (bVisible != m_bVisible)
	{
		if (!bVisible)// && m_pRenderGroup)
		{
			for (uint32 i = 0; i < m_pResource->GetMeshCount(); i++)
			{
				const ModelResource::Mesh* pMesh = m_pResource->GetMesh(i);
				RenderNode* pNode = m_meshArray[i];
				if (m_bPerBoneCulling)
				{
					Bone* pBone = skeleton.GetBone(pMesh->primitive.uRootIndex);
					pBone->RemoveRenderNode(m_pScene, pNode);
				}
				else
				{
					m_pRenderGroup->RemoveRenderNode(pNode);
				}
			}
		}
		else if (bVisible && !m_bVisible)
		{

			uint8 uMaxLod = 0;
			{
				for (uint32 i = 0; i < m_pResource->GetMeshCount(); i++)
				{
					const ModelResource::Mesh* pMesh = m_pResource->GetMesh(i);

					RenderNode* pNode = m_meshArray[i];
					if (m_bPerBoneCulling)
					{
						Bone* pBone = skeleton.GetBone(pMesh->primitive.uRootIndex);
						pBone->AttachRenderNode(pDevice, m_pScene, pNode, m_meshArray[i]->GetLod(), m_bDynamic);
					}
					else
					{
						m_pRenderGroup->AddRenderNode(pDevice, pNode, m_meshArray[i]->GetLod());
					}
					uMaxLod = Math::Max(m_meshArray[i]->GetLod(), uMaxLod);
				}

				if (uMaxLod > 0)
				{
					if (m_bPerBoneCulling)
					{
						for (uint32 uBone = 0; uBone < skeleton.GetBoneCount(); uBone++)
						{
							Bone* pBone = skeleton.GetBone(uBone);
							RenderGroup* pGroup = pBone->GetRenderGroup();
							if (!pGroup)
								continue;

							for (uint32 i = 1; i <= uMaxLod; i++)
							{
								float fLODDistance = Math::Max(m_pResource->GetBounds().GetRadius(), 3.0f)*7.5f;

								pGroup->SetLodMaxDistance(i - 1, fLODDistance*i);
							}
						}
					}
					else
					{
						for (uint32 i = 1; i <= uMaxLod; i++)
						{
							float fLODDistance = Math::Max(m_pResource->GetBounds().GetRadius(), 3.5f)*7.5f;

							m_pRenderGroup->SetLodMaxDistance(i - 1, fLODDistance*i);
						}
					}
				}
			}

			// FIXME: Replace with artist defines
			if (uMaxLod == 0 && m_pResource->GetBounds().GetRadius() < 2.f)
			{
				for (uint32 i = 0; i < m_pSkeleton->GetBoneCount(); i++)
				{

					Bone* pBone = skeleton.GetBone(i);
					float fLODDistance = Math::Max(m_pResource->GetBounds().GetRadius(), 6.0f)*80.0f;
					if (pBone->GetRenderGroup())
					{
						pBone->GetRenderGroup()->SetLodMaxDistance(0, fLODDistance);
					}
				}

			}

		}
	}

	AddDepthMesh(pDevice, bAddDepth);

	if (!m_bPerBoneCulling && m_pRenderGroup && m_pRenderGroup->IsEmpty())
	{
		m_pScene->DeleteRenderGroup(m_pRenderGroup);
		m_pRenderGroup = NULL;
	}

	m_bVisible = bVisible;
}

void Model::AddToScene(bool bVisible)
{
	m_bShouldBeVisible = bVisible;
}

void Model::ForceRemoveFromScene()
{
	m_bShouldBeVisible = false;
	AddToSceneInt(nullptr);
}

Model::RenderMesh* Model::GetRenderMesh(const char* szMaterialName)
{
	U8String matNameU8(szMaterialName);
	//ASSERT(m_bDynamic);
	for(uint32 uMesh=0; uMesh<m_pResource->GetMeshCount(); uMesh++)
	{
		if(m_pResource->GetMesh(uMesh)->matName == matNameU8)
		{
			return m_meshArray[uMesh];
		}
	}
	return NULL;
}

const Model::RenderMesh* Model::GetRenderMesh(uint32 uMeshId) const
{
	ASSERT(uMeshId < m_pResource->GetMeshCount());

	return m_meshArray[uMeshId];

}

Model::RenderMesh* Model::GetRenderMesh(uint32 uMeshId)
{
	ASSERT(uMeshId < m_pResource->GetMeshCount());

	return m_meshArray[uMeshId];

}



// FIXME: Setting the matrices directly for now, but what we really want to be doing is having a texture co-ordinator management system
void Model::SetTextureTranslate(const char* szName, uint32 uTexId, float fX, float fY, IdentifierType eNameType)
{
	ASSERT(m_bDynamic);
	U8String matNameU8(szName);

	for(uint32 uMesh=0; uMesh<m_pResource->GetMeshCount(); uMesh++)
	{
		const ModelResource::Mesh* pMesh = m_pResource->GetMesh(uMesh);
		const U8String &cmpName = eNameType == IDENTIFIER_MESH ? pMesh->name : pMesh->matName;
		if(matNameU8.Length() == 0 || cmpName == matNameU8)
		{
			UVMapper* pMapper = m_meshArray[uMesh]->GetUVMapper(uTexId);
			if(pMapper)
			{
				Vector2f vTrans(fX, fY);
				pMapper->SetUVTranslation(vTrans);
			}
			//break;
		}
	}
}

void Model::SetScale(float fScale)
{
	if (fScale == m_fScale)
		return;

	ASSERT(m_bDynamic);
	if (!m_bDynamic)
		return;


	for(uint32 uMesh=0; uMesh<m_pResource->GetMeshCount(); uMesh++)
	{
		m_meshArray[uMesh]->SetScale(fScale, m_pOverrideMaterials[uMesh].customFX);
		m_meshArray[uMesh]->RequestOverride(0);
		if (m_depthMeshArray)
		{
			m_depthMeshArray[uMesh]->RequestOverride(0);
		}
	}
	m_fScale = fScale;
}

uint32 Model::GetMeshIndex(const char* szName, IdentifierType eNameType) const
{
	ASSERT(m_bDynamic);

	for(uint32 uMesh=0; uMesh<m_pResource->GetMeshCount(); uMesh++)
	{
		const ModelResource::Mesh* pMesh = m_pResource->GetMesh(uMesh);
		const U8String &cmpName = eNameType == IDENTIFIER_MESH ? pMesh->name : pMesh->matName;
		if(cmpName == szName)
		{
			return uMesh;
		}
	}
	return USG_INVALID_ID;
}


void Model::AddTextureTranslate(const char* szName, uint32 uTexId, float fX, float fY, IdentifierType eNameType)
{
	ASSERT(m_bDynamic);
	U8String matNameU8(szName);

	for(uint32 uMesh=0; uMesh<m_pResource->GetMeshCount(); uMesh++)
	{
		const ModelResource::Mesh* pMesh = m_pResource->GetMesh(uMesh);
		const U8String &cmpName = eNameType == IDENTIFIER_MESH ? pMesh->name : pMesh->matName;
		if(matNameU8.Length() == 0 || cmpName == matNameU8)
		{
			UVMapper* pMapper = m_meshArray[uMesh]->GetUVMapper(uTexId);
			if(pMapper)
			{
				Vector2f vTrans(fX, fY);
				pMapper->SetUVTranslation(vTrans + pMapper->GetUVTranslation());
			}
			//break;
		}
	}
}


void Model::SetTextureScale( const char* szName, uint32 uTexId, float fX, float fY, IdentifierType eNameType /*= IDENTIFIER_MATERIAL */ )
{
	ASSERT( m_bDynamic );
	U8String matNameU8( szName );

	for( uint32 uMesh = 0; uMesh < m_pResource->GetMeshCount(); uMesh++ ) {
		const ModelResource::Mesh* pMesh = m_pResource->GetMesh( uMesh );
		const U8String &cmpName = eNameType == IDENTIFIER_MESH ? pMesh->name : pMesh->matName;
		if( matNameU8.Length() == 0 || cmpName == matNameU8 ) {
			UVMapper* pMapper = m_meshArray[uMesh]->GetUVMapper( uTexId );
			if( pMapper ) {
				Vector2f scale( fX, fY );
				pMapper->SetScale( scale );
			}
		}
	}
}

void Model::AddTextureRotation( const char* szName, uint32 uTexId, float fRot, IdentifierType eNameType )
{
	ASSERT(m_bDynamic);
	U8String matNameU8(szName);

	for(uint32 uMesh=0; uMesh<m_pResource->GetMeshCount(); uMesh++)
	{
		const ModelResource::Mesh* pMesh = m_pResource->GetMesh(uMesh);
		const U8String &cmpName = eNameType == IDENTIFIER_MESH ? pMesh->name : pMesh->matName;
		if(matNameU8.Length() == 0 || cmpName == matNameU8)
		{
			UVMapper* pMapper = m_meshArray[uMesh]->GetUVMapper(uTexId);
			if(pMapper)
			{
				pMapper->SetUVRotation(fRot + pMapper->GetUVRotation());
			}
			//break;
		}
	}
}


bool Model::OverrideTexture(const char* szTextureName, TextureHndl pOverrideTex)
{
	ASSERT(m_bDynamic);
	ASSERT(pOverrideTex.get()!=NULL);

	U8String texNameU8(szTextureName);

	bool bFound = false;
	UNUSED_VAR(bFound);
	for(uint32 uMesh=0; uMesh<m_pResource->GetMeshCount(); uMesh++)
	{
		const ModelResource::Mesh* pSrcMesh = m_pResource->GetMesh(uMesh);
		DescriptorSet* pMaterial = &m_meshArray[uMesh]->GetDescriptorSet();
		for(uint32 uTex = 0; uTex < ModelResource::Mesh::MAX_UV_STAGES; uTex++)
		{
			TextureHndl pOrigTex = pSrcMesh->pTextures[uTex];
			if(pOrigTex.get() != NULL)
			{
				SamplerHndl origSamp = pSrcMesh->samplers[uTex];
				U8String cmpName = pOrigTex->GetName();
				cmpName.RemovePath();
				if ((cmpName == texNameU8))
				{
					pMaterial->SetImageSamplerPairAtBinding(uTex, pOverrideTex, origSamp);
					bFound = true;
				}
			}
		}
	}

	return bFound;
}

void Model::GPUUpdate(GFXDevice* pDevice, bool bVisible)
{
	// Should internally check for any relevant changes, drop out if nothing needs changing
	AddToSceneInt(pDevice);

	// Update the bone buffers
	if (bVisible)
	{
		if (m_skinnedBones.IsValid())
		{
			const usg::vector<uint32>& skinnedIndices = m_pResource->GetSmoothBoneIndices();
			Matrix4x3* pMat = (Matrix4x3*)m_skinnedBones.Lock((uint32)(sizeof(Matrix4x3)*skinnedIndices.size()));
			for (uint32 uBone = 0; uBone < skinnedIndices.size(); uBone++)
			{
				const Bone* pBone = m_pSkeleton->GetBone(skinnedIndices[uBone]);
				pMat[uBone] = (pBone->GetInverseBindMatrix() * pBone->GetWorldMatrix());
			}
			m_skinnedBones.Unlock();
			m_skinnedBones.UpdateData(pDevice);

		}

		if (m_staticBones.IsValid())
		{
			const usg::vector<uint32>& rigidIndices = m_pResource->GetRigidBoneIndices();
			Matrix4x3* pMat = (Matrix4x3*)m_staticBones.Lock((uint32)(sizeof(Matrix4x3)*rigidIndices.size()));
			for (uint32 uBone = 0; uBone < rigidIndices.size(); uBone++)
			{
				const Bone* pBone = m_pSkeleton->GetBone(rigidIndices[uBone]);
				pMat[uBone] = (pBone->GetInverseBindMatrix() * pBone->GetWorldMatrix());
			}
			m_staticBones.Unlock();
			m_staticBones.UpdateData(pDevice);
		}
	}
}

void Model::UpdateDescriptors(GFXDevice* pDevice)
{
	for (uint32 uMesh = 0; uMesh < m_pResource->GetMeshCount(); uMesh++)
	{
		DescriptorSet& desc = m_meshArray[uMesh]->GetDescriptorSet();
		desc.UpdateDescriptors(pDevice);
	}
}

void Model::SetTransform(const Matrix4x4 &trans)
{
	m_pTransformNode->SetMatrix( trans );
	//else
		//ASSERT(false);
}


void Model::RestoreDefaultLayer()
{
	for(uint32 i=0; i<m_pResource->GetMeshCount(); i++)
	{
		const ModelResource::Mesh* pMesh = m_pResource->GetMesh(i);
		m_meshArray[i]->SetLayer( pMesh->layer );
	}
}

void Model::SetLayer(usg::RenderLayer uLayer)
{
	for(uint32 i=0; i<m_pResource->GetMeshCount(); i++)
	{
		m_meshArray[i]->SetLayer(uLayer);
	}
}

void Model::SetPriority(uint8 uPriority)
{
	for(uint32 i=0; i<m_pResource->GetMeshCount(); i++)
	{
		m_meshArray[i]->SetPriority(uPriority);
	}
}

void Model::AddDepthMesh(GFXDevice* pDevice, bool bAdd)
{
	if (bAdd == m_bDepthPassMeshEnabled)
		return;

	Skeleton& skeleton = GetSkeleton();
	for (uint32 i = 0; i < m_pResource->GetMeshCount(); i++)
	{
		const ModelResource::Mesh* pMesh = m_pResource->GetMesh(i);
		if(pMesh->bCanFade)
		{
			Bone* pBone = skeleton.GetBone(pMesh->primitive.uRootIndex);
			RenderNode* pDepthNode = m_depthMeshArray[i];
			pDepthNode->SetRenderMaskIncShadow(m_depthRenderMask);
			if (bAdd)
			{
				if (m_bPerBoneCulling)
				{
					pBone->AttachRenderNode(pDevice, m_pScene, pDepthNode, m_meshArray[i]->GetLod(), m_bDynamic);
				}
				else
				{
					m_pRenderGroup->AddRenderNode(pDevice, pDepthNode, m_meshArray[i]->GetLod());
				}
			}
			else if(m_bPerBoneCulling)
			{
				pBone->RemoveRenderNode(m_pScene, pDepthNode);
			}
			else
			{
				m_pRenderGroup->RemoveRenderNode(pDepthNode);
			}
		}
	}

	m_bDepthPassMeshEnabled = bAdd;

}

void Model::SetFade(bool bFade, float fAlpha)
{
	m_fAlpha = bFade ? fAlpha : 1.0f;

	if (bFade && fAlpha >= Math::EPSILON)
	{
		usg::Color value(fAlpha, fAlpha, fAlpha, fAlpha);
		for (uint32 i = 0; i < m_uMeshCount; i++)
		{
			RenderMesh* pNode = GetRenderMesh(i);
			if (pNode)
			{				
				pNode->SetBlendColor(value);
			}
		}
	}

	if (bFade && fAlpha > Math::EPSILON)
	{
		// Add to the main render
		m_depthRenderMask |= (m_uRenderMask);
	}
	else
	{
		// Remove from the main render but not the shadow pass
		m_depthRenderMask &= ~(RenderMask::RENDER_MASK_ALL);
	}
	
	UpdateRenderMaskInt();


	if (m_bFade == bFade)
		return;


	RenderPassHndl renderPass;
	for (uint32 i = 0; i < m_uMeshCount; i++)
	{
		const usg::ModelResource::Mesh* pResMesh = m_pResource->GetMesh(i);
		bool bTranslucent = false;
		if (bFade)
		{
			if (pResMesh->bCanFade)
			{
				bTranslucent = true;
				m_meshArray[i]->SetLayer(usg::RenderLayer::LAYER_TRANSLUCENT);
				m_meshArray[i]->SetPriority(1);
			}
			else
			{
				m_meshArray[i]->SetLayer(usg::RenderLayer::LAYER_TRANSLUCENT);
				m_meshArray[i]->SetPriority(2);
			}		
		}
		else
		{
			m_meshArray[i]->SetLayer(pResMesh->layer);
			m_meshArray[i]->SetPriority(pResMesh->priority);
		}
		// Need to cache these pipelines
		renderPass = m_pScene->GetRenderPasses(0).GetRenderPass(*m_meshArray[i]);
	}

	m_bFade = bFade;
}

UVMapper* Model::GetUVMapper(uint32 uMesh, uint32 uTexIndex)
{
	ASSERT(uMesh < m_pResource->GetMeshCount());
	return m_meshArray[uMesh]->GetUVMapper(uTexIndex);
}

void Model::OverrideVariable(const char* szVarName, void* pData, uint32 uSize, uint32 uIndex)
{
	ASSERT(m_bDynamic);

	for (uint32 uMesh = 0; uMesh < m_pResource->GetMeshCount(); uMesh++)
	{
		if (m_pOverrideMaterials[uMesh].customFX.SetVariable(szVarName, pData, uSize, 0))
		{
			m_meshArray[uMesh]->RequestOverride(1);
		}
	}
}

const U8String& Model::GetName() const
{
	return m_pResource->GetName();
}

uint32 Model::GetMeshCount() const
{ 
	return m_pResource->GetMeshCount();
}

}

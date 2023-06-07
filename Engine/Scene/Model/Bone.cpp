/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Resource/ModelResource.h"
#include "Engine/Resource/SkeletonResource.h"
#include "Engine/Scene/Model/Model.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Graphics/RenderConsts.h"
#include "Bone.h"


namespace usg
{


Bone::Bone()
{
	m_pTransformNode = NULL;
	m_pRenderGroup = NULL;
	m_pParent = NULL;
}

Bone::~Bone()
{

}

void Bone::Init(GFXDevice* pDevice, Bone* pParent, const SkeletonResource::Bone* pBone)
{
	m_pResource = pBone;
	m_pParent = pParent;

	m_boneConstants.Init(pDevice, SceneConsts::g_instanceCBDecl);
}

void Bone::Cleanup(GFXDevice* pDevice)
{
	m_boneConstants.Cleanup(pDevice);
}

void Bone::CreateTransformNode(Scene* pScene, TransformNode* pRootNode)
{
	// If pRootNode is null we are assuming the hierarchy is handled elsewhere

	if(m_pTransformNode)
		return;	// Already initialized

	if(m_pParent)
	{
		m_pParent->CreateTransformNode(pScene, pRootNode);
		m_pTransformNode = pScene->CreateTransformNode();
	}
	else
	{
		m_pTransformNode = pScene->CreateTransformNode();
	}

	m_pTransformNode->SetMatrix(m_pResource->mMatrix);
	m_pTransformNode->SetBoundingSphere( m_pResource->cColSphere );
}


void Bone::UpdateScaleBounds(float fScale)
{
	usg::Sphere sphere = m_pResource->cColSphere;
	sphere.SetRadius(sphere.GetRadius() * fScale);
	// Pos will get transformed by the matrix
	m_pTransformNode->SetBoundingSphere(sphere);
}


void Bone::RemoveTransformNode(Scene* pScene)
{
	if (m_pTransformNode)
	{
		pScene->DeleteTransformNode(m_pTransformNode);
		m_pTransformNode = NULL;
	}
}


Matrix4x4 Bone::GetDefaultMatrix(bool bIncludeParents) const
{
	usg::Matrix4x4 mat = Matrix4x4::Identity();
	const usg::SkeletonResource::Bone* pBone = m_pResource;
	const Bone* pParent = m_pParent;
	while (pBone)
	{
		mat = mat * pBone->mMatrix;
		pBone = pBone->parentIndex != USG_INVALID_ID ? pParent->GetResource() : nullptr;
		pParent = pParent ? pParent->m_pParent : nullptr;
	}
	return mat;
}


void Bone::AttachRenderNode(GFXDevice* pDevice, Scene* pScene, RenderNode* pNode, uint8 uLod, bool bDynamic)
{
	if(!m_pRenderGroup)
	{
		m_pRenderGroup = pScene->CreateRenderGroup(m_pTransformNode);
		m_pRenderGroup->UseVisibilityUpdate(true);
	}
	m_pRenderGroup->AddRenderNode(pDevice, pNode, uLod);
}

void Bone::RemoveRenderNode(Scene* pScene, RenderNode* pNode)
{
	if(!m_pRenderGroup)
		return;

	m_pRenderGroup->RemoveRenderNode(pNode);

	if(m_pRenderGroup->IsEmpty())
	{
		pScene->DeleteRenderGroup(m_pRenderGroup);
		m_pRenderGroup = NULL;
	}
}

void Bone::RemoveRenderNodes(Scene* pScene)
{
	if(m_pRenderGroup)
	{
		pScene->DeleteRenderGroup(m_pRenderGroup);
		m_pRenderGroup = NULL;
	}
}


}
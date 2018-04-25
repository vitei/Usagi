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

void Bone::CleanUp(GFXDevice* pDevice)
{
	m_boneConstants.CleanUp(pDevice);
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


void Bone::RemoveTransformNode(Scene* pScene)
{
	if (m_pTransformNode)
	{
		pScene->DeleteTransformNode(m_pTransformNode);
		m_pTransformNode = NULL;
	}
}


void Bone::AttachRenderNode(Scene* pScene, RenderNode* pNode, uint8 uLod, bool bDynamic)
{
	if(!m_pRenderGroup)
	{
		m_pRenderGroup = pScene->CreateRenderGroup(m_pTransformNode);
		m_pRenderGroup->UseVisibilityUpdate(bDynamic);
	}
	m_pRenderGroup->AddRenderNode(pNode, uLod);
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
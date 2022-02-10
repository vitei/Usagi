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
#include "Skeleton.h"


namespace usg
{

Skeleton::Skeleton()
{
	m_pResource = NULL;
	m_pBones = NULL;
	m_bAutoTransform = false;
	m_bInUse = false;
}

Skeleton::Skeleton(GFXDevice* pDevice, Scene* pScene, Model* pModel, bool bAutoTransform)
{
	m_pBones = NULL;
	Load(pDevice, pScene, pModel, bAutoTransform);
}

Skeleton::~Skeleton()
{
	if(m_pBones)
	{
		vdelete[] m_pBones;
		m_pBones = NULL;
	}
}

bool Skeleton::Load( GFXDevice* pDevice, Scene* pScene, Model* pModel, bool bAutoTransform)
{
	m_pResource = pModel->GetResource()->GetDefaultSkeleton();
	if(m_pResource)
	{
		m_uBoneCount = m_pResource->GetBoneCount();
		ASSERT(m_pBones==NULL);
		m_pBones = vnew(ALLOC_GEOMETRY_DATA) Bone[m_uBoneCount];

		for(uint32 i=0; i<m_uBoneCount; i++)
		{
			Bone* pParent = NULL;
			if(m_pResource->GetBoneByIndex(i)->parentName.length() > 0)
			{
				uint32 uIndex = m_pResource->GetBoneIndex(m_pResource->GetBoneByIndex(i)->parentName.c_str());
				pParent = &m_pBones[uIndex];
			}
			m_pBones[i].Init(pDevice, pParent, m_pResource->GetBoneByIndex(i));
		}
	}

	return true;
}

void Skeleton::Cleanup(GFXDevice* pDevice)
{
	for (uint32 i = 0; i < m_uBoneCount; i++)
	{
		m_pBones[i].Cleanup(pDevice);
	}
}

void Skeleton::SetInUse(Scene* pScene, Model* pModel, bool bInUse)
{
	if (bInUse && !m_bInUse)
	{
		for (uint32 i = 0; i < m_uBoneCount; i++)
		{
			m_pBones[i].CreateTransformNode(pScene, m_bAutoTransform ? pModel->GetTransform() : NULL);
		}
	}
	else if (!bInUse && m_bInUse)
	{
		for (uint32 i = 0; i < m_uBoneCount; i++)
		{
			m_pBones[i].RemoveTransformNode(pScene);
		}
	}

	m_bInUse = bInUse;
}

void Skeleton::Update(float fElapsed)
{
	// Currently no animation so do nothing
}

Bone* Skeleton::GetBone(uint32 uIndex)
{
	ASSERT(uIndex < m_uBoneCount);
	return &m_pBones[uIndex];
}

const Bone* Skeleton::GetBone(uint32 uIndex) const
{
	ASSERT(uIndex < m_uBoneCount);
	return &m_pBones[uIndex];
}

Bone* Skeleton::GetBone(const usg::string& name)
{
	for(uint32 i=0; i<m_uBoneCount; i++)
	{
		if(m_pResource->GetBoneByIndex(i)->name == name)
		{
			return &m_pBones[i];
		}
	}

	return NULL;
}


const Bone* Skeleton::GetBone(const usg::string& name) const
{
	for(uint32 i=0; i<m_uBoneCount; i++)
	{
		if(m_pResource->GetBoneByIndex(i)->name == name)
		{
			return &m_pBones[i];
		}
	}

	return NULL;
}

}
/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "SkeletonResource.h"


namespace usg{


	SkeletonResource::SkeletonResource()
	{
		m_pRootBone = NULL;
		m_pBones	= NULL;
		m_uBoneCount	= 0;
	}
	
	SkeletonResource::~SkeletonResource()
	{
		if(m_pBones)
		{
			vdelete[] m_pBones;
		}
	}

	void SkeletonResource::Init(usg::exchange::Skeleton* pExhange, usg::exchange::Bone* pBones)
	{
		m_uBoneCount = pExhange->bonesNum;
		ASSERT(m_pBones == NULL);
		m_pBones = vnew(ALLOC_GEOMETRY_DATA) Bone[m_uBoneCount];

		for( uint32 i = 0; i < pExhange->bonesNum; ++i )
		{
			const usg::exchange::Bone& rSrcBone = pBones[i];
			Bone& rDestBone = m_pBones[i];

			rDestBone.name = rSrcBone.name;
			rDestBone.parentName = rSrcBone.parentName;
			rDestBone.parentIndex = rSrcBone.parentIndex;
			rDestBone.mMatrix = rSrcBone.transform;
			rDestBone.mBindMatrix = rSrcBone.bindPoseTrans;
			rDestBone.mInvBindMatrix = rSrcBone.invBindPoseTrans;
			rDestBone.vRotate = rSrcBone.rotate;
			rDestBone.vTranslate = rSrcBone.translate;
			rDestBone.vScale = rSrcBone.scale;
			rDestBone.bNeededRendering = rSrcBone.isNeededRendering != 0;

			rDestBone.cColSphere.SetPos( rSrcBone.boundingSphere.center );
			rDestBone.cColSphere.SetRadius( rSrcBone.boundingSphere.radius );
		}

		// Set root bone index
		m_pRootBone = &m_pBones[pExhange->rootBoneIndex];
	}

	const SkeletonResource::Bone* SkeletonResource::GetBone(const char* szName) const
	{
		usg::string cmp(szName);
		for(uint32 i=0; i<m_uBoneCount; i++)
		{
			if(m_pBones[i].name == szName)
			{
				return &m_pBones[i];
			}
		}

		return NULL;
	}

	uint32 SkeletonResource::GetBoneIndex(const char* szName) const
	{
		usg::string cmp(szName);
		for(uint32 i=0; i<m_uBoneCount; i++)
		{
			if(m_pBones[i].name == szName)
			{
				return i;
			}
		}

//		ASSERT(false);
		return USG_INVALID_ID;
	}


}


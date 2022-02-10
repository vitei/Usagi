/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Specifies bones and their starting locations
*****************************************************************************/
#ifndef _USG_GRAPHICS_SCENE_SKELETON_RESOURCE_H_
#define _USG_GRAPHICS_SCENE_SKELETON_RESOURCE_H_

#include "Engine/Maths/Matrix4x4.h"
#include "Engine/Scene/TransformNode.h"
#include "Engine/Maths/Sphere.h"
#include "Engine/Scene/Common/Mesh.h"
#include "Engine/Scene/Model/Skeleton.pb.h"


namespace vmdc {
struct Header;
}

namespace usg{

class SkeletonResource
{
public:
	SkeletonResource();
	~SkeletonResource();

	void Init(usg::exchange::Skeleton* pExhange, usg::exchange::Bone* pBones);

	struct Bone
	{
		usg::string		name;
		usg::string		parentName;
		uint32			parentIndex;
		// TODO: Improve the other matrix class so we can use it directly
		Matrix4x4		mMatrix;
		Matrix4x4		mBindMatrix;
		Matrix4x4		mInvBindMatrix;
		// The default values
		Vector3f		vScale;
		Vector3f		vRotate;
		Vector3f		vTranslate;
		bool			bNeededRendering;

		// A visibility sphere which is calculated based on the geometry which
		// references this bone. Any geometry which is attached to a single bone
		// can use this as an optimization
		usg::Sphere	cColSphere;
	};


	const Bone* GetBone(const char* szName) const;
	uint32 GetBoneIndex(const char* szName) const;
	const Bone* GetRootBone() const { return m_pRootBone; }
	const Bone* GetBoneByIndex(uint32 uIndex) const { ASSERT(uIndex<m_uBoneCount); return &m_pBones[uIndex]; }
	uint32 GetBoneCount() const { return m_uBoneCount; }

private:
	Bone*		m_pRootBone;
	Bone*		m_pBones;
	uint32		m_uBoneCount;
};

}

#endif	// #ifndef _USG_GRAPHICS_SCENE_SKELETON_RESOURCE_H_

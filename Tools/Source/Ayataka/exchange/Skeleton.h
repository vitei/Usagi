#ifndef exchange_Skeleton_h__
#define exchange_Skeleton_h__

#include "OwnSTLDecl.h"

#include "Engine/Scene/Model/Skeleton.pb.h"


#include "exchange/LoaderUtil.h"

namespace exchange {

class Skeleton
{
public:
	typedef std::vector< usg::exchange::Bone, aya::Allocator<usg::exchange::Bone> > VectorBone;

	Skeleton() {
		usg::exchange::Skeleton_init( &mSkeleton );
	}
	virtual ~Skeleton() {}

	usg::exchange::Skeleton& pb() { return mSkeleton; }
	const usg::exchange::Skeleton& pb() const { return mSkeleton; }

	VectorBone& Bones() { return mBones; }
	const VectorBone& Bones() const { return mBones; }

	uint32_t SearchBone( const char* name )
	{
		uint32_t i = 0;
		for( auto bone : mBones ) {
			if( strcmp( bone.name, name ) == 0 ) {
				return i;
			}
			++i;
		}

		return UINT32_MAX;
	}

	void ReverseCoordinate( void )
	{
		VectorBone::iterator ite = mBones.begin();
		VectorBone::iterator end = mBones.end();
		while( ite != end ) {
			ReverseCoordinateInt( *ite );
			++ite;
		}

		ite = mBones.begin();
		while (ite != end) {
			FillOutBindPoseMatrix(*ite);
			++ite;
		}
	}

	void FillOutBindPoseMatrix(usg::exchange::Bone& bone)
	{
		Matrix4x4 mBindMat = GetParentMatrix(bone);

		bone.bindPoseTrans = mBindMat;
		bone.bindPoseTrans.GetInverse(bone.invBindPoseTrans);
	}

private:
	void ReverseCoordinateInt( usg::exchange::Bone& bone ) {
		bone.translate.x *= -1.0f;
		bone.rotate.y *= -1.0f;
		bone.rotate.z *= -1.0f;
		LoaderUtil::setupTransformMatrix( bone.transform, bone.scale, bone.rotate, bone.translate );
		bone.boundingSphere.center.x *= -1.0f;
	}

	Matrix4x4 GetParentMatrix(usg::exchange::Bone& bone)
	{
		Matrix4x4 mMat = bone.transform;
		if (bone.parentName[0])
		{
			uint32 uIndex = SearchBone(bone.parentName);
			usg::exchange::Bone& parentBone = Bones().at(uIndex);
			mMat = mMat * GetParentMatrix(parentBone);
		}

		return mMat;
	}



	usg::exchange::Skeleton mSkeleton;
	VectorBone mBones;
};

} // namespace exchange

#endif // exchange_Skeleton_h__

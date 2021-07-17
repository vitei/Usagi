#ifndef FBXLOAD_H
#define FBXLOAD_H

#include "cmdl/Cmdl.h"
#include "exchange/Mesh.h"
#include "exchange/Shape.h"
#include "exchange/Material.h"
#include "exchange/Skeleton.h"
#include "exchange/MaterialAnimation.h"
#include "Dependencies/DependencyTracker.h"
#include "Engine/Core/Utility.h"
#include "Engine/Core/stl/vector.h"
#include "exchange/MaterialOverrides.h"
#include "Engine/Scene/Model/SkeletalAnimation.pb.h"
#include "Engine/Scene/Model/MaterialAnimation.pb.h"
#include <fbxsdk.h>

struct FBXVertexElement
{
	FBXVertexElement(std::string inHint, usg::exchange::VertexAttribute inType, usg::VertexElementType eInElementType, uint32 inIndex, uint32 inCount)
	{
		memset(hint, 0, sizeof(hint));
		for (uint32 i = 0; i < 4; i++)
		{
			elements[i] = 0.0f;
		}
		strcpy_s(hint, inHint.c_str());
		type = inType;
		uIndex = inIndex;
		uCount = inCount;
		eElementType = eInElementType;
	}

	// Only raw types allowed as we CRC the contents
	char hint[32];
	usg::exchange::VertexAttribute type;
	usg::VertexElementType eElementType;
	uint32	uIndex;
	uint32  uCount;
	union
	{
		float	elements[4];	// Nothing has more than 4
		uint8	uElements[4];
	};

	void Transform(FbxAMatrix& matrix, float fW = 0.0f)
	{
		ASSERT(uCount > 2);
		FbxVector4 vec(elements[0], elements[1], elements[2], fW);
		vec = matrix.MultT(vec);
		vec.Normalize();
		for (uint32 i = 0; i < uCount; i++)
		{
			elements[i] = (float)vec[i];
		}
	}

	bool operator==(const FBXVertexElement& rhs) const
	{
		if (type != rhs.type || uIndex != rhs.uIndex || uCount != rhs.uCount || eElementType != rhs.eElementType)
		{
			return false;
		}

		for (uint32 i = 0; i < 4; i++)
		{
			if (elements[i] != rhs.elements[i])
			{
				return false;
			}
		}

		return true;
	}

	bool operator!=(const FBXVertexElement& rhs) const
	{
		return !(*this == rhs);
	}
};

struct TempVertex
{
	usg::vector<FBXVertexElement> elements;
	union 
	{
		struct
		{
			uint32 controlPointIndex;
			uint32 caluclatedHash;
		};
		uint64 cmpValue = 0;
	};
	

	void CalculateHash()
	{
		caluclatedHash = utl::CRC32(elements.data(), (uint32)(sizeof(FBXVertexElement) * elements.size()));
	}

	bool operator==(const TempVertex& rhs) const
	{
		return caluclatedHash == rhs.caluclatedHash &&controlPointIndex == rhs.controlPointIndex;
	}
};

class FbxLoad
{
public:
	FbxLoad();

	void Load(Cmdl& cmdl, FbxScene*	modelScene, bool bSkeletonOnly, bool bCollisionMode, DependencyTracker* pDependencies, MaterialOverrides* pOverrides);
	void SetAppliedScale(double appliedScale) { m_appliedScale = appliedScale; }
	void SetAttenScale(double fScale) { m_fAttenScale = fScale; }

private:
	void ReadMeshRecursive(Cmdl& cmdl, FbxNode* pNode, bool bStatic);

	// Skeleton
	void ReadSkeleton(::exchange::Skeleton* pSkeleton, FbxNode* pRootNode);
	void ReadBonesRecursive(::exchange::Skeleton* pSkeleton, FbxNode* pNode);
	void ReadLightsRecursive(Cmdl& cmdl, FbxNode* pNode);
	void ReadCamerasRecursive(Cmdl& cmdl, FbxNode* pNode);
	void ReadDeformersRecursive(::exchange::Skeleton* pSkeleton, FbxNode* pNode);
	
	// Mesh data
	void AddMesh(Cmdl& cmdl, ::exchange::Shape*, FbxNode* pNode, FbxMesh* mesh, bool bStatic);
	void GetUV(FbxMesh* pMesh, int iVertexIndex, int iTexUVIndex, int iUVLayer, FBXVertexElement& outUV);
	bool GetColor(FbxMesh* pMesh, int iVertexIndex, int inColorId, FBXVertexElement& outColor);
	bool GetNormal(FbxMesh* pMesh, int iVertexIndex, int iVertex, FBXVertexElement& outNormal);
	bool GetBinormal(FbxMesh* pMesh, int iVertexIndex, int iVertex, FBXVertexElement& outBinormal);
	bool GetTangent(FbxMesh* pMesh, int iVertexIndex, int iVertex, FBXVertexElement& outTangent);
	uint32 GetBlendWeightsAndIndices(Cmdl& cmdl, FbxNode* pNode, FbxMesh* currMesh); 	// Returns the maximum of bones weighting any one vertex
	void RemoveDuplicateVertices();

	// Animations
	void ReadAnimations(Cmdl& cmdl, FbxScene* pScene);
	void ReadAnimationsRecursive(FbxAnimStack* pAnimStack, ::exchange::Animation* pAnim, FbxNode* pNode);
	void ReadAnimationKeyFramesRecursively(FbxAnimStack* pAnimStack, ::exchange::Animation* pAnim, ::exchange::MaterialAnimation* pMatAnim, FbxNode* pNode);
	bool GetAnimBoneInfluences(FbxAnimLayer* pAnimStack, FbxNode* pNode, bool& bTrans, bool& bRot, bool& bScale);
	void FillOutAnimFrame(FbxNode* pNode, FbxTime currTime, usg::exchange::BoneAnimationFrame* pFrame);

	// Materials
	void AddMaterials(Cmdl& cmdl, FbxNode* pNode, bool bSkinned);
	void AddMaterialTextures(FbxSurfaceMaterial* pFBXMaterial, ::exchange::Material* pNewMaterial);
	bool GetTextureIndex(const FbxTexture& textureInfo, const char* szTexName, ::exchange::Material* pMaterial, uint32& uIndex);	

	::exchange::Shape* NewShape(Cmdl& cmdl, FbxNode* pShapeNode);
	::exchange::Mesh* NewMesh(Cmdl& cmdl, FbxNode* pShapeNode);
	::exchange::Animation* FbxLoad::NewAnimation(Cmdl& cmdl, FbxAnimStack* layerNode);
	::exchange::MaterialAnimation* FbxLoad::NewMaterialAnimation(Cmdl& cmdl, FbxAnimStack* layerNode);
	::exchange::Material* NewMaterial(FbxSurfaceMaterial* pFBXMaterial, bool bSkin);
	::exchange::Material* DummyMaterial(bool bSkinned);
	::exchange::Skeleton* NewSkeleton();

	void AddIdentityBone(::exchange::Skeleton* pSkeleton);
	bool IsIdentityBoneRequired(FbxNode* pRootNode);
	int GetParentBoneCountRecursive(FbxNode* pRootNode, int Count);
	void AddBone(::exchange::Skeleton* pSkeleton, FbxNode* pNode, bool bIsNeededRendering);
	void AddLight(Cmdl& cmdl, FbxNode* pNode);
	void AddCamera(Cmdl& cmdl, FbxNode* pNode);
	uint32 FindBone(::exchange::Skeleton& skel, const char* szName);
	uint32 FindBoneRenderingId(Cmdl& cmdl, const char* szName);
	void AddStreams(Cmdl& cmdl, ::exchange::Shape* pShape, FbxNode* pNode, FbxMesh* pCurrMesh);
	bool SetDefaultMaterialVariables(FbxSurfaceMaterial* pFBXMaterial, ::exchange::Material* pMaterial);
	void SetBoolBasedOnTexture(::exchange::Material* pNewMaterial, const char* szTexName, const char* szBoolName);
	void SetRenderState(::exchange::Material* pNewMaterial, FbxSurfaceMaterial* inMaterial, bool bTransparent) const;
	FbxAMatrix GetCombinedMatrixForNode(FbxNode* pNode, FbxNode* pParent, FbxTime pTime = FBXSDK_TIME_INFINITE);
	FbxAMatrix GetGlobalPoseMatrix(FbxNode* pNode);
	FbxAMatrix GetLocalPoseMatrix(Cmdl& cmdl, FbxAMatrix globalPose, const char* szParentName);
	bool IsBone(FbxNode* pNode);
	usg::exchange::CurveKeyFrameType GetKeyFrameType(FbxAnimCurveDef::EInterpolationType eTypeIn);
	bool AddAnimCurve(FbxAnimStack* pAnimStack, ::exchange::MaterialAnimation* pMatAnim, FbxPropertyT<FbxDouble3>& prop, usg::exchange::MaterialAnimationMemberType eType, const char* szName);

	// Below here are custom tweaks for our own behaviour
	// At this stage only used for culling duplicate bones used for the LOD system
	bool PostProcessSkeleton(Cmdl& cmdl);
	// Primarily for the collision mesh
	void SetupAdjacencyStream(Cmdl& cmdl);
	void PostProcessing(Cmdl& cmdl);

	void RegisterBoneUsage(Cmdl& cmdl, const char* szName, usg::exchange::SkinningType eSkinType);

	struct BoneWeight
	{
		int		index;
		float	fValue;

		bool operator<(const BoneWeight &rhs)
		{
			return fValue < rhs.fValue;
		}
	};

	struct WeightingInfo
	{
		usg::vector<BoneWeight>	weights;
	};

	struct BoneInfo
	{
		uint32 uOriginalMapping;
		uint32 uNewMapping;
		char szName[40];
	};

	uint32 FindBone(uint32 uOriginalIndex, const vector<BoneInfo>& boneInfo);

	vector<BoneInfo>		m_skinnedBoneIndices;
	vector<BoneInfo>		m_rigidBoneIndices;

	// For getting the bind pose global matrices
	vector<FbxPose*>		m_bindPoses;
	vector<FbxCluster*>		m_clusters;	
	vector<FbxAMatrix>		m_globalBonePoses;

	uint32					m_uTrianglesTmp;
	uint32					m_uMeshMaterialOffset;
	uint32					m_uMeshMaterialTmp;
	bool					m_bHasNormalMap;
	bool					m_bHasDefaultStaticBone;
	bool					m_bCollisionMesh;
	double					m_appliedScale;
	double					m_fAttenScale;
	FbxNode*				m_pParentBoneNode;
	FbxScene*				m_pScene;
	usg::vector<uint32>		m_indicesTmp;
	usg::vector<TempVertex>	m_activeVerts;
	usg::vector<WeightingInfo> m_activeWeights;
	DependencyTracker*		m_pDependencies;
	MaterialOverrides*		m_pOverrides;
};

#endif // FMDALOAD_H

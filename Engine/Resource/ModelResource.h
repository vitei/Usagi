/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Simple model from binary
*****************************************************************************/
#ifndef _USG_GRAPHICS_SCENE_MODEL_RESOURCE_H_
#define _USG_GRAPHICS_SCENE_MODEL_RESOURCE_H_
#include "Engine/Common/Common.h"
#include "Engine/Maths/Matrix4x4.h"
#include "Engine/Resource/SkeletonResource.h"
#include "Engine/Resource/ResourceBase.h"
#include "Engine/Resource/ResourceDecl.h"
#include "Engine/Scene/Model/Model.pb.h"
#include "Engine/Graphics/Materials/Material.pb.h"
#include "Engine/Scene/Model/Shape.pb.h"
#include "Engine/Core/stl/vector.h"
  
namespace usg{

class GFXContext;
class CustomEffectRuntime;

class ModelResource : public ResourceBase
{
public:
	// We could do this with a standard mesh but we want
	// to allow overrides of this data
	typedef usg::exchange::TextureCoordinator TextureCoordInfo;

	struct Mesh;
	
	ModelResource();
	virtual ~ModelResource();

	bool Load( GFXDevice* pDevice, const char* szFileName, bool bInstance, bool bFastMem );
	void CleanUp(GFXDevice* pDevice);
	uint32 GetMeshCount() const { return m_uMeshCount; }
	const Mesh* GetMesh(uint32 uMesh) const;
	const usg::Sphere& GetBounds() const { return m_bounds;}
	const U8String& GetName() const { return m_name; }
	bool IsInstanceModel() const { return m_bInstance; }

	uint32 GetRequiredBoneNodeCount() const { return m_uBoneNodes; }
	bool NeedsRootNode() const { return m_bNeedsRootNode; }
	// Enable when done
	const SkeletonResource* GetDefaultSkeleton() const {  return &m_defaultSkeleton; }

	static uint32 GetInstanceDecl();
	static void GetModelDeclUVReusse(const usg::exchange::Shape* pShape, const CustomEffectRuntime& runTime, const exchange::Material* pMaterial, VertexElement elements[], memsize& sizeout);
	static uint32 GetBoneIndexCount(const usg::exchange::Shape* pShape);
	static bool HasAttribute(const usg::exchange::VertexStreamInfo* pInfo, exchange::VertexAttribute attrib, uint32 uCount);
	static void GetSingleAttributeDecl( usg::exchange::VertexAttribute attr, uint32 uCount, VertexElement element[2] );
	static bool GetSingleAttributeDeclNamed(const CustomEffectRuntime& runTime,const char* szName, uint32 uCount, VertexElement element[2]);

	const usg::vector<uint32>& GetRigidBoneIndices() const { return m_rigidBoneIndices; }
	const usg::vector<uint32>& GetSmoothBoneIndices() const { return m_smoothBoneIndices; }
private:

	void SetupMeshes(const U8String & modelDir, GFXDevice* pDevice, uint8* p, bool bFastMem );
	void SetupMesh(const U8String & modelDir, GFXDevice* pDevice, usg::exchange::ModelHeader* pHeader, uint32 meshIndex, bool bFastMem );
	void SetupSkeleton( uint8* p );
	void CreateDepthPassMaterial(GFXDevice* pDevice, uint32 uMeshIndex, exchange::Shape* pShape, exchange::Material* pMaterial, const U8String& effectName);
	float GetStreamScaling(const usg::exchange::VertexStreamInfo* pInfo, uint32 uCount, usg::exchange::VertexAttribute eType);

	SkeletonResource		m_defaultSkeleton;
	Mesh*                   m_meshArray;
	usg::Sphere				m_bounds;
	uint32					m_uMeshCount;

	usg::vector<uint32>		m_rigidBoneIndices;
	usg::vector<uint32>		m_smoothBoneIndices;
	bool					m_bNeedsRootNode;
	uint32					m_uBoneNodes;
	U8String				m_name;
	bool					m_bInstance;
};

}

#endif	// #ifndef _USG_GRAPHICS_SCENE_MODEL_H_

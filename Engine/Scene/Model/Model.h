/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Simple model from binary
*****************************************************************************/
#ifndef _USG_GRAPHICS_SCENE_MODEL_H_
#define _USG_GRAPHICS_SCENE_MODEL_H_


#include "Engine/Maths/Matrix4x4.h"
#include "Engine/Scene/TransformNode.h"
#include "Engine/Scene/Model/Skeleton.h"
 

namespace usg {

class UVMapper;

class Model
{
public:
	//static const int MESH_MAX_NUM = 24;
	enum IdentifierType
	{
		IDENTIFIER_MESH = 0,
		IDENTIFIER_MATERIAL
	};

	Model();
	Model(GFXDevice* pDevice, Scene* pScene, ResourceMgr* pResMgr, const char* szFileName, bool bDynamic);
	virtual ~Model();

	// bAutoTransform - Scene nodes are parented and automatically updated, set this to false when using the component system
	bool Load( GFXDevice* pDevice, Scene* pScene, ResourceMgr* pResMgr, const char* szFileName, bool bDynamic = false, bool bFastMem = true, bool bAutoTransform = true, bool bPerBoneCulling = true );
	void Cleanup(GFXDevice* pDevice);
	void InitDynamics(GFXDevice* pDevice, Scene* pScene, uint32 uMesh);
	// Note that AddToScene is dependent on a GPU update, to remove a model from the systems use ForceRemoveFromScene
	void AddToScene(bool bAdd);
	void ForceRemoveFromScene();
	const U8String& GetName() const;
	void SetInUse(bool bInUse);

	void SetTransform(const Matrix4x4 &trans);
	bool OverrideTexture(const char* szTextureName, TextureHndl pSrcTex);	

	void OverrideVariable(const char* szVarName, void* pData, uint32 uSize, uint32 uIndex);
	template <class VariableType>
	void OverrideVariable(const char* szVarName, VariableType& var, uint32 uIndex = 0)
	{
		OverrideVariable(szVarName, (void*)&var, sizeof(VariableType), uIndex);
	}
	void UpdateDescriptors(GFXDevice* pDevice);
	const Matrix4x4& GetTransform() const { return m_pTransformNode->GetMatrix(); }
	TransformNode* GetTransform() { return m_pTransformNode; }
	const ModelResHndl& GetResource() const { return m_pResource; }

	void SetFade(bool bFade, float fAlpha = 1.0f);
	void RemoveOverrides(GFXDevice* pDevice);
	void SetRenderMask(uint32 uRenderMask);
	void EnableShadow(GFXDevice* pDevice, bool bEnable);

	void SetTextureTranslate(const char* szName, uint32 uTexId, float fX, float fY, IdentifierType eNameType = IDENTIFIER_MATERIAL);
	void AddTextureTranslate(const char* szName, uint32 uTexId, float fX, float fY, IdentifierType eNameType = IDENTIFIER_MATERIAL);
	void SetTextureScale( const char* szName, uint32 uTexId, float fX, float fY, IdentifierType eNameType = IDENTIFIER_MATERIAL );
	uint32 GetMeshIndex(const char* szName, IdentifierType eNameType = IDENTIFIER_MESH) const;
	UVMapper* GetUVMapper(uint32 uMesh, uint32 uTexIndex);
	
	void AddTextureRotation(const char* szName, uint32 uTexId, float fRot, IdentifierType eNameType = IDENTIFIER_MATERIAL);

	class RenderMesh;
	RenderMesh* GetRenderMesh(const char* szMaterailName);
	RenderMesh* GetRenderMesh(uint32 uMeshId);
	const RenderMesh* GetRenderMesh(uint32 uMeshId) const;

	void SetLayer(usg::RenderLayer uLayer);
	void SetPriority(uint8 uPriority);
	void RestoreDefaultLayer();
	bool IsOnScreen();
	bool ShouldDraw() { return m_bVisible; }
	uint32 GetMeshCount() const;
	Skeleton& GetSkeleton() { return *m_pSkeleton; }
	const Skeleton& GetSkeleton() const { return *m_pSkeleton; }

	void GPUUpdate(usg::GFXDevice* pDevice, bool bVisible = true);

	// TODO: Use sparingly!
	void SetScale(float fScale);
	float GetAlpha() const { return m_bVisible ? m_fAlpha : 0.0f; }
	float GetScale() const { return m_fScale; }
	void SetDynamic(GFXDevice* pDevice, bool bDynamic);

	const ConstantSet& GetSkinnedBones() const { return m_skinnedBones; }
	const ConstantSet& GetRigidBones() const { return m_staticBones;  }

private:
	void AddToSceneInt(GFXDevice* pDevice);
	void AddDepthMesh(GFXDevice* pDevice, bool bAdd);
	void UpdateRenderMaskInt();

	struct MaterialInfo;
	class RenderMesh;
	class BonePaletteMesh;

	ConstantSet				m_skinnedBones;
	ConstantSet				m_staticBones;

	ModelResHndl			m_pResource;
	
	RenderMesh**            m_meshArray;
	RenderMesh**            m_depthMeshArray;
	Skeleton*				m_pSkeleton;

	TransformNode*			m_pTransformNode;
	RenderGroup*			m_pRenderGroup;	// Only valid when not per bone culling
	bool					m_bPerBoneCulling;

	bool					m_bVisible;
	bool					m_bShouldBeVisible;
	bool					m_bDepthPassMeshEnabled;
	bool					m_bFade;
	float					m_fAlpha;
	uint32					m_uRenderMask;
	uint32					m_depthRenderMask;
	float					m_fScale;

	MaterialInfo*			m_pOverrideMaterials;
	bool					m_bDynamic;
	uint32					m_uMeshCount;

	Scene* m_pScene;
};


}

#endif	// #ifndef _USG_GRAPHICS_SCENE_MODEL_H_

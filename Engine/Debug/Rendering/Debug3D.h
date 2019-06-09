/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: The in game 3D debug elements
*****************************************************************************/
#ifndef __USG_DEBUG_RENDERING_DEBUG3D_H__
#define __USG_DEBUG_RENDERING_DEBUG3D_H__

#include "Engine/Graphics/Color.h"
#include "Engine/Graphics/Primitives/VertexBuffer.h"
#include "Engine/Graphics/Primitives/IndexBuffer.h"
#include "Engine/Graphics/Materials/Material.h"
#include "Engine/Debug/Rendering/CubeRender.h"
#include "Engine/Graphics/Effects/ConstantSet.h"

namespace usg {

class ViewContext;

class Debug3D : public RenderNode
{
public:
	Debug3D();
	virtual ~Debug3D();

	void Init(GFXDevice* pDevice, Scene* pScene);
	void InitContextData(GFXDevice* pDevice, ViewContext* pContext);
	void CleanUp(GFXDevice* pDevice);

	void AddSphere(const Vector3f &vPos, float fRadius, const Color& color);
	void AddCube(const Matrix4x4& mMat, const Color& color);
	void AddLine(const Vector3f& vStart, const Vector3f& vEnd, const Color& color, float fWidth);
	void Clear();
	void UpdateBuffers(GFXDevice* pDevice);
	virtual void RenderPassChanged(GFXDevice* pDevice, uint32 uContextId, const RenderPassHndl &renderPass, const SceneRenderPasses& passes) override;


	virtual bool Draw(GFXContext* pContext, RenderContext& renderContext);

	static Debug3D* GetRenderer();

	struct SphereData
	{
		Vector4f	vPos;
		Vector4f	vColor;
	};

private:
	void MakeSphere(GFXDevice* pDevice);

	enum
	{
		MAX_SPHERES = 1024,
		MAX_CUBES = 2048
	};

	static Debug3D*			m_psRenderer;

	VertexBuffer			m_transforms;
	VertexBuffer::Lock		m_transformLock;
	
	PipelineStateHndl		m_spherePipeline;
	PipelineStateHndl		m_cubePipeline;
	Material				m_lineMat;
	ConstantSet				m_lineConstants;
	
	VertexBuffer			m_sphereVB;
	VertexBuffer			m_cubeVB;
	
	IndexBuffer				m_sphereIB;
	IndexBuffer				m_cubeIB;

	CubeRender::Cube		m_cubes[MAX_CUBES];
	SphereData				m_spheres[MAX_SPHERES];

	uint32					m_uSpheres;
	uint32					m_uCubes;

	RenderGroup*			m_pRenderGroup;
};

}

#endif

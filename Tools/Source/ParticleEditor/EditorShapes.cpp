#include "Engine/Common/Common.h"
#include "Engine/Maths/MathUtil.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/SceneConstantSets.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/Scene/ViewContext.h"
#include "Engine/Graphics/Device/GFXContext.h"
#include "Engine/Graphics/StandardVertDecl.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "EditorShapes.h"

using namespace usg;

struct TransformData
{
	usg::Matrix4x4	mModel;
	usg::Vector4f	vExtents;
	usg::Vector4f	vColor;
	float			fArcStart;
	float			fArcAngle;
	bool			bUseArc;
};

static const ShaderConstantDecl g_transformConstantDef[] = 
{
	SHADER_CONSTANT_ELEMENT( TransformData, mModel,		CT_MATRIX_44, 1 ),
	SHADER_CONSTANT_ELEMENT( TransformData, vExtents,	CT_VECTOR_4, 1 ),
	SHADER_CONSTANT_ELEMENT( TransformData, vColor,		CT_VECTOR_4, 1 ),
	SHADER_CONSTANT_ELEMENT(TransformData, fArcStart,	CT_FLOAT, 1),
	SHADER_CONSTANT_ELEMENT(TransformData, fArcAngle,	CT_FLOAT, 1),
	SHADER_CONSTANT_ELEMENT(TransformData, bUseArc,		CT_BOOL, 1),
	SHADER_CONSTANT_END()
};

static const DescriptorDeclaration g_descriptorDecl[] =
{
	DESCRIPTOR_ELEMENT(SHADER_CONSTANT_MATERIAL, DESCRIPTOR_TYPE_CONSTANT_BUFFER, 1, SHADER_FLAG_VS_GS),
	DESCRIPTOR_END()
};

EditorShapes::EditorShapes() 
	: m_bEnable(true)
{
	SetLayer(LAYER_SKY);
	SetPriority(128);
}

void EditorShapes::Init(usg::GFXDevice* pDevice, usg::Scene* pScene)
{
	
	PipelineStateDecl pipeline;
	pipeline.ePrimType = PT_TRIANGLES;

	pipeline.inputBindings[0].Init(GetVertexDeclaration(usg::VT_POSITION));
	pipeline.uInputBindingCount = 1;

	DescriptorSetLayoutHndl matDescriptors = pDevice->GetDescriptorSetLayout(g_descriptorDecl);
	pipeline.layout.descriptorSets[0] = pDevice->GetDescriptorSetLayout(SceneConsts::g_globalDescriptorDecl);
	pipeline.layout.descriptorSets[1] = matDescriptors;
	pipeline.layout.uDescriptorSetCount = 2;

	AlphaStateDecl& alphaDecl = pipeline.alphaState;
	alphaDecl.SetColor0Only();
	alphaDecl.uColorTargets = 2;
	alphaDecl.bBlendEnable = false;
	alphaDecl.blendEq = BLEND_EQUATION_ADD;
	alphaDecl.srcBlend = BLEND_FUNC_SRC_ALPHA;
	alphaDecl.dstBlend = BLEND_FUNC_ONE_MINUS_SRC_ALPHA;

	DepthStencilStateDecl& depthDecl = pipeline.depthState;
	depthDecl.bDepthWrite		= true;
	depthDecl.bDepthEnable		= true;
	depthDecl.eDepthFunc 		= DEPTH_TEST_LEQUAL;
	depthDecl.bStencilEnable	= false;
	depthDecl.eStencilTest		= STENCIL_TEST_ALWAYS;

	RasterizerStateDecl& rasDecl = pipeline.rasterizerState;
	// FIXME: Replace with a wireframe geometry shader as this is deprecated and will cause some systems to fall over
	rasDecl.bWireframe = true;
	rasDecl.eCullFace = usg::CULL_FACE_NONE;
	rasDecl.bUseDepthBias = false; 

	usg::RenderPassHndl renderPassHndl = pScene->GetViewContext(0)->GetRenderPasses().GetRenderPass(*this);
	pipeline.pEffect = usg::ResourceMgr::Inst()->GetEffect(pDevice, "Debug.Wireframe");
	m_objectMat.Init(pDevice, pDevice->GetPipelineState(renderPassHndl, pipeline), pDevice->GetDescriptorSetLayout(g_descriptorDecl));
	m_hollowObjectMat.Init(pDevice, pDevice->GetPipelineState(renderPassHndl, pipeline), pDevice->GetDescriptorSetLayout(g_descriptorDecl));
	pipeline.pEffect = usg::ResourceMgr::Inst()->GetEffect(pDevice, "Debug.Wireframe");
	m_gridMat.Init(pDevice, pDevice->GetPipelineState(renderPassHndl, pipeline), pDevice->GetDescriptorSetLayout(g_descriptorDecl));

	m_objectConstants.Init(pDevice, g_transformConstantDef);
	m_hollowObjectConstants.Init(pDevice, g_transformConstantDef);
	m_gridConstants.Init(pDevice, g_transformConstantDef);
	m_objectMat.SetConstantSet(SHADER_CONSTANT_MATERIAL, &m_objectConstants);
	m_objectMat.UpdateDescriptors(pDevice);
	m_hollowObjectMat.SetConstantSet(SHADER_CONSTANT_MATERIAL, &m_hollowObjectConstants);
	m_hollowObjectMat.UpdateDescriptors(pDevice);
	m_gridMat.SetConstantSet(SHADER_CONSTANT_MATERIAL, &m_gridConstants);

	TransformData* pTransformData = m_gridConstants.Lock<TransformData>();
	pTransformData->mModel = Matrix4x4::Identity();
	pTransformData->vColor.Assign(0.7f, 0.7f, 0.7f, 0.4f);
	pTransformData->vExtents.Assign(1.0f, 1.0f, 1.0f, 1.0f);
	m_gridConstants.Unlock();
	m_gridConstants.UpdateData(pDevice);
	m_gridMat.UpdateDescriptors(pDevice);

	MakeSphere(pDevice);
	MakeCube(pDevice);
	MakeCylinder(pDevice);
	MakeGrid(pDevice);

	m_pScene = pScene;
	m_pRenderGroup = pScene->CreateRenderGroup(NULL);


	RenderNode* pNode = this;
	m_pRenderGroup->AddRenderNodes( pDevice, &pNode, 1, 0 );

}

void EditorShapes::CleanUp(usg::GFXDevice* pDevice)
{
	m_objectConstants.CleanUp(pDevice);
	m_hollowObjectConstants.CleanUp(pDevice);
	m_objectMat.Cleanup(pDevice);
	m_hollowObjectMat.Cleanup(pDevice);
	m_gridMat.Cleanup(pDevice);
	m_gridConstants.CleanUp(pDevice);
	m_grid.vb.CleanUp(pDevice);
	m_sphere.vb.CleanUp(pDevice);
	m_box.vb.CleanUp(pDevice);
	m_cylinder.vb.CleanUp(pDevice);
	m_line.vb.CleanUp(pDevice);
	m_grid.ib.CleanUp(pDevice);
	m_sphere.ib.CleanUp(pDevice);
	m_box.ib.CleanUp(pDevice);
	m_cylinder.ib.CleanUp(pDevice);
	m_line.ib.CleanUp(pDevice);

}

bool EditorShapes::Draw(usg::GFXContext* pContext, RenderContext& renderContext)
{
	if (!m_bEnable)
		return false;

	m_gridMat.Apply(pContext);
	pContext->SetVertexBuffer(&m_grid.vb);
	pContext->DrawIndexed(&m_grid.ib);

	for (uint32 i = 0; i < 2; i++)
	{
		usg::Material& mat = i == 0 ? m_objectMat : m_hollowObjectMat;
		mat.Apply(pContext);
		switch (m_eShape)
		{
		case usg::particles::EMITTER_SHAPE_SPHERE:
		{
			pContext->SetVertexBuffer(&m_sphere.vb);
			pContext->DrawIndexed(&m_sphere.ib);
			break;
		}
		case usg::particles::EMITTER_SHAPE_CUBE:
		{
			pContext->SetVertexBuffer(&m_box.vb);
			pContext->DrawIndexed(&m_box.ib);
			break;
		}
		case usg::particles::EMITTER_SHAPE_CYLINDER:
		{
			pContext->SetVertexBuffer(&m_cylinder.vb);
			pContext->DrawIndexed(&m_cylinder.ib);
			break;
		}
		default:
			break;
		}
	}

	return true;
}

void EditorShapes::MakeGrid(GFXDevice* pDevice)
{
	uint32 uDivisions = 20;
	uint32 uLines = uDivisions + 1;
	float fDivisionWidth = 1.0f;

	uint32 uVertices = uLines*uLines;
	PositionVertex* pVertices = NULL;
	ScratchObj<PositionVertex> scratchVertices(pVertices, uVertices, 4);

	uint32 uIndices = uDivisions*uDivisions*6;	// 2 triangles per square

	uint16* puIndices = NULL;
	ScratchObj<uint16> scratchIndices(puIndices, uIndices, 4);

	float fHalfWidth = (uDivisions/2.f)*fDivisionWidth;
	Vector3f vPosition(-fHalfWidth, 0.0f, -fHalfWidth);

	// First fill out the vertices
	PositionVertex* pVert = pVertices;
	for(uint32 z=0; z<uLines; z++)
	{
		for(uint32 x=0; x<uLines; x++)
		{
			pVert->x = vPosition.x;
			pVert->y = 0.0f;
			pVert->z = vPosition.z;
			vPosition.x += fDivisionWidth;
			pVert++;
		}
		vPosition.x = -fHalfWidth;
		vPosition.z += fDivisionWidth;
	}

	uint16* pIndex = puIndices;
	// Now fill out the indices
	for(uint32 z=0; z<uDivisions; z++)
	{
		for(uint32 x=0; x<uDivisions; x++)
		{
			pIndex[0] = (z*uLines)+x;
			pIndex[1] = ((z+1)*uLines)+x;
			pIndex[2] = (z*uLines)+(x+1);

			pIndex[3] = ((z+1)*uLines)+x;
			pIndex[4] = ((z+1)*uLines)+(x+1);
			pIndex[5] = (z*uLines)+(x+1);

			pIndex += 6;
		}
	}

	m_grid.vb.Init(pDevice, pVertices, sizeof(PositionVertex), uVertices, "Editor grid");
	m_grid.ib.Init(pDevice, puIndices, uIndices);
}

static inline void sincosf(float angle, float* psin, float* pcos)
{
	*psin = sinf(angle);
	*pcos = cosf(angle);
}


void EditorShapes::MakeCylinder(usg::GFXDevice* pDevice)
{
	const float fRadius = 1.0f;	// Will be scaled in the	vertexShader;
	const uint32 uSlices = 32;

	uint32 uBaseTriangles = uSlices;
	uint32 uSideTriangles = uSlices * 2;

	uint32 uVertices = (uBaseTriangles * 2) + 2;	// Extras are the centre of each end
	uint32 uIndices = ((uBaseTriangles*2) + uSideTriangles) * 3;

	PositionVertex* pVertices = NULL;
	ScratchObj<PositionVertex> scratchVertices(pVertices, uVertices, 4);
	uint16* puIndices = NULL;
	ScratchObj<uint16> scratchIndices(puIndices, uIndices, 4);

	float fSinI[uSlices];
	float fCosI[uSlices];

	for (uint32 i = 0; i < uSlices; i++)
	{
		sincosf(2.0f * Math::pi * i / uSlices, fSinI + i, fCosI + i);
	}


	// Generate vertices
	PositionVertex* pVertex = pVertices;

	// Top center
	pVertex->x = 0.0f;
	pVertex->y = -1.0f;
	pVertex->z = 0.0f;
	pVertex++;

	// Base center
	pVertex->x = 0.0f;
	pVertex->y = 1.0f;
	pVertex->z = 0.0f;
	pVertex++;

	for (uint32 i = 0; i < uSlices; i++)
	{
		Vector3f norm(fSinI[i], fCosI[i], 0.0f);
		norm.Normalise();	// Shouldn't be necessary, but ensure accuracy
		Vector3f pos = norm * fRadius;

		pVertex->x = pos.x;
		pVertex->y = -1.0f;
		pVertex->z = pos.y;

		pVertex++;

		pVertex->x = pos.x;
		pVertex->y = 1.0f;
		pVertex->z = pos.y;

		pVertex++;
	}


	// Generate indices
	uint16* puFace = puIndices;


	uint16 uStart = 2;

	// The side triangles
	for (uint32 i = 0; i < uSlices - 1; i++)
	{
		puFace[2] = (i * 2) + uStart;
		puFace[1] = (i*2) + uStart + 1;
		puFace[0] = (i*2) + uStart + 2;
		puFace += 3;

		puFace[0] = (i*2)+uStart + 1;
		puFace[1] = (i*2) +uStart + 2;
		puFace[2] = (i*2) +uStart + 3;
		puFace += 3;
	}

	puFace[2] = ((uSlices-1) * 2) + uStart;
	puFace[1] = ((uSlices - 1) * 2) + uStart + 1;
	puFace[0] =  uStart;
	puFace += 3;

	puFace[0] =  ((uSlices - 1) * 2) + uStart + 1;
	puFace[1] =  uStart;
	puFace[2] =  uStart + 1;
	puFace += 3;

	// The top triangles
	for (uint32 i = 0; i < uSlices - 1; i++)
	{
		puFace[0] = (i * 2) + uStart;
		puFace[1] = 0;
		puFace[2] = ((i + 1) * 2) + uStart;
		puFace += 3;
	}

	puFace[0] = ((uSlices - 1) * 2) + uStart;
	puFace[1] = 0;
	puFace[2] = uStart;
	puFace += 3;

	// The base triangles
	for (uint32 i = 0; i < uSlices - 1; i++)
	{
		puFace[2] = (i * 2) + uStart;
		puFace[1] = 0;
		puFace[0] = ((i+1) * 2) + uStart;
		puFace[0]++;
		puFace[1]++;
		puFace[2]++;
		puFace += 3;
	}

	puFace[2] = ((uSlices-1)*2) + uStart + 1;
	puFace[1] = 1;
	puFace[0] = uStart + 1;
	puFace += 3;

	m_cylinder.vb.Init(pDevice, pVertices, sizeof(PositionVertex), uVertices, "Cylinder");
	m_cylinder.ib.Init(pDevice, puIndices, uIndices, PT_TRIANGLES);
}

void EditorShapes::MakeCube(GFXDevice* pDevice)
{
	PositionVertex verts[8] =
	{
		// Top
		{ -1.0f,  1.0f, -1.0f }, // 0 - BL
		{  1.0f,  1.0f, -1.0f }, // 1 - BR
		{  1.0f,  1.0f,  1.0f }, // 2 - FR
		{ -1.0f,  1.0f,  1.0f }, // 3 - FL
		// Bottom
		{ -1.0f, -1.0f, -1.0f }, // 4 - BL
		{  1.0f, -1.0f, -1.0f }, // 5 - BR
		{  1.0f, -1.0f,  1.0f }, // 6 - FR
		{ -1.0f, -1.0f,  1.0f }, // 7 - FL
	};

	uint16 iIndices[36] =
	{
		3, 1, 0, 3, 2, 1,    // Top
		4, 3, 0, 4, 7, 3,    // Left
		7, 2, 3, 7, 6, 2,    // Front
		6, 1, 2, 6, 5, 1,    // Right
		5, 0, 1, 5, 4, 0,    // Back
		4, 6, 7, 4, 5, 6     // Bottom
	};

	m_box.vb.Init(pDevice, verts, sizeof(PositionVertex), 8, "Cube");
	m_box.ib.Init(pDevice, iIndices, 36, PT_TRIANGLES);
}

void EditorShapes::Update(usg::GFXDevice* pDevice, usg::particles::EmitterShape eShape, const usg::particles::EmitterShapeDetails* pShape, float fElapsed)
{
	Matrix4x4 mScale;
	mScale.MakeScale(pShape->baseShape.vScale);

	Matrix4x4 mHollowScale;
	mHollowScale.MakeScale(pShape->baseShape.vScale * (1.0f - pShape->baseShape.fHollowness));

	TransformData* pTransformData = m_objectConstants.Lock<TransformData>();
	TransformData* pTransformHollow = m_hollowObjectConstants.Lock<TransformData>();
	pTransformData->mModel.LoadIdentity();
	pTransformData->mModel.MakeRotate(Math::DegToRad(pShape->baseShape.vRotation.x), -Math::DegToRad(pShape->baseShape.vRotation.y), Math::DegToRad(pShape->baseShape.vRotation.z));
	pTransformData->mModel.SetTranslation(pShape->baseShape.vPosition);
	pTransformData->mModel = pTransformData->mModel * mScale;

	pTransformData->vColor.Assign(0.0f, 0.0f, 1.0f, 0.4f);
	pTransformData->vExtents.Assign(pShape->vShapeExtents.x, pShape->vShapeExtents.y, pShape->vShapeExtents.z, 1.0f);
	// Give some room for error
	pTransformData->fArcStart = Math::DegToRad(pShape->arc.fArcStartDeg) - 0.01f;
	pTransformData->fArcAngle = Math::DegToRad(pShape->arc.fArcWidthDeg) + 0.02f;
	pTransformData->bUseArc = eShape == particles::EMITTER_SHAPE_CYLINDER || eShape == particles::EMITTER_SHAPE_SPHERE;
	*pTransformHollow = *pTransformData;
	pTransformHollow->vColor.Assign(0.5f, 0.0f, 1.0f, 0.4f);

	pTransformHollow->mModel.LoadIdentity();
	pTransformHollow->mModel.MakeRotate(Math::DegToRad(pShape->baseShape.vRotation.x), -Math::DegToRad(pShape->baseShape.vRotation.y), Math::DegToRad(pShape->baseShape.vRotation.z));
	pTransformHollow->mModel.SetTranslation(pShape->baseShape.vPosition);
	pTransformHollow->mModel = pTransformHollow->mModel * mHollowScale;

	m_hollowObjectConstants.Unlock();
	m_hollowObjectConstants.UpdateData(pDevice);
	m_objectConstants.Unlock();
	m_objectConstants.UpdateData(pDevice);


	m_objectMat.UpdateDescriptors(pDevice);
	m_hollowObjectMat.UpdateDescriptors(pDevice);

	m_eShape = eShape;
}


void EditorShapes::MakeSphere(GFXDevice* pDevice)
{
	const float fRadius = 1.0f;	// Will be scaled in the vertexShader;
	const uint32 uSlices = 12;
	const uint32 uStacks = 12;

	uint32 uIndices		= (2 * ( uStacks - 1 ) * uSlices)*3;
	uint32 uVertices	= ( uStacks - 1 ) * uSlices + 2;

	PositionVertex* pVertices = NULL;
	ScratchObj<PositionVertex> scratchVertices(pVertices, uVertices, 4);
	uint16* puIndices = NULL;
	ScratchObj<uint16> scratchIndices(puIndices, uIndices, 4);

	float fSinI[uSlices];
	float fCosI[uSlices];
	float fSinJ[uStacks];
	float fCosJ[uStacks];

	for(uint32 i = 0; i < uSlices; i++)
	{
		sincosf(2.0f * Math::pi * i / uSlices, fSinI + i, fCosI + i);
	}

	for(uint32 j = 0; j < uStacks; j++)
	{
		sincosf(Math::pi * j / uStacks, fSinJ + j, fCosJ + j);
	}

	// Generate vertices
	PositionVertex* pVertex = pVertices;

	// +Z pole
	pVertex->x = 0.0f;
	pVertex->y = 0.0f;
	pVertex->z = fRadius;
	pVertex++;

	// Stacks
	for(uint32 j = 1; j < uStacks; j++)
	{
		for(uint32 i = 0; i < uSlices; i++)
		{
			Vector3f norm(fSinI[i]* fSinJ[j], fCosI[i]* fSinJ[j], fCosJ[j]);
			norm.Normalise();	// Shouldn't be necessary, but ensure accuracy
			Vector3f pos = norm*fRadius;

			pVertex->x = pos.x;
			pVertex->y = pos.y;
			pVertex->z = pos.z;

			pVertex++;
		}
	}

	// Z- pole
	pVertex->x = 0.0f;
	pVertex->y =0.0f;
	pVertex->z = -fRadius;
	pVertex++;

	// Generate indices
	uint16* puFace = puIndices;
	uint16 uRowA, uRowB;

	// Z+ pole
	uRowA = 0;
	uRowB = 1;

	for(uint32 i = 0; i < uSlices - 1; i++)
	{
		puFace[2] = (uint16)(uRowA);
		puFace[1] = (uint16)(uRowB + i + 1);
		puFace[0] = (uint16)(uRowB + i);
		puFace += 3;
	}

	puFace[2] = (uint16)(uRowA);
	puFace[1] = (uint16)(uRowB);
	puFace[0] = (uint16)(uRowB + (uSlices-1));
	puFace += 3;

	// Interior stacks
	for(uint32 j = 1; j < uStacks - 1; j++)
	{
		uRowA = 1 + (j - 1) * uSlices;
		uRowB = uRowA + uSlices;

		for(uint32 i = 0; i < uSlices - 1; i++)
		{
			puFace[2] = (uint16)(uRowA + i);
			puFace[1] = (uint16)(uRowA + i + 1);
			puFace[0] = (uint16)(uRowB + i);
			puFace += 3;

			puFace[2] = (uint16)(uRowA + i + 1);
			puFace[1] = (uint16)(uRowB + i + 1);
			puFace[0] = (uint16)(uRowB + i);
			puFace += 3;
		}

		puFace[2] = (uint16)(uRowA + uSlices - 1);
		puFace[1] = (uint16)(uRowA);
		puFace[0] = (uint16)(uRowB + uSlices - 1);
		puFace += 3;

		puFace[2] = (uint16)(uRowA);
		puFace[1] = (uint16)(uRowB);
		puFace[0] = (uint16)(uRowB + uSlices - 1);
		puFace += 3;
	}

	// Z- pole
	uRowA = 1 + (uStacks - 2) * uSlices;
	uRowB = uRowA + uSlices;

	for(uint32 i = 0; i < uSlices - 1; i++)
	{
		puFace[2] = (uint16)(uRowA + i);
		puFace[1] = (uint16)(uRowA + i + 1);
		puFace[0] = (uint16)(uRowB);
		puFace += 3;
	}

	puFace[2] = (uint16)(uRowA + uSlices - 1);
	puFace[1] = (uint16)(uRowA);
	puFace[0] = (uint16)(uRowB);

	m_sphere.vb.Init(pDevice, pVertices, sizeof(PositionVertex), uVertices, "EditorSphere");
	m_sphere.ib.Init(pDevice, puIndices, uIndices);
}
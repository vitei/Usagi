#ifndef _USG_PARTICLE_EDITOR_EDITOR_SHAPES_H_
#define _USG_PARTICLE_EDITOR_EDITOR_SHAPES_H_

#include "Engine/Graphics/Color.h"
#include "Engine/Graphics/Primitives/VertexBuffer.h"
#include "Engine/Graphics/Primitives/IndexBuffer.h"
#include "Engine/Graphics/Materials/Material.h"
#include "Engine/Graphics/Effects/ConstantSet.h"
#include "Engine/Particles/Scripted/EmitterShapes.h"

class EditorShapes : public usg::RenderNode
{
public:
	EditorShapes();
	~EditorShapes() {}

	void Init(usg::GFXDevice* pDevice, usg::Scene* pScene);
	void Cleanup(usg::GFXDevice* pDevice);
	void Update(usg::GFXDevice* pDEvice, usg::particles::EmitterShape eShape, const usg::particles::EmitterShapeDetails* pShape, float fElapsed);
	bool Draw(usg::GFXContext* pContext, RenderContext& renderContext) override;
	void Enable(bool bEnable) { m_bEnable = bEnable; }
	
private:
	void MakeSphere(usg::GFXDevice* pDevice);
	void MakeCube(usg::GFXDevice* pDevice);
	void MakeCylinder(usg::GFXDevice* pDevice);
	void MakeGrid(usg::GFXDevice* pDevice);

	struct RenderItem
	{
		usg::VertexBuffer	vb;
		usg::IndexBuffer	ib;
	};

	usg::Scene*			m_pScene;
	usg::RenderGroup*	m_pRenderGroup;
	RenderItem			m_grid;
	RenderItem			m_sphere;
	RenderItem			m_box;
	RenderItem			m_cylinder;
	RenderItem			m_line;
	usg::particles::EmitterShape m_eShape;

	usg::Material		m_objectMat;
	usg::Material		m_hollowObjectMat;
	usg::ConstantSet	m_objectConstants;
	usg::ConstantSet	m_hollowObjectConstants;
	usg::Material		m_gridMat;
	usg::ConstantSet	m_gridConstants;

	bool				m_bEnable;
};


#endif

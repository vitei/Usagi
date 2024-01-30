/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//  Description: Text which is transformed in world space before rendering
****************************************************************************/
#pragma once

#include "Engine/Common/Common.h"
#include "Engine/Layout/Fonts/Text.h"
#include "Engine/Framework/InstanceMgr.h"
#include "Engine/Scene/RenderNode.h"

namespace usg {

	class GFXDevice;
	class Scene;
	class PostFXSys;

	// FIXME: Ideally this should be a full UI rendering in 3D, however text is enough for now
	class Text3D : public RenderNode
	{
	public:
		Text3D();
		virtual ~Text3D();

		void Init(GFXDevice* pDevice, ResourceMgr* pRes, Scene* pScene, bool bDepthTest = true);
		void Cleanup(GFXDevice* pDevice);
		virtual bool Draw(GFXContext* pContext, RenderContext& renderContext) override;
		void SetWorldPosition(const Vector3f& vWorldPos);

		void Update(float deltaTime);
		void UpdateBuffers(GFXDevice* pDevice);

		// Fading out in the world
		void SetDepthRange(float fNear, float fFar);
		void DisableDepthCull();
		void SetOffset2D(const usg::Vector2f vOffset) { m_vOffset2D = vOffset;}

		void AddToScene(usg::GFXDevice* pDevice, bool bAdd);

		Text& GetText() { return m_text; }

	private:

		Text				m_text;
		Vector3f			m_vWorldPos;
		Vector2f			m_vOffset2D;
		float				m_fNear;
		float				m_fFar;
		ConstantSet			m_materialVSConstants;
		ConstantSet			m_materialGSConstants;
		DescriptorSet		m_descriptorSet;
		bool				m_bCanRender;
		usg::RenderGroup*	m_pRenderGroup;
		usg::Scene*			m_pScene;
	};

}

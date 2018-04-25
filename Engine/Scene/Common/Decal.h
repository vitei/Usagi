/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#pragma once

#ifndef USG_ENGINE_SCENE_DECALS
#define USG_ENGINE_SCENE_DECALS
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Materials/Material.h"
#include "Engine/Graphics/Primitives/VertexBuffer.h"
#include "Engine/Graphics/Primitives/IndexBuffer.h"
#include "Engine/Scene/RenderNode.h"
#include "Engine/Physics/CollisionQuadTree.h"

namespace usg
{

	class Decal : public RenderNode
	{
	public:
		Decal();
		virtual ~Decal();

		void Init(GFXDevice* pDevice, Scene* pScene, TextureHndl pTexture, uint32 uMaxTriangles = 30, float fDepthBias = -30.f);
		void UpdateBuffers(GFXDevice* pDevice);
		virtual bool Draw( GFXContext* pContext, PostFXSys* pPostFXSys );

		// Be sure to call in the following order when creating a decal
		void AddToScene(Scene* pScene, bool bAdd);
		void SetContents( GFXDevice* pDevice, const usg::Sphere* pBounds, const VertexBuffer* pBuffer, const uint16* pIndices, uint32 uIndices );
		void SetMatrix(const Matrix4x4& mProj, const Matrix4x4 &mView);

		void SetOpacity( float opacity );
		void SetTexture(usg::GFXDevice* pDevice, TextureHndl pTexture);
		bool IsVisible() { return m_pTransformNode!= NULL; }


	private:
		uint32				m_uMaxTriangles;
		Material			m_material;
		ConstantSet			m_projConsts;
		ConstantSet			m_pixelConstants;
		SamplerHndl			m_sampler;
		IndexBuffer			m_indexBuffer;
		uint32				m_indicesNum;
		// TODO: Construct a new vertex buffer with triangles shrunk to the appropriate size
		// rather than just referencing the existing one
		RenderGroup*		m_pRenderGroup;
		TransformNode*		m_pTransformNode;
		const VertexBuffer*	m_pVertexBuffer;
	};
} // namespace usg


#endif // USG_ENGINE_SCENE_DECALS

/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: A basic renderable element
*****************************************************************************/
#ifndef USG_GRAPHICS_OFFSCREEN_RENDER_NODE_H
#define USG_GRAPHICS_OFFSCREEN_RENDER_NODE_H

#include "Engine/Maths/Matrix4x4.h"
#include "Engine/Graphics/Device/PipelineState.h"
#include "Engine/Graphics/RenderConsts.h"

namespace usg
{

	class OffscreenRenderNode
	{
	public:
		OffscreenRenderNode();
		virtual ~OffscreenRenderNode();

		virtual bool Draw(GFXContext* pContext) = 0;

	private:
		PRIVATIZE_COPY(OffscreenRenderNode);

	};

}


#endif


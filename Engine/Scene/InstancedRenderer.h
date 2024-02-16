/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Base class for a manager which renders multiple instances of geo
*****************************************************************************/
#pragma once

#include "Engine/Maths/Matrix4x4.h"
#include "Engine/Graphics/Device/PipelineState.h"
#include "Engine/Graphics/RenderConsts.h"
#include "Engine/Scene/RenderGroup.h"
#include "Engine/Scene/RenderNode.h"

namespace usg{

class Scene;
class TransformNode;
class RenderGroup;
class GFXContext;
class PostFXSys;
class Texture;

class InstancedRenderer
{
public:
	InstancedRenderer() {}
	virtual ~InstancedRenderer() {}

	virtual void Cleanup(GFXDevice* pDevice) = 0;
	virtual uint64 GetInstanceId() = 0;
	virtual void Draw(GFXContext* pContext, RenderNode::RenderContext& renderContext, uint32 uDrawId) = 0;
	virtual void AddNode(RenderNode* pNode) = 0;
	virtual RenderNode* EndBatch() = 0;
	virtual void PreDraw(GFXDevice* pDevice) = 0;
	virtual void DrawFinished() = 0;

private:

};

}


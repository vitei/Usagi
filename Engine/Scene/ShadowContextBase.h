/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: A context for shadow depth rendering
*****************************************************************************/
#pragma once

#include "Engine/Memory/FastPool.h"
#include "Engine/Scene/SceneContext.h"
#include "Engine/Scene/SceneSearchObject.h"
#include "Engine/Graphics/Device/DescriptorSet.h"

namespace usg{

class Scene;
class Camera;
class PostFXSys;

class ShadowContextBase : public SceneContext
{
public:
	typedef SceneContext Inherited;

	virtual void ClearLists();
	void CacheDirtyInfo();
	bool IsDirty() const;

protected:
	virtual usg::Matrix4x4 GetLightMat() const = 0;

	struct ComparisonData
	{
		memsize			CmpNode;	// TODO: Create Ids
		usg::Matrix4x4	CmpLoc;
	};

	list<RenderNode*>		m_drawList;
	vector<ComparisonData>	m_prevData;
};

}



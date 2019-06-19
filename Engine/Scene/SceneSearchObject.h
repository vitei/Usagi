/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: An octree search callback for
*****************************************************************************/
#ifndef _USG_GRAPHICS_SCENE_SCENESEARCHNODE_H_
#define _USG_GRAPHICS_SCENE_SCENESEARCHNODE_H_

#include "Engine/Scene/Octree.h"

namespace usg{

class Scene;
class SceneContext;

class SceneSearchFrustum : public Octree::SearchFrustum
{
public:
	SceneSearchFrustum()
	{
		m_pScene = NULL;
		m_pContext = NULL;
	}

	~SceneSearchFrustum() {}

	void Init(Scene* pScene, SceneContext* pContext, uint32 uMask)
	{
		m_pScene	= pScene;
		m_pContext	= pContext;
		InitInt(uMask);
	}

	virtual void Callback(void* pUserData);

private:
	Scene*			m_pScene;
	SceneContext*	m_pContext;
};


class SceneSearchSphere : public Octree::SearchSphere
{
public:
	SceneSearchSphere()
	{
		m_pScene = NULL;
		m_pContext = NULL;
	}

	~SceneSearchSphere() {}

	void Init(Scene* pScene, SceneContext* pContext, const Sphere& sphere, uint32 uMask)
	{
		m_pScene	= pScene;
		m_pContext	= pContext;
		InitInt(uMask);
		SetSphere(&sphere);

	}

	virtual void Callback(void* pUserData);

private:
	Scene*			m_pScene;
	SceneContext*	m_pContext;
};

}

#endif



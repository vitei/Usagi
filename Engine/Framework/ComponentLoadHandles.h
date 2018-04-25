/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _FRAMEWORK_COMPONENT_LOAD_HANDLES_
#define _FRAMEWORK_COMPONENT_LOAD_HANDLES_
#include "Engine/Common/Common.h"

namespace usg
{
	class GFXDevice;
	class Scene;
	class ResourceMgr;
	class ModelMgr;

	namespace physics
	{
		struct PhysicsSceneData;
	}

	struct ComponentLoadHandles
	{
		ComponentLoadHandles() { pDevice = nullptr; pScene = nullptr; pResourceMgr = nullptr; pPhysicsScene = nullptr; pModelMgr = nullptr; }

		GFXDevice*					pDevice;
		Scene*						pScene;
		ResourceMgr*				pResourceMgr;
		physics::PhysicsSceneData*	pPhysicsScene;
		ModelMgr*					pModelMgr;
	};

}

#endif

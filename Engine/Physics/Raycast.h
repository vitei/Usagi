/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2016
****************************************************************************/

#pragma once

#include "Engine/Maths/Vector3f.h"

namespace physx
{
	class PxGeometry;
}

namespace usg
{
	namespace Components
	{
		typedef struct _PhysicsScene PhysicsScene;
	}

	class EventManager;
	class SystemCoordinator;

	struct AsyncRaycastRequest
	{
		Vector3f vFrom;
		Vector3f vTo;
		uint32 uFilter = 0xffffffff;
		uint32 uMaxHits = 16;
		uint32 uRaycastId = 0;
	};

	struct AsyncSweepRequest
	{
		uint32 uFilter = 0xffffffff;
		uint32 uMaxHits = 16;
		uint32 uRaycastId = 0;
		physx::PxGeometry* pGeo = nullptr;
		usg::Matrix4x4 mTrans;
		Vector3f vDir = Vector3f(0.0f, 0.0f, 1.0f);
		float fDist = 0.0f;
	};

	void ExecuteRaycasts(Required<usg::PhysicsScene> scene, EventManager& eventManager, SystemCoordinator& systemCoordinator);
}
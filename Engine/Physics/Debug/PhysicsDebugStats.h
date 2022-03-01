/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Stats which should always be available regardless of the mode
*****************************************************************************/
#pragma once
#ifndef USG_DEBUG_PHYSICS_DEBUG_STATS_H
#define USG_DEBUG_PHYSICS_DEBUG_STATS_H

#include "Engine/Debug/Rendering/IDebugStatGroup.h"
#include "Engine/Graphics/Color.h"

namespace physx
{
	class PxScene;
}

namespace usg
{
	class Debug3D;

	class PhysicsDebugStats : public IDebugStatGroup
	{
	public:
		PhysicsDebugStats();
		~PhysicsDebugStats();

		void Init(physx::PxScene* pScene);
		void Cleanup(GFXDevice* pDevice);
		void Update(float fElapsed) override;

		void Draw(DebugRender* pRender) override;
		void PreDraw(GFXDevice* pDevice) override;
		void PostDraw(GFXDevice* pDevice) override;
		void SetActive(bool bActive) override;
		void SetPage(uint32 uPage) override;

		uint32 GetPageCount() const { return PAGE_COUNT; }


	private:
		void DrawPhysicsPage(DebugRender* pRender);


		enum GLOBAL_PAGES
		{
			PAGE_COLLISION = 0,
			PAGE_JOINTS,
			PAGE_COUNT
		};

		physx::PxScene* m_pPhysicsScene;

	};

}

#endif // USG_DEBUG_STAT_GROUP_H

/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Stats which should always be available regardless of the mode
*****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Color.h"
#include "Engine/Debug/Rendering/DebugRender.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Core/Timer/ProfilingTimer.h"
#include "Engine/Scene/SceneContext.h"
#include "Engine/Physics/PhysX.h"
#include "Engine/Scene/Scene.h"
#include "PhysicsDebugStats.h"

#ifndef FINAL_BUILD

namespace usg {

	PhysicsDebugStats::PhysicsDebugStats() :
		IDebugStatGroup(),
		m_pPhysicsScene(nullptr)
	{

	}

	PhysicsDebugStats::~PhysicsDebugStats()
	{

	}

	void PhysicsDebugStats::Init(physx::PxScene* pScene)
	{
		m_pPhysicsScene = pScene;
	}

	void PhysicsDebugStats::Cleanup(GFXDevice* pDevice)
	{
	
	}

	void PhysicsDebugStats::Update(float fElapsed)
	{

	}

	void PhysicsDebugStats::Draw(DebugRender* pRender)
	{
		switch (m_uActivePage)
		{
		case PAGE_COLLISION:
		case PAGE_JOINTS:
			DrawPhysicsPage(pRender);
			break;
		default:
			ASSERT(false);
		}
	}

	void PhysicsDebugStats::PreDraw(GFXDevice* pDevice)
	{

	}

	void PhysicsDebugStats::PostDraw(GFXDevice* pDevice)
	{

	}

	void PhysicsDebugStats::SetActive(bool bActive)
	{
		if (!bActive)
		{
			m_pPhysicsScene->setVisualizationParameter(physx::PxVisualizationParameter::eCOLLISION_SHAPES, 0.0f);
			m_pPhysicsScene->setVisualizationParameter(physx::PxVisualizationParameter::eJOINT_LIMITS, 0.f);
			m_pPhysicsScene->setVisualizationParameter(physx::PxVisualizationParameter::eJOINT_LOCAL_FRAMES, 0.0f);
			m_pPhysicsScene->setVisualizationParameter(physx::PxVisualizationParameter::eSCALE, 0.0f);

		}
		IDebugStatGroup::SetActive(bActive);
	}


	void PhysicsDebugStats::SetPage(uint32 uPage)
	{
		if (m_pPhysicsScene)
		{
			m_pPhysicsScene->setVisualizationParameter(physx::PxVisualizationParameter::eCOLLISION_SHAPES, uPage == PAGE_COLLISION ? 1.0f : 0.0f);
			m_pPhysicsScene->setVisualizationParameter(physx::PxVisualizationParameter::eJOINT_LIMITS, uPage == PAGE_JOINTS ? 1.0f : 0.f);
			m_pPhysicsScene->setVisualizationParameter(physx::PxVisualizationParameter::eJOINT_LOCAL_FRAMES, uPage == PAGE_JOINTS ? 1.0f : 0.0f);
			//pPhysicsScene->setVisualizationParameter(physx::PxVisualizationParameter::eBODY_AXES, 1.0f);
			m_pPhysicsScene->setVisualizationParameter(physx::PxVisualizationParameter::eSCALE, 1.0f);
		}
		IDebugStatGroup::SetPage(uPage);
	}


	void PhysicsDebugStats::DrawPhysicsPage(DebugRender* pRender)
	{
		Color cStatCol(0.0f, 1.0f, 1.0f, 1.0f);

		if (!m_pPhysicsScene)
		{
			pRender->AddString("Physics system not set", 0.0f, 2.0f, cStatCol);
			return;
		}
		else
		{
			// TODO: Display debug type
			pRender->AddString("Physics debug", 0.0f, 2.0f, cStatCol);
		}


		const physx::PxRenderBuffer& rb = m_pPhysicsScene->getRenderBuffer();

		for (physx::PxU32 i = 0; i < rb.getNbLines(); i++)
		{
			const physx::PxDebugLine& line = rb.getLines()[i];
			// render the line
			usg::Vector3f vStart(line.pos0.x, line.pos0.y, line.pos0.z);
			usg::Vector3f vEnd(line.pos1.x, line.pos1.y, line.pos1.z);
			usg::Color color; color.AssignRGBA32(line.color0);
			Debug3D::GetRenderer()->AddLine(vStart, vEnd, color, 0.01f);
		}

		for (physx::PxU32 i = 0; i < rb.getNbTriangles(); i++)
		{
			const physx::PxDebugTriangle& tri = rb.getTriangles()[i];

			usg::Vector3f pos0(tri.pos0.x, tri.pos0.y, tri.pos0.z);
			usg::Vector3f pos1(tri.pos1.x, tri.pos1.y, tri.pos1.z);
			usg::Vector3f pos2(tri.pos2.x, tri.pos2.y, tri.pos2.z);

			usg::Color color0; color0.AssignRGBA32(tri.color0);
			usg::Color color1; color1.AssignRGBA32(tri.color1);
			usg::Color color2; color2.AssignRGBA32(tri.color2);

			Debug3D::GetRenderer()->AddTriangle(pos0, color0, pos1, color1, pos2, color2);

		}

	}

}

#endif

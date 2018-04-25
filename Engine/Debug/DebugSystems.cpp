/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Framework/GameComponents.h"
#include "Engine/Framework/Signal.h"
#include "Engine/Framework/SystemCategories.h"
#include "Engine/Framework/FrameworkComponents.pb.h"
#include "Engine/Physics/CollisionData.pb.h"
#include "Engine/Debug/DebugStats.h"
#include "Engine/Debug/Rendering/DebugRender.h"
#include "Engine/Debug/DebugComponents.pb.h"
#include "Engine/Framework/SystemCoordinator.h"

namespace usg
{
	
	class DebugShapeRenderer
	{
	protected:
		static void RenderDebugShape(const DebugRenderData& debug, const usg::Components::Sphere& sphere, const Optional<MatrixComponent>& matrix)
		{
#ifndef FINAL_BUILD
			if (DebugStats::Inst()->GetCurrentType() != debug.page) { return; }

			const Vector3f centre = matrix.Exists()
				? matrix.Force()->matrix.TransformVec3(sphere.centre)
				: sphere.centre;

			DEBUG_PRINT("Rendering sphere at %0.02f %0.02f %0.02f \n", centre.x, centre.y, centre.z);
			Debug3D::GetRenderer()->AddSphere(centre, sphere.radius, debug.color);
#endif
		}
	};

	template<typename Shape>
	class RenderDebug : public System, public DebugShapeRenderer
	{
	public:
		typedef Shape ShapeTP;
		struct Inputs
		{
			Optional<MatrixComponent> matrix;

			Required<DebugRenderData> debug;
			Required<ShapeTP>         shape;
		};

		static const SystemCategory CATEGORY = SYSTEM_DEFAULT_PRIORITY;
		static const char* Name()
		{
			static const char* const name = "RenderDebug"; // Would be good to include the shape name
			return name;
		}

		static uint32 GetSystemId()
		{
			return usg::GetSystemId<RenderDebug<Shape> >();
		}

		static bool GetInputOutputs(ComponentGetter& GetComponent, Inputs& inputs, Outputs& outputs)
		{
			return GetComponent(inputs.matrix) &&
				GetComponent(inputs.debug) &&
				GetComponent(inputs.shape);
		}

		static void Run(const Inputs& in, Outputs&, float)
		{
			RenderDebugShape(*in.debug, *in.shape, in.matrix);
		}
	};

	namespace Systems {
		typedef RenderDebug<usg::Components::Sphere> RenderDebugSphere;
	}
}

#include GENERATED_SYSTEM_CODE(Engine/Debug/DebugSystems.cpp)

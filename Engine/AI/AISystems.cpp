#include "Engine/Common/Common.h"
#include "Engine/Framework/FrameworkComponents.pb.h"
#include "Engine/Physics/PhysicsEvents.pb.h"
#include "Engine/Framework/System.h"
#include "Engine/Framework/GameComponents.h"
#include "Engine/Framework/SystemCategories.h"
#include "Engine/AI/AgentNavigationUtil.h"
#include "Engine/Scene/Common/SceneComponents.pb.h"
#include "Engine/Physics/Raycast.h"
#include "Engine/Physics/CollisionQuadTree.h"
#include "Engine/AI/AgentNavigationUtil.h"
#include "Engine/AI/AgentNavigation.h"

namespace usg
{
	namespace ai
	{
		namespace Systems
		{

			class UpdateNavigationSystem : public usg::System, public usg::ai::AgentNavigationUtil
			{
			public:
				struct Inputs
				{
					Required<usg::MatrixComponent> mtx;
				};

				struct Outputs
				{
					Required<AgentNavigationComponent> navigation;
				};

				DECLARE_SYSTEM(usg::SYSTEM_EARLY)
				static void Run(const Inputs& inputs, Outputs& outputs, float fDelta)
				{
					AgentNavigationComponent& nav = outputs.navigation.Modify();
					UpdateNavigation(nav, inputs.mtx->matrix.vPos().v3(), inputs.mtx->matrix.vFace().v3(), inputs.mtx->matrix.vRight().v3());
					ResetNavigation(nav);
				}

			};

		}
	}

}

#include GENERATED_SYSTEM_CODE(Engine/AI/AISystems.cpp)
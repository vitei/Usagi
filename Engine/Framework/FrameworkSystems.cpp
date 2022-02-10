/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include OS_HEADER(Engine/Core/Timer, TimeTracker.h)
#include "Engine/Maths/MathUtil.h"
#include "Engine/Framework/Signal.h"
#include "Engine/Framework/ComponentEntity.h"
#include "Engine/Framework/EventManager.h"
#include "Engine/Core/stl/utility.h"
#include "Engine/Framework/Signal.h"
#include "Engine/Framework/HealthModifier.h"
#include "Engine/Framework/FrameworkComponents.pb.h"
#include "Engine/Scene/Model/ModelComponents.pb.h"
#include "Engine/Framework/FrameworkEvents.pb.h"
#include "Engine/Physics/PhysicsComponents.pb.h"
#include "Engine/Scene/Common/SceneEvents.pb.h"
#include "Engine/Physics/CollisionData.pb.h"

namespace usg
{
	namespace Systems
	{

		class EntityStateEventHandler : public usg::System
		{
		public:
			struct Outputs
			{
				Required<usg::StateComponent> state;
			};

			DECLARE_SYSTEM(usg::SYSTEM_PRE_EARLY)
			static void OnEvent(const Inputs& inputs, Outputs& outputs, const KillEntityEvent& killEntity)
			{
				outputs.state.Modify().current = STATUS_DEAD;
			}
		};


		class KillAfterTime : public usg::System
		{
		public:
			struct Inputs
			{
				Required<Lifetime>		lifetime;
				Required<MaxLifetime>	maxLife;
			};

			struct Outputs
			{
				Required<StateComponent> state;
				Optional<HealthComponent> health;
			};

			DECLARE_SYSTEM(SYSTEM_DEFAULT_PRIORITY)
			static void Run(const Inputs& inputs, Outputs& outputs, float fDelta)
			{
				if (inputs.lifetime->fLifetime > inputs.maxLife->fMaxLifetime)
				{
					if (inputs.maxLife->bSetHealthToZero)
					{
						ASSERT(outputs.health.Exists());
						outputs.health.Force().Modify().fLife = 0;
					}
					else
					{
						outputs.state.Modify().current = STATUS_DEAD;
					}
				}
			}
		};

		// Catch all the objects not shifted by the physics system
		class OriginShiftNonPhysicsBodies : public usg::System 
		{
		public:

			struct Inputs
			{
				Required<TransformComponent> transform;
			};

			struct Outputs
			{
				Required<TransformComponent> transform;
			};

			DECLARE_SYSTEM(SYSTEM_DEFAULT_PRIORITY)


			EXCLUSION(IfHas<TransformComponent, FromParents>, IfHas<RigidBody, FromSelfOrParents>)

			static void OnEvent(const Inputs& inputs, Outputs& outputs, const ShiftWorldOrigin& evt)
			{
				outputs.transform.Modify().position = inputs.transform->position - evt.vShift;
			}
		};

		class MeasureDistance : public usg::System
		{
		public:
			struct Inputs
			{
				Required<RigidBody> rigidBody;
				Required<DistanceTravelled> distance;
			};

			struct Outputs
			{
				Required<DistanceTravelled> distance;
			};

			DECLARE_SYSTEM(SYSTEM_DEFAULT_PRIORITY)
			static void Run(const Inputs& inputs, Outputs& outputs, float fDelta)
			{
				DistanceTravelled& dist = outputs.distance.Modify();
				const float fFrameDist = physics::GetLinearVelocity(inputs.rigidBody).Magnitude()*fDelta;
				dist.fDistance += fFrameDist;
			}
		};

		struct MeasureLifetime : public usg::System
		{
			struct Inputs
			{
				Required<Lifetime> lifetime;
				Required<usg::SimulationActive, FromParents> simactive;
			};

			struct Outputs
			{
				Required<Lifetime> lifetime;
			};

			DECLARE_SYSTEM(SYSTEM_DEFAULT_PRIORITY)

			static void Run(const Inputs& inputs, Outputs& outputs, float fDelta)
			{
				if (!inputs.simactive->bActive)
					return;
				outputs.lifetime.Modify().fLifetime = inputs.lifetime->fLifetime + fDelta;
			}
		};

		class OscillateSpring : public usg::System
		{
		public:
			struct Inputs
			{
				Required<SpringComponent> spring;
			};

			struct Outputs
			{
				Required<SpringComponent> spring;
			};

			DECLARE_SYSTEM(usg::SYSTEM_DEFAULT_PRIORITY)

			static void Run(const Inputs& inputs, Outputs& outputs, float fDelta)
			{
				SpringComponent& spring = outputs.spring.Modify();

				if (spring.bOscillating)
				{
					spring.fAngle += spring.fKineticEnergy;

					spring.fTransfer = (spring.fPotentialEnergy - 0.5f) * (1.0f - spring.fAngle) * spring.fTransferRate;
					spring.fPotentialEnergy -= spring.fTransfer;
					spring.fKineticEnergy += spring.fTransfer;

					spring.fOutput = (spring.fAngle - 1.0f) * spring.fDampening;

					if (spring.fDampening > 0.0f)
					{
						spring.fDampening -= fDelta;
					}
					else
					{
						SpringComponent_init(&(spring));
					}
				}
			}
		};

		class ToggleVisiblity : public System
		{
		public:
			struct Outputs
			{
				Required<VisibilityComponent>      visibility;
			};

			DECLARE_SYSTEM(SYSTEM_POST_GAMEPLAY)

			static void OnEvent(const Inputs& inputs, Outputs& outputs, const ToggleVisibility& evt)
			{
				outputs.visibility.Modify().bVisible = evt.bVisible;
			}
		};

		class ConstructWorldMatrix : public System
		{
		public:
			struct Inputs
			{
				Optional<MatrixComponent, FromParents>      parentMtx; // Optional parent matrix
				Required<TransformComponent>				tran;
				Optional<ScaleComponent>					scale;
			};

			struct Outputs
			{
				Required<MatrixComponent>      worldMtx;
			};

			DECLARE_SYSTEM(SYSTEM_TRANSFORM)

			static  void Run(const Inputs& inputs, Outputs& outputs, float fDelta)
			{
				Matrix4x4& mOut = outputs.worldMtx.Modify().matrix;
				mOut = inputs.tran->rotation;
				mOut.Translate(inputs.tran->position.x, inputs.tran->position.y, inputs.tran->position.z);

				if(inputs.parentMtx.Exists() && inputs.tran->bInheritFromParent)
				{
					mOut = mOut * inputs.parentMtx.Force()->matrix;
				}

				if (inputs.scale.Exists())
				{
					const Vector3f& s = inputs.scale.Force()->scale;
					mOut._11 *= s.x;
					mOut._12 *= s.x;
					mOut._13 *= s.x;
					mOut._14 *= s.x;

					mOut._21 *= s.y;
					mOut._22 *= s.y;
					mOut._23 *= s.y;
					mOut._24 *= s.y;

					mOut._31 *= s.z;
					mOut._32 *= s.z;
					mOut._33 *= s.z;
					mOut._34 *= s.z;
				}
			}
		};

		class TurnEntity : public usg::System
		{
		public:
			struct Inputs
			{
				Required<usg::TurnableComponent>	turnable;
				Required<usg::SimulationActive, FromParents> simactive;
				Optional<usg::BoneComponent>		bone;
			};

			struct Outputs
			{
				Required<usg::TurnableComponent>	turnable;
				Required<usg::TransformComponent>	trans;
			};

			DECLARE_SYSTEM(usg::SYSTEM_DEFAULT_PRIORITY)

			static void UpdateAngle(TurnableComponent& turnable, float fDirection, float fDelta)
			{
				turnable.fPrevAngle = turnable.fAngle;
				turnable.fAngle += fDirection;

				if(turnable.fMaxAngle < 360.f)
				{
					turnable.fAngle = Math::Clamp<float>(turnable.fAngle, turnable.fMinAngle, turnable.fMaxAngle);
				}

				if(turnable.fAngle >= 360.0f)
				{
					turnable.fPrevAngle -= 720.f;
					turnable.fAngle -= 720.f;
				}
				else if(turnable.fAngle < -360.0f)
				{
					turnable.fPrevAngle += 720.f;
					turnable.fAngle += 720.f;
				}

				if(fabsf(turnable.fAngle) > 720.f)
				{
					// Hack fix for weird bug I'm getting, ideally should be removed
					turnable.fAngle = 0.0f;
					turnable.fPrevAngle = 0.0f;
				}
			}
			static void Run(const Inputs& inputs, Outputs& outputs, float fDelta)
			{
				if (!inputs.simactive->bActive)
					return;
				UpdateAngle(outputs.turnable.Modify(), inputs.turnable->fDirection, fDelta);
				{
					outputs.trans.Modify().rotation = Quaternionf(inputs.turnable->vAxis.v3(), -Math::DegToRad(inputs.turnable->fAngle));
					if(inputs.bone.Exists())
					{
						outputs.trans.Modify().position = inputs.bone.Force()->m_translate;
					}
				}
			}
		};



		class HandleHealthEvents : public usg::System, public usg::HealthModifier
		{
		public:
			struct Inputs
			{
				Required<usg::EntityID>			self;
				Optional<LocalSim>				localSim;
				Required<EventManagerHandle, FromSelfOrParents> eventManager;
			};

			struct Outputs
			{
				Required<usg::HealthComponent> health;
			};

			DECLARE_SYSTEM(usg::SYSTEM_DEFAULT_PRIORITY)
			static void OnEvent(const Inputs& inputs, Outputs& outputs, const IncreaseHealthEvent& increaseHealth)
			{
				if (!inputs.localSim.Exists())
					return;
				AddHealth(inputs.self->id, outputs.health.Modify(),
				          inputs.eventManager->handle, increaseHealth.amount);
			}
			static void OnEvent(const Inputs& inputs, Outputs& outputs, const DecreaseHealthEvent& decreaseHealth)
			{
				if (!inputs.localSim.Exists())
					return;	
				AddDamage(inputs.self->id, outputs.health.Modify(),
				          inputs.eventManager->handle, decreaseHealth.amount, decreaseHealth.uDamageCauserTeam, decreaseHealth.iDamageCauserNUID);
			}
			static void OnEvent(const Inputs& inputs, Outputs& outputs, const SetHealthEvent& setHealth)
			{
				if (!inputs.localSim.Exists())
					return;
				SetHealth(inputs.self->id, outputs.health.Modify(),
				          inputs.eventManager->handle, setHealth.amount);
			}
		};


		class UpdateSystemTime : public usg::System
		{
		public:
			struct Outputs
			{
				Required<usg::SystemTimeComponent> sysTime;
			};

			DECLARE_SYSTEM(usg::SYSTEM_PRE_EARLY)

			static void Run(const Inputs& in, Outputs& outputs, float dt)
			{
				outputs.sysTime.Modify().fSystemElapsed = outputs.sysTime.GetRuntimeData().pTimeTracker->GetDeltaTime();
			}
		};

	}
}

#include GENERATED_SYSTEM_CODE(Engine/Framework/FrameworkSystems.cpp)

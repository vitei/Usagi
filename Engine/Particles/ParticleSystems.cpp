/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Maths/Matrix4x4.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/Physics/PhysicsEvents.pb.h"
#include "Engine/Scene/Common/SceneComponents.pb.h"
#include "Engine/Framework/FrameworkComponents.pb.h"
#include "Engine/Particles/ParticleEvents.pb.h"
#include "Engine/Particles/ParticleComponents.pb.h"


namespace usg
{

	namespace Systems
	{
		class UpdateParticleEffect : public System
		{
		public:
			struct Inputs
			{
				Required< MatrixComponent,
					FromSelfOrParents > mtx;
				Required< SceneComponent, FromSelfOrParents > scene;	// Fixme is actually an output so should be sending messages
				Required< ParticleComponent > particle;
			};

			struct Outputs
			{
				Required< ParticleComponent > particle;
			};

			DECLARE_SYSTEM(SYSTEM_POST_TRANSFORM)
			static void Run(const Inputs& inputs, Outputs& outputs, float fDelta)
			{
				ParticleEffectHndl& p = outputs.particle.GetRuntimeData().hndl;
				if (inputs.particle->bSpawn)
				{
					if (!inputs.particle.GetRuntimeData().hndl.IsValid())
					{
						if (inputs.particle->bIsScripted)
						{
							inputs.scene.GetRuntimeData().pScene->CreateScriptedEffect(p, inputs.mtx->matrix, inputs.particle->name, inputs.particle->vVelocity, inputs.particle->fScale);
						}
						else
						{
							inputs.scene.GetRuntimeData().pScene->CreateEffect(p, inputs.mtx->matrix, inputs.particle->name);
						}
					}
					outputs.particle.Modify().bSpawn = false;
				}
				p.SetWorldMat(inputs.mtx->matrix);
			}

			static void OnEvent(const Inputs& inputs, Outputs& outputs, const ParticleEffectStart& startEvent)
			{
				outputs.particle.Modify().bSpawn = true;
				outputs.particle.Modify().fScale = startEvent.fScale;
			}

			static void OnEvent(const Inputs& inputs, Outputs& outputs, const ParticleEffectStop& stopEvent)
			{
				outputs.particle.GetRuntimeData().hndl.Kill(stopEvent.bForce);
			}

			static void OnEvent(const Inputs& inputs, Outputs& outputs, const ParticleEnableEmission& emission)
			{
				outputs.particle.GetRuntimeData().hndl.EnableEmission(emission.bEnable);
			}

			static void OnEvent(const Inputs& inputs, Outputs& outputs, const SetLinearVelocity& evt)
			{
				outputs.particle.Modify().vVelocity = evt.vVelocity;
			}

		};
	}
}


#include GENERATED_SYSTEM_CODE(Engine/Particles/ParticleSystems.cpp)
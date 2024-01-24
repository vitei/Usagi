#include "Engine/Common/Common.h"
#include "Engine/Framework/ComponentEntity.h"
#include "Engine/Framework/Signal.h"
#include "Engine/Audio/Audio.h"
#include "Engine/Audio/MusicManager.h"
#include "Engine/Audio/AudioComponents.pb.h"
#include "Engine/Framework/FrameworkComponents.pb.h"
#include "Engine/Physics/PhysicsComponents.pb.h"
#include "Engine/Audio/AudioEvents.pb.h"

namespace usg
{

	namespace Systems
	{

		class ModulatePitch : public usg::System
		{
		public:
			struct Inputs
			{
				Required<SoundComponent>	sound;
				Required<PitchModulator>	pitch;
			};

			struct Outputs
			{
				Required<SoundComponent>	sound;
				Required<PitchModulator>	pitch;
			};

			DECLARE_SYSTEM(usg::SYSTEM_DEFAULT_PRIORITY)

			static void Run(const Inputs& inputs, Outputs& outputs, float fDelta)
			{
				float fCountDown = inputs.pitch->fCountdown - fDelta;
				float fPrevPitch = inputs.pitch->fPrevPitch;
				float fTargetPitch = inputs.pitch->fTargetPitch;
				float fLerpTime = inputs.pitch->fLerpTime;

				if (inputs.sound.GetRuntimeData().hndl.IsPlaying())
				{
					PitchModulator& pitchOut = outputs.pitch.Modify();
					if (fCountDown <= 0.0f)
					{
						fCountDown = Math::RangedRandom(inputs.pitch->fVariationTime - inputs.pitch->fVariationRand, inputs.pitch->fVariationTime + inputs.pitch->fVariationRand);
						fLerpTime = fCountDown;
						fPrevPitch = fTargetPitch;
						fTargetPitch = inputs.sound.GetRuntimeData().hndl.GetRandomPitch();

						pitchOut.fTargetPitch = fTargetPitch;
						pitchOut.fPrevPitch = fPrevPitch;
						pitchOut.fLerpTime = fCountDown;
					}
					float fPitch = Math::Lerp(fTargetPitch, fPrevPitch, fCountDown / fLerpTime);
					float fAdditionalPitch = Math::AccelerateToValue(inputs.pitch->fAdditionalPitch, inputs.pitch->fTargetAdditionalPitch, fDelta * inputs.pitch->fAdditionalPitchChangeRate);
					outputs.pitch.Modify().fAdditionalPitch = fAdditionalPitch;


					outputs.sound.GetRuntimeData().hndl.SetPitch(fPitch + fAdditionalPitch);


					if (inputs.pitch->fTargetVolume >= 0.0f)
					{
						float fCurrentVolume = inputs.sound.GetRuntimeData().hndl.GetVolume();
						fCurrentVolume = Math::AccelerateToValue(fCurrentVolume, inputs.pitch->fTargetVolume, fDelta * inputs.pitch->fVolumeChangeRate);
						outputs.sound.GetRuntimeData().hndl.SetVolume(fCurrentVolume);

					}

					pitchOut.fCountdown = fCountDown;
				}
			}

		};


		class TriggerMusic : public usg::System
		{
		public:
			struct Inputs
			{
				Required<usg::EntityID>		self;
				Required<MusicComponent>	sounds;
			};

			struct Outputs
			{
				Required<MusicComponent>	sounds;
			};

			DECLARE_SYSTEM(usg::SYSTEM_DEFAULT_PRIORITY)

			static void OnEvent(const Inputs& inputs, Outputs& outputs, const PlayMusic &evt)
			{
				DoPlayMusic(evt.uAudioID);
			}

			static void OnEvent(const Inputs& inputs, Outputs& outputs, const PauseMusic &evt)
			{
				MusicManager::Inst()->PauseMusic();
			}

			static void OnEvent(const Inputs& inputs, Outputs& outputs, const RestartMusic &evt)
			{
				MusicManager::Inst()->RestartMusic();
			}

			static void OnEvent(const Inputs& inputs, Outputs& outputs, const StopMusic &evt)
			{
				MusicManager::Inst()->StopMusic(MusicManager::FADE_TYPE_FADE);
			}

			static void DoPlayMusic(const uint32_t uAudioID)
			{
				MusicManager::Inst()->PlayMusic(uAudioID, 0.3f, MusicManager::FADE_TYPE_WAIT, MusicManager::FADE_TYPE_FADE);
			}

		private:
		};

		class UpdateSoundActor : public System
		{
		public:

			struct Inputs
			{
				Required<MatrixComponent, FromSelfOrParents>	mtx;
				Required<SoundActorComponent>					actor;
				Optional<RigidBody, FromSelfOrParents>			rigidBody;
			};

			struct Outputs
			{
				Required<SoundActorComponent>	actor;
			};

			DECLARE_SYSTEM(usg::SYSTEM_POST_TRANSFORM)

			static  void	 LateUpdate(const Inputs& inputs, Outputs& outputs, float fDelta)
			{
				SoundActorHandle& actor = outputs.actor.GetRuntimeData().hndl;
				actor.SetPosition(inputs.mtx->matrix.vPos().v3());
				if (inputs.rigidBody.Exists())
				{
					actor.SetVelocity(physics::GetLinearVelocity(inputs.rigidBody));
				}
			}
		};

		class HandleSoundEvents : public System
		{
		public:

			struct Inputs
			{
				Required<MatrixComponent, FromSelfOrParents> mtx;
			};

			struct Outputs
			{
				Required<SoundComponent>		sound;	// We require a sound component even if we don't want to hold onto the sound to save update IO
				Optional<SoundActorComponent>	actor;
			};

			DECLARE_SYSTEM(usg::SYSTEM_POST_TRANSFORM)

			static void OnEvent(const Inputs& inputs, Outputs& outputs, const PlaySound &evt)
			{
				SoundHandle hndl;
				if (evt.bPositional)
				{
					if (outputs.actor.Exists())
					{
						hndl = Audio::Inst()->Prepare3DSound(outputs.actor.Force().GetRuntimeData().hndl, evt.uAudioID, 1.0f, true);
					}
					else
					{
						hndl = Audio::Inst()->Play3DSound(inputs.mtx->matrix.vPos().v3(), evt.uAudioID, 1.0f);
					}
				}
				else
				{
					hndl = Audio::Inst()->Prepare2DSound(evt.uAudioID, 1.0f);
				}

				outputs.sound.GetRuntimeData().hndl = hndl;
			}

			static void OnEvent(const Inputs& inputs, Outputs& outputs, const StopSound &evt)
			{
				// No meaning to stop a sound we don't have a handle to
				outputs.sound.GetRuntimeData().hndl.Stop();
			}

		};

		class UpdateAudioListener : public System
		{
		public:

			struct Inputs
			{
				Required<MatrixComponent, FromSelfOrParents> mtx;
				Optional<RigidBody, FromSelfOrParents>		rigidBody;

			};

			struct Outputs
			{
				Required<AudioListenerComponent>		listener;
			};

			DECLARE_SYSTEM(usg::SYSTEM_POST_TRANSFORM)

			static void LateUpdate(const Inputs& inputs, Outputs& outputs, float fDelta)
			{
				usg::Vector3f vPrevPos = outputs.listener.GetRuntimeData().pListener->GetMatrix().vPos().v3();
				outputs.listener.GetRuntimeData().pListener->SetMatrix(inputs.mtx->matrix);
				if (inputs.rigidBody.Exists())
				{
					outputs.listener.GetRuntimeData().pListener->SetVelocity(physics::GetLinearVelocity(inputs.rigidBody));
				}
				else
				{
					if (fDelta > FLT_EPSILON)
					{
						usg::Vector3f vDir = inputs.mtx->matrix.vPos().v3() - vPrevPos;
						vDir /= fDelta;

						outputs.listener.GetRuntimeData().pListener->SetVelocity(vDir);
					}
				}
			}

		};



	}

}


#include GENERATED_SYSTEM_CODE(Engine/Audio/AudioSystems.cpp)
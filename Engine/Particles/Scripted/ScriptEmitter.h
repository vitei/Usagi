/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_PARTICLES_SCRIPT_EMITTER_H_
#define _USG_PARTICLES_SCRIPT_EMITTER_H_
#include "Engine/Common/Common.h"
#include "Engine/Particles/SpriteEmitter.h"
#include "Engine/Particles/Scripted/EmitterShapes.h"
#include "Engine/Resource/ParticleEmitterResource.h"
#include "Engine/Resource/ResourceDecl.h"

namespace usg{

	class ParticleMgr;

	class ScriptEmitter : public usg::SpriteEmitter
	{
		typedef usg::SpriteEmitter Inherited;
	public:
		ScriptEmitter();
		virtual ~ScriptEmitter();

		void Alloc(usg::GFXDevice* pDevice, ParticleMgr* pMgr, const char* szFileName, bool bDynamicResize = false);	// Dynamic resize should be used in the editor only
		// This should really be private, but exposed for the benefit of the shape emitter
		void CreateEmitterShape(particles::EmitterShape eShape, const particles::EmitterShapeDetails shapeDetails);
		virtual void Init(usg::GFXDevice* pDevice, const usg::ParticleEffect* pParent);
		virtual void CleanUp(GFXDevice* pDevice) override;
		virtual bool Update(float fElapsed);
		virtual void UpdateBuffers(GFXDevice* pDevice);
		virtual void CalculateMaxBoundingArea(usg::Sphere& sphereOut);
		virtual bool RequiresCPUUpdate() const { return m_bRequiredCPUUpdate; }
		virtual void SetScale(float fScale);
		virtual void RenderPassChanged(GFXDevice* pDevice, uint32 uContextId, const RenderPassHndl &renderPass) override;

		// Script specific functions
		void SetInstanceData(const Matrix4x4& mLocalMatrix, float fParticleScale, float fTriggerTime);

		const particles::EmitterEmission& GetEmission() const { return m_emissionDef; }

		virtual void FreeFromPool();

		const U8String& GetScriptName() { return m_scriptName; }

		const usg::particles::EmitterShapeDetails& GetShapeDetails() const;

		void InitMaterial(GFXDevice* pDevice, const RenderPassHndl& renderPass, ParticleEmitterResHndl pResource = NULL, bool bFirst = false);
#ifdef PARTICLE_EDITOR
		Material& GetMaterial() { return m_material; }
		particles::EmitterEmission& GetDefinition() { return m_emissionDef; }
		void SetDefinition(GFXDevice* pDevice, const particles::EmitterEmission& def)
		{
			m_emissionDef = def;
			PipelineStateDecl decl;
			pDevice->GetPipelineDeclaration(m_material.GetPipelineStateHndl(), decl);
			decl.pEffect = ParticleEmitterResource::GetEffect(pDevice, m_emissionDef.eParticleType);
			m_material.SetPipelineState(pDevice->GetPipelineState(decl));
			FillOutConstantBuffer(pDevice, true);
		}
#endif

		void FillOutConstantBuffer(GFXDevice* pDevice, bool bEditor = false);
		bool IsLocalSpace() const { return m_bLocalOffset; }

	protected:

		inline float GetColorIndex() const;
		inline float GetEmissionLife() const;
		void InitEmissionPosAndVelocity(Vector3f& vPosOut, Vector3f& vVelocityOut) const;
		float GetEffectTime() const { return m_fEffectTime; }

		const particles::EmitterEmission& GetEmissionDef() const { return m_emissionDef; }
		const EmitterShape* GetEmitterShape() const	{ return m_pEmitterShape; }
		const Matrix4x4& GetWorldMatrix() const { return m_mWorldMatrix; }

		void SetLocalOffset(bool bUse) { m_bLocalOffset = bUse; }
	private:
		PRIVATIZE_COPY(ScriptEmitter)

		virtual float InitParticleData(void* pData, void* pMetaData, float fLerp);
		virtual void UpdateParticleCPUDataInt(void* pGPUData, void* pMetaData, float fElapsed);
		virtual void InitMetaDataForParticle(void* pParticle, void* pMetaData);
		virtual void SetMaterial(usg::GFXContext* pContext);

		class FloatAnimation
		{
		public:
			FloatAnimation();
			~FloatAnimation();

			void Init(const particles::FloatAnim* pAnim, float fBaseTime = 0.0f);
			void SetMultiplier(float fMul) { m_fMultiplier = fMul;  }
			void Update(float fEffectTime);
			float GetValue() const { return m_fValue * m_fMultiplier; }

		private:
			const particles::FloatAnim*	m_pAnimBase;	
			float						m_fValue;
			float						m_fMultiplier;
			uint32						m_uActiveIdx;
		};

		void ResetMatrix(const particles::EmitterShapeDetails shapeDetails, float fScaleMul = 1.0f);

		usg::ConstantSet			m_customConstants;
		usg::ConstantSet			m_gsTransform;
		usg::ConstantSet			m_materialConsts;
		usg::ConstantSet			m_fragConsts;

		// TODO: Move over to a resource shared between all particles using the same defintion
		usg::Material 				m_material;
		Vector2f					m_vUVScale[usg::particles::EmitterEmission::textureData_max_count];

		particles::EmitterEmission	m_emissionDef;
		EmitterShape*				m_pEmitterShape;
		ParticleMgr*				m_pOwner;

		float						m_fEffectTime;
		float						m_fAccumulator;
		float						m_fInterval;
		float						m_fTriggerTime;
		float						m_fDelay;
		float						m_fParticleAspect;
		float						m_fScale;
		Vector3f					m_vShapeVelocity;
		FloatAnimation				m_baseScale;
		FloatAnimation				m_baseLife;
		FloatAnimation				m_initialSpeed;
		FloatAnimation				m_dirVelocity;
		FloatAnimation				m_emission;
		U8String					m_scriptName;
		bool						m_bRequiredCPUUpdate;
		Matrix4x4					m_definitionMatrix;
		Matrix4x4					m_effectLocalMatrix;
		Vector4f					m_vPrevPos;
		Vector3f					m_vVelocityOffset;
		Matrix4x4					m_mWorldMatrix;

		bool						m_bLocalOffset;
		bool					    m_bDynamicResize;
	};


	inline float ScriptEmitter::GetColorIndex() const
	{
		switch(m_emissionDef.particleColor.eColorMode)
		{
		case particles::PARTICLE_COLOR_CONSTANT:
		case particles::PARTICLE_COLOR_ANIMATION:
			return 0.0f;
		case particles::PARTICLE_COLOR_RANDOM:
			return (float)(Math::Rand()%3);
		default:
			ASSERT(false);
			return 0.0f;
		}
	}


	inline float ScriptEmitter::GetEmissionLife() const
	{
		float fRandRng = (m_emissionDef.fLifeRandomness * m_baseLife.GetValue() * 0.5f);
		return m_baseLife.GetValue() + Math::RangedRandom(-fRandRng, fRandRng);
	}
}

#endif // _USG_PARTICLES_SCRIPT_EMITTER_H_

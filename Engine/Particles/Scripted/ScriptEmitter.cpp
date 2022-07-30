/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Maths/MathUtil.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Particles/Scripted/ScriptedParticle.h"
#include "Engine/Maths/MathUtil.h"
#include "Engine/Particles/ParticleMgr.h"
#include "Engine/Scene/SceneConstantSets.h"
#include "Engine/Graphics/Textures/Texture.h"
#include "ScriptedParticle.h"
#include "ScriptEmitter.h"

namespace usg
{

	static RenderLayer layerMapping[] =
	{
		// FIXME: Update the shader to handle opaque
		RenderLayer::LAYER_TRANSLUCENT, //RenderLayer::LAYER_OPAQUE_UNLIT,
		RenderLayer::LAYER_TRANSLUCENT,
		RenderLayer::LAYER_SUBTRACTIVE,
		RenderLayer::LAYER_ADDITIVE
	};

	ScriptEmitter::FloatAnimation::FloatAnimation()
	{
		m_fValue = 0.0f;
		m_fMultiplier = 1.0f;
		m_uActiveIdx = 0;
		m_pAnimBase = NULL;
	}
	
	ScriptEmitter::FloatAnimation::~FloatAnimation()
	{

	}

	void ScriptEmitter::FloatAnimation::Init(const particles::FloatAnim* pAnim, float fBaseTime)
	{
		m_fValue = 0.0f;
		m_uActiveIdx = 0;
		m_pAnimBase = pAnim;
		ASSERT(m_pAnimBase->frames_count > 0);
		m_fValue = m_pAnimBase->frames[0].fValue;

		while((sint32)m_uActiveIdx < (m_pAnimBase->frames_count-1) && fBaseTime > m_pAnimBase->frames[m_uActiveIdx].fTimeIndex)
		{
			m_fValue = m_pAnimBase->frames[m_uActiveIdx].fValue;
			m_uActiveIdx++;
		}
	}

	void ScriptEmitter::FloatAnimation::Update(float fEffectTime)
	{
		if((sint32)m_uActiveIdx < (m_pAnimBase->frames_count-1))
		{
			if( fEffectTime >= m_pAnimBase->frames[m_uActiveIdx+1].fTimeIndex )
			{
				m_uActiveIdx++;
			}
		}
			
		if((int)m_uActiveIdx >= (m_pAnimBase->frames_count-1))
		{
			// Take the last value
			m_fValue = m_pAnimBase->frames[m_uActiveIdx].fValue;
		}
		else
		{
			float fTime = fEffectTime - m_pAnimBase->frames[m_uActiveIdx].fTimeIndex;
			float fRange = m_pAnimBase->frames[m_uActiveIdx+1].fTimeIndex - m_pAnimBase->frames[m_uActiveIdx].fTimeIndex;
			m_fValue = Math::Lerp(m_pAnimBase->frames[m_uActiveIdx].fValue, m_pAnimBase->frames[m_uActiveIdx+1].fValue, fTime/fRange);
		}

		//ASSERT(m_fValue >= 0.0f);
		
	}

	ScriptEmitter::ScriptEmitter()
	{
		m_pEmitterShape = NULL;
		m_fTriggerTime = 0.0f;
		m_effectLocalMatrix = Matrix4x4::Identity();
		m_bLocalOffset = false; 
		m_bDynamicResize = false;
	}


	ScriptEmitter::~ScriptEmitter()
	{
		if(m_pEmitterShape!=NULL)
		{
			// If we already have one then delete it
			vdelete m_pEmitterShape;
			m_pEmitterShape = NULL;
		}
	}

	void ScriptEmitter::Alloc(usg::GFXDevice* pDevice, ParticleMgr* pMgr, const char* szScriptName, bool bDynamicResize)
	{
		ResourceMgr* pRes = ResourceMgr::Inst();

		ParticleEmitterResHndl pResource = pRes->GetParticleEmitter(pDevice, szScriptName);

		m_pOwner = pMgr;
		m_scriptName = szScriptName;
		m_bDynamicResize = bDynamicResize;


		if (pResource)
		{
			m_emissionDef = pResource->GetEmissionDetails();
		}
		else
		{
			EmitterEmission_init(&m_emissionDef);
		}

		if(m_emissionDef.eParticleType >= particles::PARTICLE_TYPE_COUNT)
			m_emissionDef.eParticleType = particles::PARTICLE_TYPE_BILLBOARD;

		m_customConstants.Init(pDevice, Particle::g_scriptedParticlePerFrame);
		m_gsTransform.Init(pDevice, Particle::g_scriptedParticleGSTrans);

		particles::EmitterShapeDetails		shapedef;
		if (pResource)
		{
			memcpy(&shapedef, &pResource->GetShapeDetails(), sizeof(shapedef));
		}
		else
		{
			particles::EmitterShapeDetails_init(&shapedef);
		}


		CreateEmitterShape(m_emissionDef.eShape, shapedef);
		
		InitMaterial(pDevice, pResource, true);
		
		m_materialConsts.Init(pDevice, Particle::g_scriptedParticlePerEffectDecl);
		m_material.SetConstantSet(SHADER_CONSTANT_MATERIAL, &m_materialConsts, SHADER_FLAG_VS_GS);
		m_material.SetConstantSet(SHADER_CONSTANT_CUSTOM_3, &m_customConstants);
		m_material.SetConstantSet(SHADER_CONSTANT_CUSTOM_1, &m_gsTransform);
		m_fragConsts.Init(pDevice, Particle::g_scriptedFragmentDecl);
		m_material.SetConstantSet(SHADER_CONSTANT_MATERIAL_1, &m_fragConsts, SHADER_FLAG_PIXEL);

		FillOutConstantBuffer(pDevice);
		
		if(bDynamicResize)
		{
			Inherited::Alloc(pDevice, 2000, sizeof(Particle::ScriptedParticle), sizeof(Particle::ScriptedMetaData) );
		}
		else
		{
			Inherited::Alloc(pDevice, m_emissionDef.emission.uMaxParticles, sizeof(Particle::ScriptedParticle), m_bRequiredCPUUpdate ? sizeof(Particle::ScriptedMetaData): 0);
		}	

		m_mWorldMatrix.LoadIdentity();
	}

	void ScriptEmitter::InitMaterial(GFXDevice* pDevice, ParticleEmitterResHndl pResource, bool bFirst)
	{
		SetLayer(layerMapping[m_emissionDef.sortSettings.eRenderLayer]);
		SetPriority(m_emissionDef.sortSettings.uPriority);

#ifdef DEBUG_BUILD
		if(pResource)
		{
			m_name = pResource->GetName();
		}
#endif

		RenderPassHndl renderPass = m_pOwner->GetScene()->GetRenderPasses(0).GetRenderPass(*this);
		if (pResource)
		{
			m_material.Init(pDevice, pResource->GetPipeline(pDevice, renderPass), pResource->GetDescriptorLayout());
		}
		else
		{
			// TODO: This code should only be available to the particle editor
			PipelineStateDecl pipelineDecl;
			pipelineDecl.inputBindings[0].Init(Particle::g_scriptedParticleVertexElements);
			pipelineDecl.uInputBindingCount = 1;
			pipelineDecl.ePrimType = PT_POINTS;

			DescriptorSetLayoutHndl matDescriptors;
			matDescriptors = pDevice->GetDescriptorSetLayout(Particle::g_scriptedDescriptorDecl);

			pipelineDecl.layout.descriptorSets[0] = pDevice->GetDescriptorSetLayout(SceneConsts::g_globalDescriptorDecl);
			pipelineDecl.layout.descriptorSets[1] = matDescriptors;
			pipelineDecl.layout.uDescriptorSetCount = 2;


			AlphaStateDecl& alphaDecl = pipelineDecl.alphaState;
			alphaDecl.InitFromDefinition(m_emissionDef.blend);
			alphaDecl.SetColor0Only();

			DepthStencilStateDecl& depthDecl = pipelineDecl.depthState;
			depthDecl.bDepthWrite = m_emissionDef.sortSettings.bWriteDepth;
			depthDecl.bDepthEnable = true;
			depthDecl.eDepthFunc = DEPTH_TEST_LESS;
			depthDecl.bStencilEnable = false;
			depthDecl.eStencilTest = STENCIL_TEST_ALWAYS;

			RasterizerStateDecl& rasState = pipelineDecl.rasterizerState;
			rasState.eCullFace = CULL_FACE_NONE;
			if (m_emissionDef.sortSettings.has_fDepthOffset)
			{
				rasState.bUseDepthBias = m_emissionDef.sortSettings.fDepthOffset != 0.0f;
				rasState.fDepthBias = m_emissionDef.sortSettings.fDepthOffset;
			}

			pipelineDecl.pEffect = ParticleEmitterResource::GetEffect(pDevice, m_emissionDef.eParticleType);
			if (bFirst)
			{
				m_material.Init(pDevice, pDevice->GetPipelineState(renderPass, pipelineDecl), pipelineDecl.layout.descriptorSets[1]);
			}
			else
			{
				m_material.SetPipelineState(pDevice->GetPipelineState(renderPass, pipelineDecl));
				m_material.SetDescriptorLayout(pDevice, pipelineDecl.layout.descriptorSets[1]);
				m_material.SetConstantSet(SHADER_CONSTANT_MATERIAL, &m_materialConsts, SHADER_FLAG_VS_GS);
				m_material.SetConstantSet(SHADER_CONSTANT_CUSTOM_3, &m_customConstants);
				m_material.SetConstantSet(SHADER_CONSTANT_CUSTOM_1, &m_gsTransform);
				m_material.SetConstantSet(SHADER_CONSTANT_MATERIAL_1, &m_fragConsts, SHADER_FLAG_PIXEL);
			}
		}

		TextureHndl pTextures[particles::EmitterEmission::textureData_max_count];
		for (uint32 i = 0; i < m_emissionDef.textureData_count; i++)
		{
			SamplerDecl decl;
			decl.SetClamp(SAMP_WRAP_CLAMP);
			usg::string texPath = "particle/Textures/";
			texPath += m_emissionDef.textureData[i].name;
			pTextures[i] = ResourceMgr::Inst()->GetTextureAbsolutePath(pDevice, texPath.c_str());
			m_material.SetTexture(i, pTextures[i], pDevice->GetSampler(decl));
		}
		if (pTextures[0])
		{
			float fWidth = (float)pTextures[0]->GetWidth() / (float)m_emissionDef.textureData[0].uPatternRepeatHor;
			float fHeight = (float)pTextures[0]->GetHeight() / (float)m_emissionDef.textureData[0].uPatternRepeatVer;
			m_fParticleAspect = fWidth / fHeight;
		}
		else
		{
			m_fParticleAspect = 1.0f;
		}

	}

	const usg::particles::EmitterShapeDetails& ScriptEmitter::GetShapeDetails() const
	{
		ASSERT(m_pEmitterShape!=NULL);
		return m_pEmitterShape->GetDetails();
	}

	void ScriptEmitter::CreateEmitterShape(particles::EmitterShape eShape, const particles::EmitterShapeDetails shapeDetails)
	{
		if(m_pEmitterShape!=NULL)
		{
			// If we already have one then delete it
			vdelete m_pEmitterShape;
			m_pEmitterShape = NULL;
		}

		m_pEmitterShape = EmitterShape::CreateShape(eShape, shapeDetails);

		ResetMatrix(shapeDetails);
	}

	void ScriptEmitter::ResetMatrix(const particles::EmitterShapeDetails shapeDetails, float fScaleMul)
	{
		Matrix4x4 mScale;
		mScale.MakeScale(shapeDetails.baseShape.vScale*fScaleMul);
		m_definitionMatrix.LoadIdentity();
		m_definitionMatrix.MakeRotate(Math::DegToRad(shapeDetails.baseShape.vRotation.x), -Math::DegToRad(shapeDetails.baseShape.vRotation.y), Math::DegToRad(shapeDetails.baseShape.vRotation.z));
		m_definitionMatrix.SetTranslation(shapeDetails.baseShape.vPosition);
		m_definitionMatrix = m_definitionMatrix * mScale;
	}

	void ScriptEmitter::FillOutConstantBuffer(GFXDevice* pDevice, bool bEditor)
	{
		//m_material.SetEffect(m_pEffects[m_emissionDef.eParticleType]);

		for(uint32 i=0; i<m_emissionDef.textureData_count; i++)
		{ 
			m_vUVScale[i].x = 1.0f / (float)m_emissionDef.textureData[i].uPatternRepeatHor;
			m_vUVScale[i].y = 1.0f / (float)m_emissionDef.textureData[i].uPatternRepeatVer;
		}

		// For the benefit of the particle editor
		if(m_material.GetTexture(0))
		{
			float fWidth = (float)m_material.GetTexture(0)->GetWidth()/(float)m_emissionDef.textureData[0].uPatternRepeatHor;
			float fHeight = (float)m_material.GetTexture(0)->GetHeight()/(float)m_emissionDef.textureData[0].uPatternRepeatVer;
			m_fParticleAspect = fWidth/fHeight;
		}

		SetMaxCount(m_emissionDef.emission.uMaxParticles);
		SetLocalOffset( m_emissionDef.has_bLocalEffect ? m_emissionDef.bLocalEffect : false );

		m_initialSpeed.Init(&m_emissionDef.omniVelocity, m_fEffectTime);
		m_dirVelocity.Init(&m_emissionDef.dirVelocity, m_fEffectTime);
		m_baseLife.Init(&m_emissionDef.life, m_fEffectTime);
		m_baseScale.Init(&m_emissionDef.particleScale.standardValue, m_fEffectTime);
		m_emission.Init(&m_emissionDef.emission.emissionRate, m_fEffectTime);

		const particles::EmitterEmission& res = m_emissionDef;
		Particle::ScriptedParticleConstants* pMaterialSettings = m_materialConsts.Lock<Particle::ScriptedParticleConstants>();
		pMaterialSettings->vColorSet[0] = res.particleColor.cColor0;
		pMaterialSettings->vColorSet[1] = res.particleColor.cColor1;
		pMaterialSettings->vColorSet[2] = res.particleColor.cColor2;
		pMaterialSettings->bLocalEffect = IsLocalSpace();
		pMaterialSettings->vAlphaValues.Assign(	res.particleAlpha.fInitialAlpha,
			res.particleAlpha.fIntermediateAlpha,
			res.particleAlpha.fEndAlpha, 0.0f);
		pMaterialSettings->vAlphaTiming.Assign( res.particleAlpha.fFinishInTime, res.particleAlpha.fOutStartTiming );

		for (uint32 i = 0; i < m_emissionDef.textureData_count; i++)
		{
			pMaterialSettings->vUVScale[i] = m_vUVScale[i];
		}
		pMaterialSettings->vScaleTiming.Assign( res.particleScale.fBeginScaleIn, res.particleScale.fStartScaleOut);
		pMaterialSettings->vScaling.Assign(res.particleScale.fInitial, res.particleScale.fIntermediate, res.particleScale.fEnding, 0.0f);
		//pMaterialSettings->fTexFrames[0] = (float)res.textureData->uPatternRepeatHor;
		//pMaterialSettings->fTexFrames[1] = (float)res.textureData->uPatternRepeatVer;
		
		pMaterialSettings->fColorBehaviour = (float)res.particleColor.eColorMode;
		pMaterialSettings->vColorTiming.Assign(res.particleColor.fInTimeEnd, res.particleColor.fPeak, res.particleColor.fOutTimeStart, (float)res.particleColor.uRepetitionCount);

		pMaterialSettings->fAirResistance = 1.0f - m_emissionDef.fDrag;
		pMaterialSettings->fUpdatePosition = m_emissionDef.bCPUPositionUpdate ? 0.0f : 1.0f;
		pMaterialSettings->vGravityDir.Assign(res.vGravityDir * res.fGravityStrength, 0.0f);

		// Apply the environment color
		if (m_pOwner)
		{
			pMaterialSettings->vEnvColor = Math::Lerp(Color(1.0f, 1.0f, 1.0f, 1.0f), m_pOwner->GetEnvironmentColor(), m_emissionDef.particleColor.fLerpEnvColor);
		}
		else
		{
			pMaterialSettings->vEnvColor.Assign(1.0f, 1.0f, 1.0f, 1.0f);
		}

		pMaterialSettings->fColorAnimRepeat = (float)m_emissionDef.particleColor.uRepetitionCount;
		m_materialConsts.Unlock(); 

		Particle::ScriptedParticleFragment* pFrag = m_fragConsts.Lock<Particle::ScriptedParticleFragment>();
		pFrag->fAlphaRef  = res.blend.alphaTestFunc == usg::ALPHA_TEST_ALWAYS ? -1.0f : res.blend.alphaTestReference;
		pFrag->fDepthFade = res.fSoftFadeDistance;
		m_fragConsts.Unlock();
		m_fragConsts.UpdateData(pDevice);

		Particle::ScriptedParticleGSTrans* pFrame = m_gsTransform.Lock<Particle::ScriptedParticleGSTrans>();
		pFrame->vParticleCenter = m_emissionDef.vParticleCenter;
		pFrame->fDepthFadeDist = m_emissionDef.fSoftFadeDistance;
		pFrame->fCameraOffset = m_emissionDef.has_fCameraOffset ? m_emissionDef.fCameraOffset : 0.0f;
		pFrame->bCustomMatrix = m_emissionDef.eParticleType == particles::PARTICLE_TYPE_USER_ORIENTED;
		pFrame->bYAxisAlign = m_emissionDef.eParticleType == particles::PARTICLE_TYPE_Y_AXIS_ALIGNED;
		pFrame->mOrientation = Matrix4x4::Identity();
		m_gsTransform.Unlock();

		m_materialConsts.UpdateData(pDevice);
		m_gsTransform.UpdateData(pDevice);

		m_bRequiredCPUUpdate = false;
		for(uint32 i=0; i<m_emissionDef.textureData_count; i++)
		{
			m_bRequiredCPUUpdate |= m_emissionDef.textureData[0].textureAnim.eTexMode != usg::particles::TEX_MODE_NONE;
		}
		m_bRequiredCPUUpdate |= m_emissionDef.bCPUPositionUpdate;


	}

	void ScriptEmitter::Init(GFXDevice* pDevice, const usg::ParticleEffect* pParent)
	{
		Particle::ScriptedParticlePerFrame* pFrame = m_customConstants.Lock<Particle::ScriptedParticlePerFrame>();
		pFrame->fEffectTime = 0.0f;
		m_customConstants.Unlock();

		Inherited::Init(pDevice, pParent);
		m_fInterval = 0.0f;
		m_fEffectTime = 0.0f;
		m_fAccumulator = 0.0f;
		m_fDelay = m_fTriggerTime;



		if (m_emissionDef.emission.has_vUserRotationRandom)
		{
			m_vRandomRot = usg::Vector3f::RandomRange(-m_emissionDef.emission.vUserRotationRandom, m_emissionDef.emission.vUserRotationRandom);
		}

		m_initialSpeed.Init(&m_emissionDef.omniVelocity);
		m_dirVelocity.Init(&m_emissionDef.dirVelocity);
		m_baseLife.Init(&m_emissionDef.life);
		m_baseScale.Init(&m_emissionDef.particleScale.standardValue);
		m_emission.Init(&m_emissionDef.emission.emissionRate);
		const usg::particles::EmitterShapeDetails& shapeDef = m_pEmitterShape->GetDetails();
		m_baseScale.Init(&m_emissionDef.particleScale.standardValue, 0.0f);
		m_vVelocityOffset = Vector3f::ZERO;
		m_baseScale.SetMultiplier(m_fScale);

		// Always on to silence warnings
		//if (m_emissionDef.has_bLocalEffect && m_emissionDef.bLocalEffect)
		{
			m_material.SetConstantSet(SHADER_CONSTANT_CUSTOM_0, GetParent()->GetConstantSet());
		}

		m_material.UpdateDescriptors(pDevice);

		ResetMatrix(shapeDef, m_fScale);
	}

	void ScriptEmitter::Cleanup(GFXDevice* pDevice)
	{
		m_customConstants.Cleanup(pDevice);
		m_gsTransform.Cleanup(pDevice);
		m_materialConsts.Cleanup(pDevice);
		m_fragConsts.Cleanup(pDevice);
		m_material.Cleanup(pDevice);
		Inherited::Cleanup(pDevice);
	}

	void ScriptEmitter::SetScale(float fScale)
	{
		m_fScale = fScale;
	}

	void ScriptEmitter::RenderPassChanged(GFXDevice* pDevice, uint32 uContextId, const RenderPassHndl &renderPass, const SceneRenderPasses& scenePasses)
	{
		m_material.UpdateRenderPass(pDevice, renderPass);
	}


	bool ScriptEmitter::Update(float fElapsed)
	{
		m_fDelay -= fElapsed;
		if(m_fDelay > 0.0f)
			return true;

		m_vPrevPos = m_mWorldMatrix.vPos();
		{
			if(IsLocalSpace())
			{
				m_mWorldMatrix = m_definitionMatrix * m_effectLocalMatrix;
			}
			else
			{
				m_mWorldMatrix = m_definitionMatrix * m_effectLocalMatrix * GetParent()->GetMatrix();
			}
		}

		// First frame
		if(m_fEffectTime == 0.0f)
		{
			const usg::particles::EmitterShapeDetails& shapeDef = m_pEmitterShape->GetDetails();
			m_vPrevPos = m_mWorldMatrix.vPos();
			// Initialise the velocity in effect space
			float velocityRandRange = shapeDef.baseShape.fSpeedRand * 0.5f;
			m_vShapeVelocity = shapeDef.baseShape.vVelocity + (Math::RangedRandom(-velocityRandRange, velocityRandRange)*shapeDef.baseShape.vVelocity);
			//usg::Matrix4x4 mTransform = m_effectLocalMatrix * GetParent()->GetMatrix();
			m_mWorldMatrix.TransformVec3(m_vShapeVelocity, 0.0f);
		}

		m_vShapeVelocity += m_pEmitterShape->GetDetails().baseShape.vGravity * fElapsed;
		m_vVelocityOffset += m_vShapeVelocity * fElapsed;

		m_mWorldMatrix.Translate(m_vVelocityOffset);

		m_definitionMatrix.Translate( m_pEmitterShape->GetDetails().baseShape.vVelocity * fElapsed );

		m_fInterval -= fElapsed;
		m_baseLife.Update(m_fEffectTime);
		m_initialSpeed.Update(m_fEffectTime);
		m_emission.Update(m_fEffectTime);
		m_dirVelocity.Update(m_fEffectTime);
		m_baseScale.Update(m_fEffectTime);
		
		m_fEffectTime += fElapsed;

		uint32 uRequestedEmission = 0;
		if(m_emissionDef.emission.eEmissionType == usg::particles::EMISSION_TYPE_ONE_SHOT)
		{
			if(m_fEffectTime == fElapsed)
			{	
				uRequestedEmission = m_emissionDef.emission.uMaxParticles;
			}
			
		}
		else if(m_fEffectTime < m_emissionDef.emission.fEmissionTime || m_emissionDef.emission.eEmissionType == usg::particles::EMISSION_TYPE_INFINITE)
		{
			if(m_fInterval <= 0.0f)
			{
				if (m_fInterval < -m_emissionDef.emission.fReleaseInterval)
				{
					m_fInterval = m_emissionDef.emission.fReleaseInterval;
					if (m_emissionDef.emission.fReleaseIntervalRandom > 0.0f)
					{
						m_fInterval += Math::RangedRandom(-m_emissionDef.emission.fReleaseIntervalRandom, m_emissionDef.emission.fReleaseIntervalRandom);
					}
				}

				m_fAccumulator += m_emission.GetValue() * fElapsed;
				m_fAccumulator = usg::Math::Clamp(m_fAccumulator, 0.0f, (float)m_emissionDef.emission.uMaxParticles);
				uRequestedEmission = (uint32)m_fAccumulator;
				m_fAccumulator -= floorf(m_fAccumulator);
			}
		}

		if(uRequestedEmission > 0)
		{
			if (!m_emissionDef.emission.has_fReleaseRandom || m_emissionDef.emission.fReleaseRandom >= 1.0f)
			{
				EmitParticle(uRequestedEmission);
			}
			else
			{
				for (uint32 i = 0; i < uRequestedEmission; i++)
				{
					if (Math::RangedRandom(0.0f, 1.0f) < m_emissionDef.emission.fReleaseRandom)
					{
						EmitParticle(1);
					}
				}
			}
		}
			

		Particle::ScriptedParticlePerFrame* pFrame = m_customConstants.Lock<Particle::ScriptedParticlePerFrame>();
		pFrame->fEffectTime = m_fEffectTime;
		m_customConstants.Unlock();

		if(m_emissionDef.eParticleType == usg::particles::PARTICLE_TYPE_USER_ORIENTED)
		{
			// FIXME: Add a custom artist orientation
			Particle::ScriptedParticleGSTrans* pGS = m_gsTransform.Lock<Particle::ScriptedParticleGSTrans>();
			if (pGS->bCustomMatrix)
			{
				usg::Matrix4x4 mRot;
				Vector3f vRot = m_emissionDef.emission.vUserRotation;
				vRot += m_vRandomRot;

				mRot.MakeRotateYPR(Math::DegreesToRadians(vRot.y), Math::DegreesToRadians(vRot.x), Math::DegreesToRadians(vRot.z));
				pGS->mOrientation = mRot;
			}
			else
			{
				pGS->mOrientation = usg::Matrix4x3::Identity();
			}
			m_gsTransform.Unlock();	
		}

		Inherited::Update(fElapsed);

		if(m_emissionDef.emission.eEmissionType == usg::particles::EMISSION_TYPE_ONE_SHOT)
			return false;

		if(m_emissionDef.emission.eEmissionType == usg::particles::EMISSION_TYPE_INFINITE)
			return true;

		return (m_fEffectTime < m_emissionDef.emission.fEmissionTime);
	}


	void ScriptEmitter::UpdateBuffers(GFXDevice* pDevice)
	{
		// Custom and transform constants are dynamic, don't need to update descriptors on their behalf
		m_customConstants.UpdateData(pDevice);
		m_gsTransform.UpdateData(pDevice);
		m_materialConsts.UpdateData(pDevice);
		m_fragConsts.UpdateData(pDevice);
		Inherited::UpdateBuffers(pDevice);
		m_material.UpdateDescriptors(pDevice);
	}

	void ScriptEmitter::UpdateParticleCPUDataInt(void* pGPUData, void* pMetaDataVoid, float fElapsed)
	{
		if(!m_bRequiredCPUUpdate)
			return;

		Particle::ScriptedParticle* pOut = (Particle::ScriptedParticle*)pGPUData;
		Particle::ScriptedMetaData* pMetaData = (Particle::ScriptedMetaData*)pMetaDataVoid;
		float fPartElapsed = pMetaData->fElapsed;
		float fNextElapsed = fPartElapsed + fElapsed;

		// Texture animation
		for(uint32 i=0; i<m_emissionDef.textureData_count; i++)
		{
			const usg::particles::TextureData& texData = m_emissionDef.textureData[0];
			const usg::particles::TextureAnimation& texAnim = texData.textureAnim;

			uint32 patternIdx = 0;
			switch(texAnim.eTexMode)
			{
			case usg::particles::TEX_MODE_FLIPBOOK_ONCE:
				patternIdx = (uint32)(((m_fEffectTime- pOut->fLifeStart) * pOut->fInvLife)*texAnim.animIndex_count);
				break;
			case usg::particles::TEX_MODE_RANDOM_IMAGE:
				patternIdx = Math::Rand();
				break;
			case usg::particles::TEX_MODE_FIT_TO_TIME:
				patternIdx = (uint32)(((m_fEffectTime - pOut->fLifeStart) * pOut->fInvLife) * (texData.uPatternRepeatHor * texData.uPatternRepeatVer) );
				break;
			case usg::particles::TEX_MODE_FLIPBOOK_LOOP:
				patternIdx = (uint32)(m_fEffectTime *texData.textureAnim.fAnimTimeScale*60.f);
				break;
			default:
				continue;
			}

			patternIdx += pMetaData->uRandom[i];
			if (texAnim.eTexMode != usg::particles::TEX_MODE_FIT_TO_TIME)
			{
				patternIdx = texAnim.animIndex[patternIdx % texAnim.animIndex_count];
			}

			uint32 uNoX = patternIdx % texData.uPatternRepeatHor;
			uint32 uNoY = (patternIdx / texData.uPatternRepeatHor);

			if (texAnim.eTexMode != usg::particles::TEX_MODE_RANDOM_IMAGE)
			{
				pOut->vUVOffset.x = m_vUVScale[i].x * uNoX;
				pOut->vUVOffset.y = m_vUVScale[i].y * uNoY;
			}
			else
			{
				if (fPartElapsed == 0.0f || (uint32)(fPartElapsed* texData.textureAnim.fAnimTimeScale * 60.f) != (uint32)(fNextElapsed* texData.textureAnim.fAnimTimeScale * 60.f))
				{
					pOut->vUVOffset.x = m_vUVScale[i].x * uNoX;
					pOut->vUVOffset.y = m_vUVScale[i].y * uNoY;
				}
			}
		}
		
		// CPU based position update (for better drag)
		if(m_emissionDef.bCPUPositionUpdate)
		{
			pOut->vPos += pOut->vVelocity * fElapsed;
			pOut->vVelocity += (m_emissionDef.vGravityDir*m_emissionDef.fGravityStrength) * fElapsed;
			pOut->vVelocity -= (pOut->vVelocity * fElapsed * (m_emissionDef.fDrag*5.f));
		}
		
		
		pMetaData->fElapsed+=fElapsed;
	}
	

	void ScriptEmitter::InitMetaDataForParticle(void* pParticle, void* pMetaData)
	{
		Particle::ScriptedMetaData* pData = (Particle::ScriptedMetaData*)pMetaData;
		for(uint32 i=0; i<m_emissionDef.textureData_count; i++)
		{
			if( m_emissionDef.textureData[i].textureAnim.bRandomOffset )
			{
				pData->uRandom[i] = Math::Rand()%m_emissionDef.textureData[i].textureAnim.animIndex_count;
			}
			else
			{
				pData->uRandom[i] = 0;
			}
		}
		pData->fElapsed = 0.0f;
	}

	float ScriptEmitter::InitParticleData(void* pData, void* pMetaData, float fLerp)
	{
		Matrix4x4 mParentMat = m_mWorldMatrix;
		mParentMat.SetPos(Lerp(m_vPrevPos, mParentMat.vPos(), fLerp));
		const particles::EmitterEmission& res = m_emissionDef;
		Particle::ScriptedParticle& out = *(Particle::ScriptedParticle*)pData;
	
		if(pMetaData)
		{
			InitMetaDataForParticle(pData, pMetaData);
		}

		float fLife = GetEmissionLife();

		out.fLifeStart = m_fEffectTime;
		out.fInvLife = 1.0f/fLife;

		InitEmissionPosAndVelocity(out.vPos, out.vVelocity, mParentMat);

		const particles::ParticleRotation& rotDef = m_emissionDef.particleRotation;

		// Set the speed
		out.fRotSpeed = rotDef.fSpeed;
		if (rotDef.fSpeedRandomise)
		{
			out.fRotSpeed += Math::RangedRandom(-rotDef.fSpeedRandomise*0.5f, rotDef.fSpeedRandomise*0.5f);
		}

		out.fRotSpeed = Math::DegreesToRadians(out.fRotSpeed);

		// Set the starting rotation
		out.fRotStart = rotDef.fBaseRotation;
		if(rotDef.fRandomise)
		{
			out.fRotStart += Math::RangedRandom(-rotDef.fRandomise*0.5f, rotDef.fRandomise*0.5f);
		}


		// Set the scale
		float fScaleValue = m_baseScale.GetValue();
		if(m_emissionDef.particleScale.fRandomness > 0.0f)
		{
			float fRange = fScaleValue * 0.5f * m_emissionDef.particleScale.fRandomness;
			fScaleValue += Math::RangedRandom(-fScaleValue*0.5f, fScaleValue*0.5f);
		}
		
		out.vSizeBase = Vector2f(m_fParticleAspect*fScaleValue, fScaleValue);
		out.vUVOffset = Vector2f(0.0f, 0.0f);

		// If the texture has multiple images pick the first one to use
		if(m_emissionDef.textureData[0].textureAnim.eTexMode == usg::particles::TEX_MODE_NONE && m_emissionDef.textureData[0].textureAnim.bRandomOffset)
		{
			uint32 uPatternIdx = Math::Rand()%(m_emissionDef.textureData[0].uPatternRepeatHor*m_emissionDef.textureData[0].uPatternRepeatVer);
			uint32 uNoX = uPatternIdx % m_emissionDef.textureData[0].uPatternRepeatHor;
			uint32 uNoY = (uPatternIdx / m_emissionDef.textureData[0].uPatternRepeatHor);

			out.vUVOffset.x = m_vUVScale[0].x * uNoX;
			out.vUVOffset.y = m_vUVScale[0].y * uNoY;
		}

		out.fColorIndex = GetColorIndex();


		return fLife;
	}
	
	void ScriptEmitter::SetMaterial(usg::GFXContext* pContext)
	{
		m_material.Apply(pContext);
	}
	
	void ScriptEmitter::CalculateMaxBoundingArea(Sphere& sphereOut)
	{

	}

	void ScriptEmitter::FreeFromPool()
	{
		if(m_pOwner && !m_bDynamicResize)
		{
			m_pOwner->FreeScriptedEffect(this);
		}
	}

	void ScriptEmitter::SetInstanceData(const Matrix4x4& mLocalMatrix, float fParticleScale, float fTriggerTime)
	{
		m_effectLocalMatrix = mLocalMatrix;
		m_fTriggerTime = fTriggerTime;
		m_fDelay = fTriggerTime;
		
		Particle::ScriptedParticleConstants* pMaterialSettings = m_materialConsts.Lock<Particle::ScriptedParticleConstants>();
		pMaterialSettings->vScaling.Assign(m_emissionDef.particleScale.fInitial*fParticleScale,
			m_emissionDef.particleScale.fIntermediate*fParticleScale,
			m_emissionDef.particleScale.fEnding*fParticleScale, 0.0f);
		m_materialConsts.Unlock();
	}


	void ScriptEmitter::InitEmissionPosAndVelocity(Vector3f& vPosOut, Vector3f& vVelocityOut, const usg::Matrix4x4& mCurrMat) const
	{
		const EmitterShape* pShape = GetEmitterShape();

		// Firstly get a position and velocity as defined by the emitters shape
		pShape->FillEmissionParams(vPosOut, vVelocityOut);

		// Apply a random offset
		if(m_emissionDef.fPositionRandomness != 0.0f)
		{
			Vector3f vPosRand;
			Vector3f vRandRange(m_emissionDef.fPositionRandomness*0.5f);
			vPosRand = Vector3f::RandomRange(-vRandRange, vRandRange);
			vPosOut += vPosRand;
		}


		// Apply a random vector in any direction to the velocity
		Vector3f vOmniVelocity;
		vOmniVelocity.SphericalRandomVector(m_initialSpeed.GetValue());
		vVelocityOut += vOmniVelocity;


		float fVelocityMul = Math::RangedRandom(1.0f - m_emissionDef.fSpeedRandomness, 1.0f + m_emissionDef.fSpeedRandomness);
		if(m_emissionDef.fVelocityDirConeDeg > 0.0f)
		{	
			float fRadAngle = Math::DegToRad(m_emissionDef.fVelocityDirConeDeg);
			float fHeight = Math::RangedRandom( cosf(fRadAngle), 1);
			float fRot = Math::RangedRandom(0, Math::two_pi);
			float32 fRadius = Math::SqrtSafe(1.0f - fHeight * fHeight);
			float fSinT, fCosT;
			Math::SinCos(fRot, fSinT, fCosT);
			Vector3f vVelocity(fRadius * fCosT, fHeight, fRadius * fSinT);

			// Rotation matrix to rotate in the specified direction
			Quaternionf q;
			Vector3f dir = m_emissionDef.vVelocityDir;
			dir.TryNormalise();
			q.MakeVectorRotation(Vector3f::Y_AXIS, dir );
			vVelocity = vVelocity * q;

			vVelocityOut = ( vVelocityOut + vVelocity * m_dirVelocity.GetValue() );
		}
		else
		{
			vVelocityOut += (m_emissionDef.vVelocityDir * m_dirVelocity.GetValue()) ;
		}
		vVelocityOut *= fVelocityMul;


		// Finally transform the position and the direction by the emitters matrix
		vPosOut = vPosOut * mCurrMat;
		vVelocityOut = mCurrMat.TransformVec3(vVelocityOut, 0.0f);

		if(m_emissionDef.bInheritVelocity)
		{
			vVelocityOut += GetParent()->GetSystemVelocity();
		}
	}

}



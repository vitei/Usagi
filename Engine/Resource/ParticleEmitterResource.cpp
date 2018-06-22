#include "Engine/Common/Common.h"
#include "Engine/Core/ProtocolBuffers/ProtocolBufferFile.h"
#include "Engine/Core/ProtocolBuffers/ProtocolBufferMemory.h"
#include "Engine/Particles/Scripted/ScriptedParticle.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/SceneConstantSets.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/Resource/ParticleEmitterResource.h"
#include "Engine/Graphics/Device/RenderState.h"
#include "Engine/Graphics/Device/GFXDevice.h"

namespace usg{

	ParticleEmitterResource::ParticleEmitterResource()
    {

    }
	ParticleEmitterResource::~ParticleEmitterResource()
    {

    }

	bool ParticleEmitterResource::Load(GFXDevice* pDevice, const vector<RenderPassHndl>& renderPasses, const char* szFileName)
	{
		SetupHash(szFileName);
		ProtocolBufferFile emitterVPB(szFileName);
		bool bReadSucceeded = emitterVPB.Read(&m_emissionDef);
		ASSERT(bReadSucceeded);
		bReadSucceeded &= emitterVPB.Read(&m_shapeDef);

		if (!bReadSucceeded)
		{
			return false;
		}

		Load(pDevice, renderPasses);

		return true;
	}

	bool ParticleEmitterResource::Load(GFXDevice* pDevice, const ParticleEmitterPak& pak, const void* pData, const char* szPackPath)
	{
		U8String fileName = szPackPath;
		fileName += pak.resHdr.strName;
		fileName += ".pem";

		SetupHash(fileName.CStr());
		ProtocolBufferMemory emitterVPB(pData, pak.resHdr.uDataSize);
		bool bReadSucceeded = emitterVPB.Read(&m_emissionDef);
		ASSERT(bReadSucceeded);
		bReadSucceeded &= emitterVPB.Read(&m_shapeDef);

		if (!bReadSucceeded)
		{
			return false;
		}

		Load(pDevice);

		return true;
	}

	void ParticleEmitterResource::Load(GFXDevice* pDevice)
	{
		m_pEffect = GetEffect(pDevice, m_emissionDef.eParticleType);

		m_stateDecl.inputBindings[0].Init(Particle::g_scriptedParticleVertexElements);
		m_stateDecl.uInputBindingCount = 1;
		m_stateDecl.ePrimType = PT_POINTS;

		if (m_emissionDef.has_bLocalEffect && m_emissionDef.bLocalEffect)
		{
			m_descriptorLayout = pDevice->GetDescriptorSetLayout(Particle::g_scriptedDescriptorDeclLocal);
		}
		else
		{
			m_descriptorLayout = pDevice->GetDescriptorSetLayout(Particle::g_scriptedDescriptorDecl);
		}
		
		m_stateDecl.layout.descriptorSets[0] = pDevice->GetDescriptorSetLayout(SceneConsts::g_globalDescriptorDecl);
		m_stateDecl.layout.descriptorSets[1] = m_descriptorLayout;
		m_stateDecl.layout.uDescriptorSetCount = 2;


		AlphaStateDecl& alphaDecl = m_stateDecl.alphaState;
		alphaDecl.InitFromDefinition(m_emissionDef.blend);

		DepthStencilStateDecl& depthDecl = m_stateDecl.depthState;
		depthDecl.bDepthWrite = m_emissionDef.sortSettings.bWriteDepth;
		depthDecl.bDepthEnable = true;
		depthDecl.eDepthFunc = DEPTH_TEST_LESS;
		depthDecl.bStencilEnable = false;
		depthDecl.eStencilTest = STENCIL_TEST_ALWAYS;

		RasterizerStateDecl& rasState = m_stateDecl.rasterizerState;
		rasState.eCullFace = CULL_FACE_NONE;
		if (m_emissionDef.sortSettings.has_fDepthOffset)
		{
			rasState.bUseDepthBias = m_emissionDef.sortSettings.fDepthOffset != 0.0f;
			rasState.fDepthBias = m_emissionDef.sortSettings.fDepthOffset;
		}

		m_stateDecl.pEffect = m_pEffect;


		SetReady(true);
	}


	PipelineStateHndl ParticleEmitterResource::GetPipeline(GFXDevice* pDevice, const RenderPassHndl& renderPass) const
	{
		return pDevice->GetPipelineState(renderPass, m_stateDecl);
	}

	EffectHndl ParticleEmitterResource::GetEffect(GFXDevice* pDevice, ::usg::particles::ParticleType eParticleType)
	{
		ResourceMgr* pRes = ResourceMgr::Inst();
		
		switch (eParticleType)
		{
		case particles::PARTICLE_TYPE_BILLBOARD:
		case particles::PARTICLE_TYPE_USER_ORIENTED:
		case particles::PARTICLE_TYPE_Y_AXIS_ALIGNED:
			return pRes->GetEffect(pDevice, "ScriptedParticle");
		case particles::PARTICLE_TYPE_DIR_POLYGON:
			return pRes->GetEffect(pDevice, "ScriptedDirParticle");
		case particles::PARTICLE_TYPE_TRAIL:
			return pRes->GetEffect(pDevice, "ScriptedTrailParticle");
		default:
			ASSERT(false);
			return NULL;
		}

	}
}


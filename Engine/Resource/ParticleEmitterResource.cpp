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

	bool ParticleEmitterResource::Load(GFXDevice* pDevice, const vector<RenderPassHndl>& renderPasses, const ParticleEmitterPak& pak, const void* pData, const char* szPackPath)
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

		Load(pDevice, renderPasses);

		return true;
	}

	void ParticleEmitterResource::Load(GFXDevice* pDevice, const vector<RenderPassHndl>& renderPasses)
	{
		m_pEffect = GetEffect(pDevice, m_emissionDef.eParticleType);

		PipelineStateDecl pipelineDecl;
		pipelineDecl.inputBindings[0].Init(Particle::g_scriptedParticleVertexElements);
		pipelineDecl.uInputBindingCount = 1;
		pipelineDecl.ePrimType = PT_POINTS;

		if (m_emissionDef.has_bLocalEffect && m_emissionDef.bLocalEffect)
		{
			m_descriptorLayout = pDevice->GetDescriptorSetLayout(Particle::g_scriptedDescriptorDeclLocal);
		}
		else
		{
			m_descriptorLayout = pDevice->GetDescriptorSetLayout(Particle::g_scriptedDescriptorDecl);
		}
		
		pipelineDecl.layout.descriptorSets[0] = pDevice->GetDescriptorSetLayout(SceneConsts::g_globalDescriptorDecl);
		pipelineDecl.layout.descriptorSets[1] = m_descriptorLayout;
		pipelineDecl.layout.uDescriptorSetCount = 2;


		AlphaStateDecl& alphaDecl = pipelineDecl.alphaState;
		alphaDecl.InitFromDefinition(m_emissionDef.blend);

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

		pipelineDecl.pEffect = m_pEffect;

		m_pipelines.resize(renderPasses.size());
		for (size_t i = 0; i < m_pipelines.size(); i++)
		{
			m_pipelines[i].renderPass = renderPasses[i];
			pipelineDecl.renderPass = renderPasses[i];
			m_pipelines[i].pipeline = pDevice->GetPipelineState(pipelineDecl);
		}
		SetReady(true);
	}

	PipelineStateHndl ParticleEmitterResource::GetPipeline(const RenderPassHndl& renderPass) const
	{
		for (size_t i = 0; i < m_pipelines.size(); i++)
		{
			if (m_pipelines[i].renderPass == renderPass)
			{
				return m_pipelines[i].pipeline;
			}
		}
		return PipelineStateHndl(); 
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


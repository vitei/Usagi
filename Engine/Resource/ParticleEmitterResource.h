/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_PARTICLE_EMITTER_RESOURCE_H_
#define _USG_PARTICLE_EMITTER_RESOURCE_H_
#include "Engine/Common/Common.h"
#include "Engine/Memory/MemHeap.h"
#include "Engine/Particles/Scripted/EffectGroup.pb.h"
#include "Engine/Particles/Scripted/ScriptEmitter.pb.h"
#include "Engine/Particles/Scripted/EmitterShapes.pb.h"
#include "Engine/Resource/ResourceBase.h"
#include "Engine/Graphics/Device/RenderState.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Core/stl/vector.h"

namespace usg{


class ParticleEmitterResource : public ResourceBase
{
public:
	ParticleEmitterResource();
    virtual ~ParticleEmitterResource();

	bool Load(GFXDevice* pDevice, const char* szFileName);

	const particles::EmitterEmission& GetEmissionDetails() const { return m_emissionDef;  }
	const usg::particles::EmitterShapeDetails& GetShapeDetails() const { return m_shapeDef; }
	PipelineStateHndl GetPipeline(GFXDevice* pDevice, const RenderPassHndl& renderPass) const;
	DescriptorSetLayoutHndl GetDescriptorLayout() const { return m_descriptorLayout;  }

	static EffectHndl GetEffect(GFXDevice* pDevice, ::usg::particles::ParticleType eParticleType);

private:
	void Load(GFXDevice* pDevice);

protected:

	particles::EmitterEmission			m_emissionDef;
	particles::EmitterShapeDetails		m_shapeDef;
	EffectHndl							m_pEffect;

	DescriptorSetLayoutHndl				m_descriptorLayout;

	PipelineStateDecl			m_stateDecl;
};

}

#endif // _USG_EMITTER_RESOURCE_H_

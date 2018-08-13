/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_PARTICLE_EFFECT_RESOURCE_H_
#define _USG_PARTICLE_EFFECT_RESOURCE_H_
#include "Engine/Common/Common.h"
#include "Engine/Memory/MemHeap.h"
#include "Engine/Particles/Scripted/EffectGroup.pb.h"
#include "Engine/Particles/Scripted/ScriptEmitter.pb.h"
#include "Engine/Particles/Scripted/EmitterShapes.pb.h"
#include "Engine/Resource/ResourceBase.h"
#include "Engine/Core/String/U8String.h"

namespace usg{

class GFXDevice;

class ParticleEffectResource : public ResourceBase
{
public:
    ParticleEffectResource();
    virtual ~ParticleEffectResource();

	bool Load(const char* szFileName);

	const particles::EffectGroup& GetEffectGroup() const { return m_definition; }
	uint32 GetEmitterCount() const { return m_definition.emitters_count; }
	uint32 GetRibbonCount() const { return m_definition.ribbons_count; }
	const U8String& GetName() const { return m_name; }
protected:

	U8String					m_name;
	particles::EffectGroup		m_definition;		
};

}

#endif // _USG_PARTICLE_EFFECT_RESOURCE_H_

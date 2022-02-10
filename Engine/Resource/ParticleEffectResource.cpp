#include "Engine/Common/Common.h"
#include "Engine/Resource/ParticleEffectResource.h"
#include "Engine/Core/ProtocolBuffers/ProtocolBufferFile.h"
#include "Engine/Core/ProtocolBuffers/ProtocolBufferMemory.h"
#include "Engine/Graphics/Device/GFXDevice.h"

namespace usg{

	ParticleEffectResource::ParticleEffectResource() :
		ResourceBase(StaticResType)
    {
    }
    ParticleEffectResource::~ParticleEffectResource()
    {

    }

	bool ParticleEffectResource::Load(const char* szFileName)
	{
		m_name = szFileName;
		str::RemovePath(m_name);
		str::TruncateExtension(m_name);
		SetupHash(szFileName);
		ProtocolBufferFile effectVPB(szFileName);
		bool bReadSucceeded = effectVPB.Read(&m_definition);	
		SetReady(true);
		return bReadSucceeded;
	}

}


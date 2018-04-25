#include "Engine/Common/Common.h"
#include "Engine/Resource/ParticleEffectResource.h"
#include "Engine/Core/ProtocolBuffers/ProtocolBufferFile.h"
#include "Engine/Core/ProtocolBuffers/ProtocolBufferMemory.h"
#include "Engine/Graphics/Device/GFXDevice.h"

namespace usg{

    ParticleEffectResource::ParticleEffectResource()
    {
    }
    ParticleEffectResource::~ParticleEffectResource()
    {

    }

	bool ParticleEffectResource::Load(const char* szFileName)
	{
		m_name = szFileName;
		m_name.RemovePath();
		m_name.TruncateExtension();
		SetupHash(szFileName);
		ProtocolBufferFile effectVPB(szFileName);
		bool bReadSucceeded = effectVPB.Read(&m_definition);	
		SetReady(true);
		return bReadSucceeded;
	}

	bool ParticleEffectResource::Load(GFXDevice* pDevice, const ParticleEffectPak& pak, const void* pData, const char* szPackPath)
	{
		UNUSED_VAR(pDevice);
		U8String fileName = szPackPath;
		fileName += pak.resHdr.strName;
		m_name = fileName;
		m_name.RemovePath();
		fileName += ".pfx";

		SetupHash(fileName.CStr());
		ProtocolBufferMemory effectVPB(pData, pak.resHdr.uDataSize);
		bool bReadSucceeded = effectVPB.Read(&m_definition);
		SetReady(true);
		return bReadSucceeded;
	}
}


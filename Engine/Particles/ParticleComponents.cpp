#include "Engine/Common/Common.h"
#include "Engine/Particles/ParticleComponents.pb.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/Resource/ParticleEffectResource.h"
#include "Engine/Resource/ParticleEmitterResource.h"

namespace usg
{
	template<>
	void OnDeactivate<ParticleComponent>(Component<ParticleComponent>& p, ComponentLoadHandles& handles)
	{
		if(p.GetRuntimeData().hndl.IsValid())
		{
			p.GetRuntimeData().hndl.Kill(false);
		}
	}

	void PreloadEffect(usg::GFXDevice* pDevice, const char* szName)
	{
		ResourceMgr::Inst()->GetParticleEffect(szName);
	}

	template<>
	void PreloadComponentAssets<ParticleComponent>(const usg::ComponentHeader& hdr, ProtocolBufferFile& file, ComponentLoadHandles& handles)
	{
		ParticleComponent component;
		bool readSuccess = file.Read(&component);
		ASSERT(readSuccess);
		if (component.name[0] != '\0')
		{
			PreloadEffect(handles.pDevice, component.name);
		}
	}

}

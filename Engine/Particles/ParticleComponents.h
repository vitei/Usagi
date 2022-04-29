/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _ENGINE_PARTICLES_PARTICLE_COMPONENT_H_
#define _ENGINE_PARTICLES_PARTICLE_COMPONENT_H_

#include "Engine/Framework/GameComponents.h"
#include "Engine/Particles/ParticleEffectHndl.h"


namespace usg
{
	template<>
	struct RuntimeData<ParticleComponent>
	{
		ParticleEffectHndl		hndl;
	};

	void PreloadEffect(usg::GFXDevice* pDevice, const char* szName);

	template<>
	void PreloadComponentAssets<ParticleComponent>(const usg::ComponentHeader& hdr, ProtocolBufferFile& file, ComponentLoadHandles& handles, usg::set<usg::string>& referencedEntities);
}


#endif


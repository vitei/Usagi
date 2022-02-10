/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_PARTICLE_SET_H_
#define _USG_PARTICLE_SET_H_

#include "Engine/Memory/MemHeap.h"

namespace usg
{
	class ParticleEffect;
	class GFXDevice;

	class ParticleSet
	{
	public:
		virtual void InitPools(GFXDevice* pDevice) = 0;
		virtual void FreePools() = 0;
		virtual bool CreateCustomEffect(usg::string name, ParticleEffect* pParent) = 0;
	protected:

	};

}

#endif // _USG_PARTICLE_SET_H_

/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/

#ifndef _CACHEABLE_COMPONENTS_H_
#define _CACHEABLE_COMPONENTS_H_

#include "Engine/Framework/CacheComponent.h"
#include "Engine/Framework/FrameworkComponents.pb.h"

namespace usg {

namespace Components
{
	typedef CacheInterval<TransformComponent> CachedTransform;
}

namespace Systems
{
	typedef CacheComponent<TransformComponent> CacheTransform;
}

}

#endif //_CACHEABLE_COMPONENTS_H_

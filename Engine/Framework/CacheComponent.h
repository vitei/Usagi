/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//
// CacheComponent is a generic system that allows you to save the state of a
// component at regular intervals.  In order to make use of it, you have to
// instantiate both the CacheInterval component and the CacheComponent system
// by using a typedef in the Components and Systems namespaces respectively.
// You can then expose these to .yml by making a from_header .proto file and
// including the header containing the typedefs.
//
// Example:
//     // Engine/Framework/CacheableComponents.h
//     namespace usg { // Only put in the usg namespace if it's a usg component
//         namespace Components { typedef CacheInterval<TransformComponent> CachedTransform; }
//         namespace Systems    { typedef CacheComponent<TransformComponent> CacheTransform; }
//     }
//
//     // Engine/Framework/CacheableComponents.proto
//     package usg.Components;
//     message CachedTransform
//     {
//         option (nanopb_msgopt).from_header = "Engine/Framework/CacheableComponents.h";
//         required float cacheIntervalSeconds = 1; // No need to expose anything other than cacheIntervalSeconds
//     }
//
****************************************************************************/

#ifndef _CACHE_COMPONENT_H_
#define _CACHE_COMPONENT_H_

#include "Engine/Framework/System.h"
#include "Engine/Framework/Component.h"
#include "Engine/Framework/GameComponents.h"
#include "Engine/Framework/FrameworkComponents.pb.h"
#include "Engine/Framework/SystemCoordinator.h"

namespace usg {

template<typename ComponentType>
struct CacheInterval
{
	float cacheIntervalSeconds;
	float secondsSinceLastCache;
	float secondsBehindOriginal;
};

template <typename ComponentType>
struct ComponentInitializer< CacheInterval<ComponentType> >
{
	static void Init(CacheInterval<ComponentType>* pComponent)
	{
		pComponent->cacheIntervalSeconds  = 0.f;
		pComponent->secondsSinceLastCache = 0.f;
		pComponent->secondsBehindOriginal = 0.f;
	}
};

template<typename ComponentType>
class CacheComponent : public System
{
public:
	typedef ComponentType ComponentTypeTP;
	typedef CacheInterval<ComponentTypeTP> CacheIntervalComp;
	static const SystemCategory CATEGORY = SYSTEM_CACHE;

	struct Inputs
	{
		Required< CacheIntervalComp > cache;
		Required< ComponentTypeTP, FromParents >   component;
		Optional< CacheIntervalComp,
		                         FromParents >   parentCache;
	};

	struct Outputs
	{
		Required< CacheIntervalComp > cache;
		Required< ComponentTypeTP >                cachedComponent;
	};

	static uint32 GetSystemId()
	{
		return usg::GetSystemId< CacheComponent<ComponentType> >();
	}

	static const char* Name()
	{
		static const char* const name = "CacheComponent"; // Would be good to include the component name
		return name;
	}

	static bool GetInputOutputs(ComponentGetter& GetComponent, Inputs& inputs, Outputs& outputs)
	{
		return GetComponent(inputs.cache, outputs.cache) &&
		       GetComponent(inputs.component)            &&
			   GetComponent(inputs.parentCache)          &&
			   GetComponent(outputs.cachedComponent);
	}

	static void Run(const Inputs& in, Outputs& out, float fDelta)
	{
		CacheInterval<ComponentType>& cache = out.cache.Modify();

		float secondsPassed = in.cache->secondsSinceLastCache + fDelta;
		if(secondsPassed > in.cache->cacheIntervalSeconds)
		{
			secondsPassed -= in.cache->cacheIntervalSeconds;
			out.cachedComponent.Modify() = *in.component;

			float secondsBehindOriginal = in.parentCache.Exists()
				? in.parentCache.Force()->secondsBehindOriginal : 0.f;
			secondsBehindOriginal += in.cache->cacheIntervalSeconds;
			cache.secondsBehindOriginal = secondsBehindOriginal;
		}

		cache.secondsSinceLastCache = secondsPassed;
	}
};

}

#endif //_CACHE_COMPONENT_H_

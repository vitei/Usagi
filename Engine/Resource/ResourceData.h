/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Manages a group of named resources
*****************************************************************************/
#pragma once

#ifndef USG_RESOURCE_RESOURCE_DATA_H
#define USG_RESOURCE_RESOURCE_DATA_H
#include "Engine/Common/Common.h"
#include "Engine/Core/String/U8String.h"


//#define DEBUG_RESOURCE_MGR

#ifdef DEBUG_RESOURCE_MGR
#include "Engine/Core/Timer/ProfilingTimer.h"
#endif


namespace usg{

class GFXDevice;

class ResourceDataBase
{
public:
	ResourceDataBase()
	: m_uTag(0)
	, m_bLoadAsStatic(true)
	{
	}

	~ResourceDataBase() {}

	void SetTag(uint32 uTag) { m_uTag = uTag; }
	void SetStaticLoading(bool bStaticLoading) { m_bLoadAsStatic = bStaticLoading; }
	virtual void FreeResourcesWithTag(GFXDevice* pDevice, uint32 uTag) = 0;
	virtual void FreeAllResources(GFXDevice* pDevice) = 0;
	virtual void GarbageCollectUnreferencedResources() = 0;

#ifdef DEBUG_RESOURCE_MGR
	void StartLoad() { m_loadTimer.Start(); }
	void ClearTimers() { m_loadTimer.Clear(); m_findTimer.Clear(); }
	float GetLoadTime() { return m_loadTimer.GetTotalSeconds(); }
	float GetFindTime() { return m_findTimer.GetTotalSeconds(); }
#else
	void StartLoad() {  }
	void ClearTimers() {  }
	float GetLoadTime() { return 0.0f;  }
	float GetFindTime() { return 0.0f; }
#endif

public:
#ifdef DEBUG_RESOURCE_MGR
	ProfilingTimer	m_findTimer;
	ProfilingTimer	m_loadTimer;
#endif

	uint32	m_uTag;
	bool	m_bLoadAsStatic;
};

template <class ResourceType>
class ResourceData : public ResourceDataBase
{
private:
	struct ResourceInfo
	{
		ResourcePointer<const ResourceType>	resource;
		uint32								uTag;
		bool								bStatic;
	};

public:
	ResourceData();
	~ResourceData();

	typedef typename FastPool<ResourceInfo>::DynamicIterator ResourceDynamicIter;
	typedef typename FastPool<ResourceInfo>::Iterator ResourceIter;

	virtual void FreeResourcesWithTag(GFXDevice* pDevice, uint32 uTag)
	{
		for (ResourceDynamicIter it = m_resources.BeginDynamic(); !it.IsEnd(); ++it)
		{
			ResourceInfo* val = *it;
			if (val->uTag == uTag)
			{
				// Ensure it's not being used elsewhere
				// FIXME: There is a memory leak going on so can't check in this assert yet
				//ASSERT((*it)->resource.unique());
				const_cast<ResourceType*>(val->resource.get())->CleanUp(pDevice);
				vdelete val->resource.get();
				val->resource.reset();
				it.RemoveElement();
			}
		}
	}

	virtual void FreeAllResources(GFXDevice* pDevice)
	{
		for (ResourceDynamicIter it = m_resources.BeginDynamic(); !it.IsEnd(); ++it)
		{
			ResourceInfo* val = *it;
			const_cast<ResourceType*>(val->resource.get())->CleanUp(pDevice);
			vdelete val->resource.get();
			val->resource.reset();
			it.RemoveElement();
		}
	}

	virtual void GarbageCollectUnreferencedResources()
	{
		for (ResourceDynamicIter it = m_resources.BeginDynamic(); !it.IsEnd(); ++it)
		{
			ResourceInfo* val = *it;
			if ((*it)->resource.unique() && !val->bStatic)
			{
				// Ensure it's not being used elsewhere
				vdelete val->resource.get();
				val->resource.reset();
				it.RemoveElement();
			}
		}
	}
	 
	ResourcePointer<const ResourceType> GetResourceHndl(const U8String& resName);
	ResourcePointer<const ResourceType> AddResource(const ResourceType* pResource);


	uint32 GetResourceCount() const { return m_resources.Size(); }

	// TODO: Remove this functions accessing the raw resource
	const ResourceType* GetResource(const U8String &resName);
	const ResourceType* GetResource(uint32 uIndex) const
	{
		// Slow!!! Do not use outside of init
		ResourceIter it = m_resources.Begin();
		for (; !it.IsEnd() && uIndex; ++it, uIndex--)
		{	
			
		}
		ASSERT(uIndex == 0);
		return (*it)->resource.get();
	}


private:
	FastPool<ResourceInfo>	m_resources;

};


template <class ResourceType>
inline ResourceData<ResourceType>::ResourceData() : m_resources(50, true, true)
{
}

template <class ResourceType>
inline ResourceData<ResourceType>::~ResourceData()
{
	ASSERT(m_resources.Size() == 0);
}


template <class ResourceType>
inline ResourcePointer<const ResourceType> ResourceData<ResourceType>::AddResource(const ResourceType* pResource)
{
#ifdef DEBUG_RESOURCE_MGR
	m_loadTimer.Stop();
#endif
	ResourcePointer<const ResourceType> ret;
	// TODO: Remove me from a final build
	ResourceInfo* pInfo = m_resources.Alloc();
	pInfo->resource = pResource;
	pInfo->uTag = m_uTag;
	pInfo->bStatic = m_bLoadAsStatic;
	ret = pInfo->resource;

	return ret;
}

template <class ResourceType>
ResourcePointer<const ResourceType> ResourceData<ResourceType>::GetResourceHndl(const U8String& resName)
{
	// TODO: Bad for cache misses and completely unsorted, create a lookup table
	NameHash nameHash = ResourceDictionary::calcNameHash( resName.CStr() );
	DataHash dataHash = ResourceDictionary::searchDataHashByName( nameHash );
	
#ifdef DEBUG_RESOURCE_MGR
	m_findTimer.Start();
#endif
	for (typename FastPool<ResourceInfo>::Iterator it = m_resources.Begin(); !it.IsEnd(); ++it)
	{
		if( (*it)->resource->GetNameHash() == nameHash
			|| ((*it)->resource->GetDataHash() == dataHash && dataHash != 0 ) )
		{
#ifdef DEBUG_RESOURCE_MGR
			m_findTimer.Stop();
#endif
			return (*it)->resource;
		}
	}

#ifdef DEBUG_RESOURCE_MGR
	m_findTimer.Stop();
#endif

	return ResourcePointer<const ResourceType>(nullptr);
}

template <class ResourceType>
const ResourceType* ResourceData<ResourceType>::GetResource(const U8String &resName)
{
	return GetResourceHndl(resName).get();
}

} // namespace usg

#endif // USG_RESOURCE_RESOURCE_DATA_H


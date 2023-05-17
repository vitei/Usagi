/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Manages a group of named resources
*****************************************************************************/
#pragma once

#ifndef USG_RESOURCE_RESOURCE_DATA_H
#define USG_RESOURCE_RESOURCE_DATA_H

#include "Engine/Core/stl/string.h"


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

class ResourceData : public ResourceDataBase
{
private:
	struct ResourceInfo
	{
		BaseResHandle		resource;
		uint32				uTag;
		bool				bStatic;
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
				const_cast<ResourceBase*>(val->resource.get())->Cleanup(pDevice);
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
			const_cast<ResourceBase*>(val->resource.get())->Cleanup(pDevice);
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
	 
	BaseResHandle GetResourceHndl(const usg::string& resName, ResourceType eType);
	BaseResHandle AddResource(const ResourceBase* pResource);
	void AddResource(BaseResHandle resHndl);


	uint32 GetResourceCount() const { return m_resources.Size(); }

	// TODO: Remove this functions accessing the raw resource
	template <class ResourceType>
	const ResourceType* GetResource(const usg::string&resName, usg::ResourceType eType);
	
	template <class ResourceType>
	const ResourceType* GetResource(uint32 uIndex) const
	{
		// Slow!!! Do not use outside of init
		ResourceIter it = m_resources.Begin();
		for (; !it.IsEnd() && uIndex; ++it, uIndex--)
		{	
			
		}
		ASSERT(uIndex == 0);
		return GetAs<ResourceType>((*it)->resource);
	}


private:
	FastPool<ResourceInfo>	m_resources;

};


inline ResourceData::ResourceData() : m_resources(50, true, true)
{
}

inline ResourceData::~ResourceData()
{
	ASSERT(m_resources.Size() == 0);
}


inline BaseResHandle ResourceData::AddResource(const ResourceBase* pResource)
{
#ifdef DEBUG_RESOURCE_MGR
	m_loadTimer.Stop();
#endif
	BaseResHandle ret;
	// TODO: Remove me from a final build
	ResourceInfo* pInfo = m_resources.Alloc();
	pInfo->resource = pResource;
	pInfo->uTag = m_uTag;
	pInfo->bStatic = m_bLoadAsStatic;
	ret = pInfo->resource;

	return ret;
}

inline void ResourceData::AddResource(BaseResHandle resHandle)
{
#ifdef DEBUG_RESOURCE_MGR
	m_loadTimer.Stop();
#endif
	ResourceInfo* pInfo = m_resources.Alloc();
	pInfo->resource = resHandle;
	pInfo->uTag = m_uTag;
	pInfo->bStatic = m_bLoadAsStatic;
}


BaseResHandle ResourceData::GetResourceHndl(const usg::string& resName, ResourceType eType)
{
	// TODO: Bad for cache misses and completely unsorted, create a lookup table
	NameHash nameHash = ResourceDictionary::calcNameHash( resName.c_str() );
	DataHash dataHash = ResourceDictionary::searchDataHashByName( nameHash );
	
#ifdef DEBUG_RESOURCE_MGR
	m_findTimer.Start();
#endif
	for (typename FastPool<ResourceInfo>::Iterator it = m_resources.Begin(); !it.IsEnd(); ++it)
	{
		if(  (*it)->resource->GetNameHash() == nameHash
			&& (*it)->resource->GetResourceType() == eType )
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

	return BaseResHandle(nullptr);
}

template <class ResourceType>
const ResourceType* ResourceData::GetResource(const usg::string &resName, usg::ResourceType eType)
{
	return GetAs<ResourceType>(GetResourceHndl(resName, eType));
}

} // namespace usg

#endif // USG_RESOURCE_RESOURCE_DATA_H


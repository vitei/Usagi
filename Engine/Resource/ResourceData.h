/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Manages a group of named resources
*****************************************************************************/
#pragma once

#ifndef USG_RESOURCE_RESOURCE_DATA_H
#define USG_RESOURCE_RESOURCE_DATA_H

#include "Engine/Core/stl/string.h"
#include "Engine/Core/stl/vector.h"


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
	struct ResourceRequestDependency
	{
		ResourceRequestDependency()
			: uFileCRC(0)
			, uUsageCRC(0)
		{
		}

		ResourceRequestDependency(const usg::string& dependencyName, uint32 uDependencyFileCRC, uint32 uDependencyUsageCRC)
			: name(dependencyName)
			, uFileCRC(uDependencyFileCRC)
			, uUsageCRC(uDependencyUsageCRC)
		{
		}

		usg::string	name;
		uint32		uFileCRC;
		uint32		uUsageCRC;
	};

	struct ResourceRequest
	{
		ResourceRequest()
			: eType(ResourceType::UNDEFINED)
			, uPriority(0)
			, uTag(0)
			, bStatic(true)
			, eState(ResourceState::REQUESTED)
		{
		}

		usg::string								name;
		ResourceType							eType;
		uint32									uPriority;
		uint32									uTag;
		bool									bStatic;
		ResourceState							eState;
		usg::vector<ResourceRequestDependency>	dependencies;
		BaseResHandle							resource;
	};

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

	ResourceRequest& QueueRequest(const usg::string& resName, ResourceType eType, uint32 uPriority = 0);
	ResourceRequest* GetNextQueuedRequest();
	const ResourceRequest* GetNextQueuedRequest() const;
	ResourceRequest* FindRequest(const usg::string& resName, ResourceType eType);
	const ResourceRequest* FindRequest(const usg::string& resName, ResourceType eType) const;
	void AddRequestDependency(ResourceRequest& request, const usg::string& dependencyName, uint32 uFileCRC, uint32 uUsageCRC);
	void SetRequestState(ResourceRequest& request, ResourceState eState);
	void MarkRequestCpuLoading(ResourceRequest& request);
	void MarkRequestWaitingDependencies(ResourceRequest& request);
	void MarkRequestCpuReady(ResourceRequest& request);
	void MarkRequestQueuedGpuUpload(ResourceRequest& request);
	void MarkRequestGpuUploading(ResourceRequest& request);
	void CompleteRequest(ResourceRequest& request, BaseResHandle resHandle);
	void FailRequest(ResourceRequest& request);
	void CancelRequest(ResourceRequest& request);
	bool IsRequestTerminal(const ResourceRequest& request) const;
	bool HasInflightRequests() const;
	void ClearCompletedRequests();
	uint32 GetRequestCount() const { return (uint32)m_requests.size(); }
	bool HasQueuedRequests() const { return GetNextQueuedRequest() != nullptr; }
	void ClearRequests() { m_requests.clear(); }

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
	usg::vector<ResourceRequest>	m_requests;

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

inline ResourceData::ResourceRequest& ResourceData::QueueRequest(const usg::string& resName, ResourceType eType, uint32 uPriority)
{
	ResourceRequest* pRequest = FindRequest(resName, eType);
	if (pRequest != nullptr)
	{
		if (IsRequestTerminal(*pRequest))
		{
			pRequest->uPriority = uPriority;
			pRequest->uTag = m_uTag;
			pRequest->bStatic = m_bLoadAsStatic;
			pRequest->eState = ResourceState::REQUESTED;
			pRequest->dependencies.clear();
			pRequest->resource.reset();
			return *pRequest;
		}

		if (uPriority > pRequest->uPriority)
		{
			pRequest->uPriority = uPriority;
		}
		return *pRequest;
	}

	ResourceRequest request;
	request.name = resName;
	request.eType = eType;
	request.uPriority = uPriority;
	request.uTag = m_uTag;
	request.bStatic = m_bLoadAsStatic;
	request.eState = ResourceState::REQUESTED;
	m_requests.push_back(request);
	return m_requests.back();
}

inline ResourceData::ResourceRequest* ResourceData::GetNextQueuedRequest()
{
	ResourceRequest* pNextRequest = nullptr;
	for (uint32 i = 0; i < m_requests.size(); ++i)
	{
		if (m_requests[i].eState != ResourceState::REQUESTED)
		{
			continue;
		}

		if (pNextRequest == nullptr || m_requests[i].uPriority > pNextRequest->uPriority)
		{
			pNextRequest = &m_requests[i];
		}
	}

	return pNextRequest;
}

inline const ResourceData::ResourceRequest* ResourceData::GetNextQueuedRequest() const
{
	return const_cast<ResourceData*>(this)->GetNextQueuedRequest();
}

inline ResourceData::ResourceRequest* ResourceData::FindRequest(const usg::string& resName, ResourceType eType)
{
	for (uint32 i = 0; i < m_requests.size(); ++i)
	{
		if (m_requests[i].name == resName && m_requests[i].eType == eType)
		{
			return &m_requests[i];
		}
	}

	return nullptr;
}

inline const ResourceData::ResourceRequest* ResourceData::FindRequest(const usg::string& resName, ResourceType eType) const
{
	return const_cast<ResourceData*>(this)->FindRequest(resName, eType);
}

inline void ResourceData::AddRequestDependency(ResourceRequest& request, const usg::string& dependencyName, uint32 uFileCRC, uint32 uUsageCRC)
{
	request.dependencies.push_back(ResourceRequestDependency(dependencyName, uFileCRC, uUsageCRC));
}

inline void ResourceData::SetRequestState(ResourceRequest& request, ResourceState eState)
{
	request.eState = eState;
}

inline void ResourceData::MarkRequestCpuLoading(ResourceRequest& request)
{
	SetRequestState(request, ResourceState::CPU_LOADING);
}

inline void ResourceData::MarkRequestWaitingDependencies(ResourceRequest& request)
{
	SetRequestState(request, ResourceState::WAITING_DEPENDENCIES);
}

inline void ResourceData::MarkRequestCpuReady(ResourceRequest& request)
{
	SetRequestState(request, ResourceState::CPU_READY);
}

inline void ResourceData::MarkRequestQueuedGpuUpload(ResourceRequest& request)
{
	SetRequestState(request, ResourceState::QUEUED_GPU_UPLOAD);
}

inline void ResourceData::MarkRequestGpuUploading(ResourceRequest& request)
{
	SetRequestState(request, ResourceState::GPU_UPLOADING);
}

inline void ResourceData::CompleteRequest(ResourceRequest& request, BaseResHandle resHandle)
{
	request.resource = resHandle;
	request.eState = (resHandle.get() != nullptr) ? ResourceState::READY : ResourceState::FAILED;
}

inline void ResourceData::FailRequest(ResourceRequest& request)
{
	request.resource.reset();
	SetRequestState(request, ResourceState::FAILED);
}

inline void ResourceData::CancelRequest(ResourceRequest& request)
{
	if (!IsRequestTerminal(request))
	{
		request.resource.reset();
		SetRequestState(request, ResourceState::CANCELLED);
	}
}

inline bool ResourceData::IsRequestTerminal(const ResourceRequest& request) const
{
	return request.eState == ResourceState::READY
		|| request.eState == ResourceState::FAILED
		|| request.eState == ResourceState::CANCELLED;
}

inline bool ResourceData::HasInflightRequests() const
{
	for (uint32 i = 0; i < m_requests.size(); ++i)
	{
		if (!IsRequestTerminal(m_requests[i]))
		{
			return true;
		}
	}

	return false;
}

inline void ResourceData::ClearCompletedRequests()
{
	for (uint32 i = 0; i < m_requests.size();)
	{
		if (IsRequestTerminal(m_requests[i]))
		{
			m_requests.erase(m_requests.begin() + i);
			continue;
		}

		++i;
	}
}


BaseResHandle ResourceData::GetResourceHndl(const usg::string& resName, ResourceType eType)
{
	// TODO: Bad for cache misses and completely unsorted, create a lookup table
	NameHash nameHash = ResourceDictionary::calcNameHash( resName.c_str() );
//	DataHash dataHash = ResourceDictionary::searchDataHashByName( nameHash );
	
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

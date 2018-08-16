/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: An optional system for managing allocated and freed models
//	I still really think we ought to be allowing new and delete rather than
//	this kind of ugly ****
*****************************************************************************/
#ifndef _USG_FRAMEWORK_INSTANCE_MGR_
#define _USG_FRAMEWORK_INSTANCE_MGR_
#include "Engine/Common/Common.h"
#include "Engine/Core/Containers/List.h"
#include "Engine/Graphics/Device/GFXDevice.h"

namespace usg
{

	class Model;
	class Scene;

	template <class InstanceType>
	class InstanceMgr
	{
	public:
		InstanceMgr(void): m_pool(50, true) {  }
		~InstanceMgr(void) {} 

		void Init(GFXDevice* pDevice, Scene* pScene)
		{
			m_pDevice = pDevice;
			m_pScene = pScene;
		}

		virtual void Free(InstanceType* pInstance)
		{
			if (!pInstance)
				return;

			// Device not needed to remove
			pInstance->AddToScene(nullptr, false);
			if (m_inUseList.Remove(pInstance))
			{
				m_freeList.AddToEnd(pInstance);
			}
			else
			{
				DEBUG_PRINT("Attempt to free an instance we don't have a record of allocating");
			}
		}


		void Destroy(GFXDevice* pDevice)
		{
			InstanceType* pReturn = NULL;
			for (typename List<InstanceType>::Iterator it = m_freeList.Begin(); !it.IsEnd(); ++it)
			{
				(*it)->CleanUp(pDevice);
			}
		}


	protected:
		InstanceType* GetFreeInstance(U8String &name)
		{
			InstanceType* pReturn = NULL;
			for (typename List<InstanceType>::Iterator it = m_freeList.Begin(); !it.IsEnd(); ++it)
			{
				if ((*it)->GetName() == name)
				{
					pReturn = (*it);
					break;
				}
			}

			if (pReturn)
			{
				m_freeList.Remove(pReturn);
				m_inUseList.AddToEnd(pReturn);
			}

			return pReturn;
		}

		GFXDevice*					m_pDevice;
		Scene*						m_pScene;

		FastPool<InstanceType>		m_pool;
		List<InstanceType>			m_inUseList;
		List<InstanceType>			m_freeList;

	};


}

#endif



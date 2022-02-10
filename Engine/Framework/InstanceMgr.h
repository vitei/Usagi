/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: An optional system for managing allocated and freed models
//	I still really think we ought to be allowing new and delete rather than
//	this kind of ugly ****
*****************************************************************************/
#ifndef _USG_FRAMEWORK_INSTANCE_MGR_
#define _USG_FRAMEWORK_INSTANCE_MGR_

#include "Engine/Core/stl/list.h"
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
			pInstance->ForceRemoveFromScene();

			m_inUseList.remove(pInstance);
			m_freeList.push_back(pInstance);
		}


		void Destroy(GFXDevice* pDevice)
		{
			InstanceType* pReturn = NULL;
			for (auto it = m_freeList.begin(); it != m_freeList.end(); ++it)
			{
				(*it)->Cleanup(pDevice);
			}
		}


	protected:
		InstanceType* GetFreeInstance(usg::string &name)
		{
			InstanceType* pReturn = NULL;
			for (auto it = m_freeList.begin(); it != m_freeList.end(); ++it)
			{
				if ((*it)->GetName() == name)
				{
					pReturn = (*it);
					break;
				}
			}

			if (pReturn)
			{
				m_freeList.remove(pReturn);
				m_inUseList.push_back(pReturn);
			}

			return pReturn;
		}

		GFXDevice*					m_pDevice;
		Scene*						m_pScene;

		FastPool<InstanceType>		m_pool;
		usg::list<InstanceType*>	m_inUseList;
		usg::list<InstanceType*>	m_freeList;

	};


}

#endif



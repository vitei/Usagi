/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: A utility class for creating a mode, sets up the basic handles
//  required by the systems
*****************************************************************************/
#ifndef _USAGI_SYSTEMS_MODE_H_
#define _USAGI_SYSTEMS_MODE_H_
#include "Engine/Common/Common.h"
#include "Mode.h"


namespace usg
{
	class ComponentEntity;
	class Scene;
	class SystemCoordinator;
	class ComponentManager;

	typedef void(*RegisterSystemsFn) (usg::SystemCoordinator& systemCoordinator);

	class SystemsMode : public Mode, public UnsafeComponentGetter
	{
	public:
		SystemsMode(RegisterSystemsFn fnGameSysRegister);
		virtual ~SystemsMode();

		virtual void Init(GFXDevice* pDevice) override;
		virtual void CleanUp(GFXDevice* pDevice) override;
		virtual bool Update(float fElapsed) override;

		virtual void InitNetworking(UsagiNet& usagiNetwork) override;
		
		ComponentManager* GetComponentMgr();

	protected:
		ComponentEntity* GetRootEntity();
		Scene& GetScene();
	private:
		struct InternalData;
		unique_ptr<InternalData> m_pImpl;

		RegisterSystemsFn m_registerSystemsFn;
		void CreateEntityHierarchy();
	};
}

#endif 

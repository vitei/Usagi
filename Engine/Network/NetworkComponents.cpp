/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2017
****************************************************************************/

#include "Engine/Common/Common.h"
#include "Engine/Framework/Component.h"
#include "Engine/Network/NetworkComponents.pb.h"
#include "Engine/Framework/FrameworkComponents.pb.h"

namespace usg
{
	template<>
	void OnLoaded<HostManagedEntity>(Component<HostManagedEntity>& c, ComponentLoadHandles& handles, bool bWasPreviouslyCalled)
	{
		Optional<LocalSim> localSim;
		GetComponent(c.GetEntity(), localSim);
		if (localSim.Exists())
		{
			localSim.Force().Deactivate(handles);
		}

		NetworkClient* pClientComponent = GameComponents<NetworkClient>::GetComponentData(c.GetEntity());
		if (pClientComponent == NULL)
		{
			pClientComponent = GameComponents<NetworkClient>::Create(c.GetEntity());
		}
		pClientComponent->clientUID = NUID_INVALID;
	}
}
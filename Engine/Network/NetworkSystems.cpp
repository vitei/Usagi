/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2017
****************************************************************************/

#include "Engine/Common/Common.h"
#include "Engine/Framework/EventManager.h"
#include "Engine/Framework/System.h"
#include "Engine/Framework/SystemCategories.h"
#include "Engine/Network/NetworkComponents.pb.h"
#include "Engine/Network/NetManager.h"

namespace usg
{
	namespace details
	{
		struct ChangedSetter { void operator()(Entity c) { c->SetChanged(); } };
	}

	namespace Systems
	{
		class HostManagedEntitySystem : public System
		{
		public:   
			struct Inputs
			{
				Required<HostManagedEntity> hostManagedEntity;
			};
        
			struct Outputs
			{
				Required<usg::NetworkClient> netClient;
			};
    
			DECLARE_SYSTEM(SYSTEM_PRE_EARLY)

			static void Run(const Inputs& inputs, Outputs& outputs, float32 fDelta)
			{
				NetManager* pNetManager = NetManager::Inst();
				if (pNetManager != nullptr)
				{
					Entity e = inputs.hostManagedEntity.GetEntity();
					const bool bIsHost = pNetManager->IsHost();
					if (bIsHost)
					{
						const sint64 iMyUID = pNetManager->GetUID();
						if (outputs.netClient->clientUID != iMyUID)
						{
							outputs.netClient.Modify().clientUID = iMyUID;
							DEBUG_PRINT("HostManagedEntity: set client uid to MY uid of %u\n",(uint32)outputs.netClient.Modify().clientUID);
						}

						LocalSim* pLocalSim = GameComponents<LocalSim>::GetComponentData(e);
						if (pLocalSim == NULL)
						{
							pLocalSim = GameComponents<LocalSim>::Create(e);
							e->ProcessEntityRecursively(details::ChangedSetter());
							DEBUG_PRINT("HostManagedEntity: created LocalSim\n");
						}
						pLocalSim->bCPUPlayer = inputs.hostManagedEntity->bLocalSimCPUPlayerValue;
					}
					else
					{
						const sint64 iHostUID = pNetManager->GetHost() ? pNetManager->GetHost()->GetUID() : NUID_INVALID;
						if (outputs.netClient->clientUID != iHostUID)
						{
							outputs.netClient.Modify().clientUID = iHostUID;
							DEBUG_PRINT("HostManagedEntity: set client uid to HOST uid of %u\n", (uint32)iHostUID);
						}

						// No longer allowing GetComponent globaly
						ASSERT(false);
#if 0
						Optional<LocalSim> localSim;
						GetComponent(e, localSim);
						if (localSim.Exists())
						{
							DEBUG_PRINT("Deactivated local sim.\n");
							//localSim.Force().Deactivate();
							ASSERT(false);	// Need to be able to manually deactivate components
							e->ProcessEntityRecursively(details::ChangedSetter());
						}
#endif
					}
				}
			}
		};
	}
}

#include GENERATED_SYSTEM_CODE(Engine/Network/NetworkSystems.cpp)
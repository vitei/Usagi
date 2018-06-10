#include "Engine/Common/Common.h"
#include "Engine/Framework/GameComponents.h"
#include "Engine/Physics/PhysicsComponents.pb.h"

namespace usg
{
	void OnVehicleColliderActivated(Component<Components::VehicleCollider>& c);
	void OnVehicleColliderLoaded(Component<Components::VehicleCollider>& c, ComponentLoadHandles& handles);
	void OnVehicleColliderDeactivated(Component<Components::VehicleCollider>& c, ComponentLoadHandles& handles);
}
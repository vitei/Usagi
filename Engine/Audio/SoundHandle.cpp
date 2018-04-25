#include "Engine/Common/Common.h"
#include "SoundHandle.h"
#include "Engine/Framework/SystemCoordinator.h"

namespace usg
{
	template<> void RegisterComponent<SoundHandle>(SystemCoordinator& systemCoordinator) { systemCoordinator.RegisterComponent<SoundHandle>(); }
}
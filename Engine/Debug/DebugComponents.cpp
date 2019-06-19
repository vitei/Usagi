#include "Engine/Common/Common.h"
#include "Engine/Debug/DebugCamera.h"
#include "Engine/Framework/FrameworkComponents.pb.h"
#include "Engine/Framework/ComponentLoadHandles.h"
#include "Engine/Debug/DebugComponents.pb.h"

namespace usg
{

	template<>
	void OnActivate<DebugCameraComponent>(Component<DebugCameraComponent>& p)
	{
		p.GetRuntimeData().pDebugCam = nullptr;
	}

	template<>
	void OnLoaded<DebugCameraComponent>(Component<DebugCameraComponent>& p, ComponentLoadHandles& handles, bool bWasPreviouslyCalled)
	{
		if (!bWasPreviouslyCalled)
		{
			p.GetRuntimeData().pDebugCam = vnew(ALLOC_OBJECT) DebugCamera;
		}
	}

	template<>
	void OnDeactivate<DebugCameraComponent>(Component<DebugCameraComponent>& p, ComponentLoadHandles& handles)
	{
		if(p.GetRuntimeData().pDebugCam)
		{
			vdelete p.GetRuntimeData().pDebugCam;
			p.GetRuntimeData().pDebugCam = NULL;
		}
	}
}

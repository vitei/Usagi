#include "Engine/Common/Common.h"
#include "Engine/Framework/FrameworkComponents.pb.h"
#include "Engine/Framework/CacheableComponents.h"
#include "Engine/Framework/SystemCoordinator.h"

namespace usg
{

	template<> void RegisterComponent<usg::CacheInterval<TransformComponent>>(SystemCoordinator& systemCoordinator) { systemCoordinator.RegisterComponent<usg::CacheInterval<TransformComponent>>(); }

	template<>
	void OnLoaded(Component<Identifier>& c, ComponentLoadHandles& handles, bool bWasPreviouslyCalled)
	{
		if (!bWasPreviouslyCalled)
		{
			auto& rtd = c.GetRuntimeData();
			rtd.uNameHash = utl::CRC32(c->name);
		}
	}

template<>
void OnActivate<usg::Components::ActiveDevice>(Component<usg::Components::ActiveDevice>& c)
{
	c.GetRuntimeData().pDevice = NULL;
}

template<>
void OnDeactivate<usg::Components::ActiveDevice>(Component<usg::Components::ActiveDevice>& c, ComponentLoadHandles& handles)
{
	c.GetRuntimeData().pDevice = NULL;
}

template<>
void OnLoaded<MatrixComponent>(Component<MatrixComponent>& m, ComponentLoadHandles& handles, bool bWasPreviouslyCalled)
{
	// Hack to ensure matrix is initialized from the transform component
	Entity e = m.GetEntity();
	Required<usg::TransformComponent> trans;
	MatrixComponent* mat = &m.GetData();
	handles.GetComponent(e, trans);
	if(trans.IsValid())
	{
		mat->matrix = trans->rotation;
		mat->matrix.SetTranslation( trans->position );
	}
}


}

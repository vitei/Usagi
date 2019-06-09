#pragma once


#include "Engine/AI/Targetting/TargetComponents.pb.h"
#include "Engine/Core/stl/vector.h"

namespace usg
{
	template<>
	struct RuntimeData<ai::Components::TargetListComponent>
	{
		vector<ai::TargetListData> targets;
	};

}
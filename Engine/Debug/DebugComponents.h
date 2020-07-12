/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#pragma once

#include "Engine/Framework/Component.h"

namespace usg
{
	class DebugCamera;

	template<>
	struct RuntimeData<usg::Components::DebugCameraComponent>
	{
		usg::DebugCamera*				pDebugCam;
	};
}

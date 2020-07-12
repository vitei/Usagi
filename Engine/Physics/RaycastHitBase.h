#pragma once

#include "Engine/Maths/Vector3f.h"

namespace usg
{
	struct RaycastHitBase
	{
		Vector3f vPosition;
		Vector3f vNormal;
		float fDistance;
		uint32 uMaterialFlags;
		uint32 uGroup;
	};
}


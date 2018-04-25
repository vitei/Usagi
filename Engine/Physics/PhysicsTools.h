/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2016
****************************************************************************/

#pragma once

#include "Engine/Common/Common.h"

namespace physx
{
	class PxShape;
}

namespace usg
{
	namespace physics
	{
		namespace details
		{
			uint32 FetchMaterialFlags(physx::PxShape* pShape, uint32 uInternalFaceIndex);
		}
	}
}

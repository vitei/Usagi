/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/

#pragma once


#include "Engine/Framework/FrameworkComponents.pb.h"

namespace usg
{
	class TransformTool
	{
	public:
		// Get child's position and rotation in parent's local space
		static TransformComponent GetRelativeTransform(Entity parent, Entity child, ComponentLoadHandles& handles);

		static Vector3f TransformVector3(const TransformComponent& trans, const Vector3f& v);
	};
}
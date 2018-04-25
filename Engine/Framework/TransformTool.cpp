#include "Engine/Common/Common.h"
#include "Engine/Framework/TransformTool.h"
#include "Engine/Physics/PhysX.h"

namespace usg
{

	static TransformComponent GetTransform(Entity e)
	{
		Optional<TransformComponent> trans;
		GetComponent(e, trans);
		if (trans.Exists())
		{
			return *trans.Force();
		}
		TransformComponent r;
		TransformComponent_init(&r);
		return r;
	}

	TransformComponent TransformTool::GetRelativeTransform(Entity parent, Entity child)
	{
		TransformComponent usgTrans = GetTransform(child);
		if (!usgTrans.bInheritFromParent)
		{
			physx::PxTransform p = ToPhysXTransform(usgTrans);
			physx::PxTransform parentTrans = ToPhysXTransform(GetTransform(parent));

			return physics::ToUsgTransform(parentTrans.transformInv(p));
		}
		physx::PxTransform trans = ToPhysXTransform(usgTrans);
		Entity e = child;
		while (e->GetParentEntity() != parent)
		{
			physx::PxTransform parentTrans = ToPhysXTransform(GetTransform(e->GetParentEntity()));
			trans = trans.transform(parentTrans);
			e = e->GetParentEntity();
		}
		usgTrans.position = ToUsgVec3(trans.p);
		usgTrans.rotation = ToUsgQuaternionf(trans.q);
		return usgTrans;
	}

	Vector3f TransformTool::TransformVector3(const TransformComponent& trans, const Vector3f& v)
	{
		return v*trans.rotation + trans.position;
	}
}
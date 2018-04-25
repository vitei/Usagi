#include "Engine/Common/Common.h"
#include "Engine/Scene/Model/BoneComponentUtil.h"
#include "Engine/Framework/FrameworkComponents.pb.h"
#include "Engine/Scene/Model/ModelComponents.pb.h"
#include "Engine/Scene/Model/Model.h"
#include "Engine/Scene/Model/Bone.h"
#include "Engine/Scene/Model/ModelComponents.pb.h"


namespace usg
{

template<>
void OnLoaded<IntermediateBone>(Component<IntermediateBone>& c, ComponentLoadHandles& handles, bool bWasPreviouslyCalled)
{
	ProcessIntermediateBoneEntity(c.GetEntity(), bWasPreviouslyCalled);
}

template<>
void OnLoaded<BoneComponent>(Component<BoneComponent>& c, ComponentLoadHandles& handles, bool bWasPreviouslyCalled)
{
	ProcessBoneEntity(c.GetEntity(), handles, bWasPreviouslyCalled);
}

template<>
void OnActivate<BoneComponent>(Component<BoneComponent>& c)
{
	c.GetRuntimeData().pBone = NULL;
}

void InitializeBones(Entity e, ComponentLoadHandles& handles, bool bWasPreviouslyCalled)
{
	ASSERT(e != NULL);

	struct Inputs
	{
		Required<Identifier> id;
		Required<ModelComponent, FromParents> model;
	};

	struct Outputs
	{
		Required<BoneComponent>	bone;
		Optional<ModelComponent> model;
	};

	Inputs inputs;
	Outputs outputs;

	bool bDidSetInputOutputs =
		GetComponent(e, inputs.id) &&
		GetComponent(e, inputs.model) &&
		GetComponent(e, outputs.bone) &&
		GetComponent(e, outputs.model);

	if (!bDidSetInputOutputs)
	{
		return;
	}

	if (outputs.model.Exists() && outputs.model.Force().GetRuntimeData().pModel == NULL) {
		InitModel(outputs.model.Force(), handles);
	}

	outputs.bone.GetRuntimeData().pBone = inputs.model.GetRuntimeData().pModel->GetSkeleton().GetBone(inputs.id->name);
	outputs.bone.Modify().uIndex = inputs.model.GetRuntimeData().pModel->GetSkeleton().GetResource()->GetBoneIndex(inputs.id->name);
	if (!outputs.bone.GetRuntimeData().pBone)
	{
		DEBUG_PRINT("Missing Model %s's bone %s\n", inputs.model->name, inputs.id->name);
	}
}

void InitializeMatrix(Entity e)
{
	ASSERT(e != NULL);

	struct Inputs
	{
		Optional<MatrixComponent, FromParents>      parentMtx;
		Optional<TransformComponent>				tran;
		Optional<ScaleComponent>					scale;
	};

	struct Outputs
	{
		Required<MatrixComponent> worldMtx;
	};

	Inputs inputs;
	Outputs outputs;

	bool bDidSetInputOutputs =
		GetComponent(e, inputs.parentMtx) &&
		GetComponent(e, inputs.tran) &&
		GetComponent(e, inputs.scale) &&
		GetComponent(e, outputs.worldMtx);

	if (!bDidSetInputOutputs)
	{
		return;
	}

	Matrix4x4 mLocal;

	if (inputs.tran.Exists())
	{
		const Component<TransformComponent>& transform = inputs.tran.Force();
		mLocal = transform->rotation;
		mLocal.Translate(transform->position.x, transform->position.y, transform->position.z);
	}
	else
	{
		mLocal.LoadIdentity();
	}

	Matrix4x4 mWorld;

	if (inputs.parentMtx.Exists()) {
		mWorld = mLocal * inputs.parentMtx.Force()->matrix;
	}
	else {
		mWorld = mLocal;
	}

	if (inputs.scale.Exists())
	{
		Matrix4x4 mScale;
		mScale.MakeScale(inputs.scale.Force()->scale);
		mWorld = mScale * mWorld;
	}

	// cache transform info the matrix component
	outputs.worldMtx.Modify().matrix = mWorld;
}

void InitializeScale(Entity e)
{
	ASSERT(e != NULL);

	struct Inputs
	{
		Required<BoneComponent> bone;
		Required<ScaleComponent> scale;
	};

	struct Outputs
	{
		Required<ScaleComponent> scale;
	};

	Inputs inputs;
	Outputs outputs;

	bool bDidSetInputOutputs =
		GetComponent(e, inputs.bone) &&
		GetComponent(e, inputs.scale) &&
		GetComponent(e, outputs.scale);

	if (!bDidSetInputOutputs)
	{
		return;
	}

	outputs.scale.Modify().scale = inputs.bone->m_scale;
}

void SetBoneTransforms(Entity e)
{
	ASSERT(e != NULL);

	struct Inputs
	{
		Required<MatrixComponent, FromSelfOrParents> mtx;
		Required<Identifier> id;
		Required<ActiveDevice, FromSelfOrParents>	device;
		Optional<TurnableComponent> turnable;
		Optional<ModelComponent> model;
	};

	struct Outputs
	{
		Required<BoneComponent>	bone;
	};

	Inputs inputs;
	Outputs outputs;

	bool bDidSetInputOutputs =
		GetComponent(e, inputs.mtx) &&
		GetComponent(e, inputs.id) &&
		GetComponent(e, inputs.turnable) &&
		GetComponent(e, inputs.model) &&
		GetComponent(e, outputs.bone) &&
		GetComponent(e, inputs.device);

	if (!bDidSetInputOutputs)
	{
		return;
	}


	if (!outputs.bone.GetRuntimeData().pBone)
	{
		DEBUG_PRINT("Missing bone %s\n", inputs.id->name);
		ASSERT(false);
	}
	outputs.bone.GetRuntimeData().pBone->SetTransform(inputs.mtx->matrix);
	outputs.bone.GetRuntimeData().pBone->UpdateConstants(inputs.device.GetRuntimeData().pDevice);
}

void AddMatrixToEntity(Entity e)
{
	ASSERT(e != NULL);

	struct RootInputs
	{
		Optional<TransformComponent, FromParentWith<ModelComponent> > transform;
	};

	struct Inputs
	{
		Optional<BoneComponent> bone;
		Optional<Identifier> id;
		Optional<IntermediateBone> intermediate;
		Optional<MatrixComponent> mtx;
		Optional<Billboard> billboard;
	};

	Inputs inputs;

	bool bDidSetInputs =
		GetComponent(e, inputs.bone) &&
		GetComponent(e, inputs.id) &&
		GetComponent(e, inputs.intermediate) &&
		GetComponent(e, inputs.mtx) &&
		GetComponent(e, inputs.billboard);

	if (!bDidSetInputs) { return; }

	bool bIsBone = inputs.bone.Exists() && inputs.id.Exists();
	bool bIsIntermediateBone = inputs.intermediate.Exists();

	if (!(bIsBone || bIsIntermediateBone)) { return; }

	RootInputs rootInput;
	bool bHasRootTransform = GetComponent(e, rootInput.transform) && rootInput.transform.Exists();
	bool bIsBillboard = inputs.billboard.Exists();

	if (bIsBone && !bIsBillboard && !bHasRootTransform) { return; }

	if (inputs.mtx.Exists()) { return; }

	GameComponents<MatrixComponent>::Create(e);
}

void ProcessBoneEntity(Entity e, ComponentLoadHandles& handles, bool bWasPreviouslyCalled)
{
	ASSERT(e != NULL);

	InitializeBones(e, handles, bWasPreviouslyCalled);
	AddScaleToEntity(e);
	InitializeScale(e);
	AddMatrixToEntity(e);
	InitializeMatrix(e);
	SetBoneTransforms(e);
}

void ProcessIntermediateBoneEntity(Entity e, bool bWasPreviouslyCalled)
{
	AddMatrixToEntity(e);
	InitializeMatrix(e);
}

void AddScaleToEntity(Entity e)
{
	ASSERT(e != NULL);

	struct RootInputs
	{
		Optional<ScaleComponent, FromParentWith<ModelComponent> > scale;
	};

	RootInputs rootInput;
	bool bHasRootScale = GetComponent(e, rootInput.scale) && rootInput.scale.Exists();

	if (!bHasRootScale) { return; }

	struct Inputs
	{
		Required<BoneComponent> bone;
		Optional<ScaleComponent> scale;
	};

	Inputs inputs;

	bool bDidSetInputs =
		GetComponent(e, inputs.bone) &&
		GetComponent(e, inputs.scale);

	if (inputs.scale.Exists()) { return; }

	GameComponents<ScaleComponent>::Create(e);
}

}

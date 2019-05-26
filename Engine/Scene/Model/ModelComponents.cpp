#include "Engine/Common/Common.h"
#include "Engine/Scene/Model/ModelComponents.pb.h"
#include "Engine/Framework/ModelMgr.h"
#include "Engine/Scene/Model/Model.h"
#include "Engine/Scene/Model/Bone.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/Scene/Model/ModelAnimPlayer.h"
#include "Engine/Resource/ModelResource.h"
#include "Engine/Framework/FrameworkComponents.pb.h"
#include "Engine/Scene/Model/Model.h"
#include "Engine/Core/String/String_Util.h"

namespace usg
{
	void InitModel(Component<ModelComponent>& p, ComponentLoadHandles& handles)
	{
		auto pModelMgr = handles.pModelMgr;
		p.GetRuntimeData().pModel = pModelMgr->GetModel(handles.pResourceMgr, p->name, p->bDynamic, p->bPerBoneCulling);
		Optional<VisibilityComponent> visibility;
		handles.GetComponent(p.GetEntity(), visibility);
		p.GetRuntimeData().pModel->AddToScene(!visibility.Exists() || visibility.Force()->bVisible);
		if (p->bShadowCast)
		{
			p.GetRuntimeData().pModel->EnableShadow(handles.pDevice, true);
		}
	}


	void ClearModel(Component<ModelComponent>& p, ComponentLoadHandles& handles)
	{
		if (p.GetRuntimeData().pModel != NULL)
		{
			p.GetRuntimeData().pModel->ForceRemoveFromScene();
			handles.pModelMgr->Free(p.GetRuntimeData().pModel);
		}

		p.GetRuntimeData().pModel = NULL;
		p.Modify().name[0] = '\0';
	}


	template<>
	void OnLoaded<ModelComponent>(Component<ModelComponent>& p, ComponentLoadHandles& handles,
		bool bWasPreviouslyCalled)
	{
		bool bIsModelNull = p.GetRuntimeData().pModel == NULL;

		if (bWasPreviouslyCalled && !bIsModelNull)
		{
			return;
		}

		bool bHasModelString = str::StringLength(p.GetData().name) > 0;
		ASSERT(bHasModelString);
		if (bIsModelNull && bHasModelString)
		{
			InitModel(p, handles);
		}

		ASSERT(p.GetRuntimeData().pModel != NULL);
	}

	template<>
	void OnActivate<ModelComponent>(Component<ModelComponent>& p)
	{
		p.GetRuntimeData().pModel = NULL;
	}

	template<>
	void OnDeactivate<ModelComponent>(Component<ModelComponent>& p, ComponentLoadHandles& handles)
	{
		ClearModel(p, handles);
	}

	template<>
	void PreloadComponentAssets<ModelComponent>(const usg::ComponentHeader& hdr, ProtocolBufferFile& file, ComponentLoadHandles& handles)
	{
		ModelComponent component;
		bool readSuccess = file.Read(&component);
		ASSERT(readSuccess);

		handles.pModelMgr->PreloadModel(handles.pResourceMgr, component.name, component.bDynamic, component.bPerBoneCulling, component.uPreloadCount);
	}

	template<>
	void OnLoaded<UVRotation>(Component<UVRotation>& rotation, ComponentLoadHandles& handles, bool bWasPreviouslyCalled)
	{
		if (bWasPreviouslyCalled)
		{
			return;
		}
		UVIdentifier* pId = &rotation.Modify().identifier;
		Entity ent = rotation.GetEntity();
		Required<ModelComponent, FromSelfOrParents> model;
		handles.GetComponent(ent, model);
		ASSERT(model.IsValid());
		pId->uMeshIndex = model.GetRuntimeData().pModel->GetMeshIndex(pId->materialName, Model::IDENTIFIER_MATERIAL);
	}

	template<>
	void OnLoaded<UVTranslation>(Component<UVTranslation>& translation, ComponentLoadHandles& handles, bool bWasPreviouslyCalled)
	{
		if (bWasPreviouslyCalled)
		{
			return;
		}
		UVIdentifier* pId = &translation.Modify().identifier;
		Entity ent = translation.GetEntity();
		Required<ModelComponent, FromSelfOrParents> model;
		handles.GetComponent(ent, model);
		ASSERT(model.IsValid());
		pId->uMeshIndex = model.GetRuntimeData().pModel->GetMeshIndex(pId->materialName, Model::IDENTIFIER_MATERIAL);
	}

	template<>
	void OnActivate<ModelAnimComponent>(Component<ModelAnimComponent>& p)
	{
		p.GetRuntimeData().pAnimPlayer = vnew(usg::ALLOC_GEOMETRY_DATA) usg::ModelAnimPlayer();
	}

	template<>
	void PreloadComponentAssets<ModelAnimComponent>(const usg::ComponentHeader& hdr, ProtocolBufferFile& file, ComponentLoadHandles& handles)
	{
		ModelAnimComponent component;
		bool readSuccess = file.Read(&component);
		ASSERT(readSuccess);
		
		ResourceMgr::Inst()->GetBufferedFile(component.name);
	}

	template<>
	void OnLoaded<ModelAnimComponent>(Component<ModelAnimComponent>& p, ComponentLoadHandles& handles,
	                                         bool bWasPreviouslyCalled)
	{
		// TODO: check bWasPreviouslyCalled ?
		if (p->name[0] != '\0')
		{
			// FIXME: Ordering of these loads is important...
			Required<usg::ModelComponent> model;
			handles.GetComponent(p.GetEntity(), model);

			Required<ActiveDevice, FromSelfOrParents> device;
			bool bDidGetComponents = handles.GetComponent(p.GetEntity(), device);
			ASSERT(bDidGetComponents);

			ModelResHndl pResource = ResourceMgr::Inst()->GetModel(device.GetRuntimeData().pDevice, model.Modify().name, true);
			p.GetRuntimeData().pAnimPlayer->Init(pResource->GetDefaultSkeleton(), p->name, true);
		}
	}

	template<>
	void OnDeactivate<ModelAnimComponent>(Component<ModelAnimComponent>& p, ComponentLoadHandles& handles)
	{
		if (p.GetRuntimeData().pAnimPlayer)
		{
			vdelete p.GetRuntimeData().pAnimPlayer;
			p.GetRuntimeData().pAnimPlayer = NULL;
		}
	}
}
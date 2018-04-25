/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Scene/Model/Model.h"
#include "Engine/Resource/ModelResource.h"
#include "Engine/Core/String/U8String.h"
#include "Engine/Resource/ResourceMgr.h"
#include "ModelMgr.h"

namespace usg
{
	ModelMgr::ModelMgr(void)
	{
		m_bAutoTransform = false;
	}


	ModelMgr::~ModelMgr(void)
	{
	}


	Model* ModelMgr::GetModel(const char* szModelName, bool bDynamic, bool bPerBoneCulling)
	{
		U8String cmpName = ResourceMgr::Inst()->GetModelDir() + szModelName;
		Model* pModel = GetFreeInstance(cmpName);

		if(!pModel)	// Handle creating a new instance
		{
			// We need a new model
			pModel = m_pool.Alloc();
			if(pModel)
			{
				pModel->Load(m_pDevice, m_pScene, szModelName, bDynamic, true, m_bAutoTransform, bPerBoneCulling);
			}
			m_inUseList.AddToEnd(pModel);
		}
		else
		{
			pModel->SetDynamic(m_pDevice, bDynamic);
			pModel->SetFade(false);
			pModel->RemoveOverrides(m_pDevice);
			pModel->SetInUse(true);
		}

		return pModel;
	}

	void ModelMgr::Free(Model* pModel)
	{
		Inherited::Free(pModel);
		if (pModel)
			pModel->SetInUse(false);
	}

	void ModelMgr::PreloadModel(const char* szModelName, bool bDynamic, bool bPerBoneCulling, uint32 uCount)
	{
		List<Model> model;
		// First load the models
		for(uint32 i=0; i<uCount; i++)
		{
			model.AddToEnd( GetModel(szModelName, bDynamic, bPerBoneCulling) );
		}

		// Now add them to the free list
		for (List<Model>::Iterator it = model.Begin(); !it.IsEnd(); ++it)
		{
			Free( (*it));
		}
	}



}
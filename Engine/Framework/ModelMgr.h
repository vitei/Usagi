/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: An optional system for managing allocated and freed models
*****************************************************************************/
#ifndef _USG_FRAMEWORK_MODEL_MGR_
#define _USG_FRAMEWORK_MODEL_MGR_
#include "Engine/Common/Common.h"
#include "Engine/Core/Containers/List.h"
#include "Engine/Scene/Model/Model.h"
#include "InstanceMgr.h"

namespace usg
{

	class Model;
	class Scene;

	class ModelMgr : public InstanceMgr<Model>
	{
		typedef InstanceMgr<Model> Inherited;
	public:
		ModelMgr();
		~ModelMgr();

		Model* GetModel(const char* szName, bool bDynamic, bool bPerBoneCulling);
		// Set to false for component systems where you are manually managing bone hierarchies
		void SetAutoTransform(bool bAuto) { m_bAutoTransform = bAuto; }
		void PreloadModel(const char* szModelName, bool bDynamic, bool bPerBoneCulling, uint32 uCount);

		virtual void Free(Model* pInstance);
	private:
		bool						m_bAutoTransform;
	};


}

#endif



/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Scene/SceneSearchObject.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/SceneContext.h"

namespace usg {

void SceneSearchFrustum::Callback(void* pUserData)
{
	RenderGroup* pComponent = (RenderGroup*)pUserData;
	m_pScene->UpdateObject(pComponent);
	m_pContext->AddToDrawList(pComponent);
}

void SceneSearchSphere::Callback(void* pUserData)
{
	RenderGroup* pComponent = (RenderGroup*)pUserData;
	m_pScene->UpdateObject(pComponent);
	m_pContext->AddToDrawList(pComponent);
}

}




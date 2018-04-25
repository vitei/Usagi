/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Scene/Scene.h"
#include "TransformNode.h"

namespace usg
{

	TransformNode::TransformNode()
	{
		m_worldMat.LoadIdentity();
	}

	void TransformNode::SetMatrix(const Matrix4x4 &mat)
	{
		m_worldMat = mat;
	}


}


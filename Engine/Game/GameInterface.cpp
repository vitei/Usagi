/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Core/Utility.h"
#include "GameInterface.h"

namespace usg {

GameInterface::GameInterface()
{
	m_bIsRunning = false;
	m_bReqReset = false;
}

GameInterface::~GameInterface()
{
//	utl::SafeDelete( &m_pInput );
}


}


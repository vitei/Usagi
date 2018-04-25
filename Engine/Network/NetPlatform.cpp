/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Network/NetPlatform.h"
#include "Engine/Core/Containers/List.h"
#include "Engine/Common/Common.h"

#include OS_HEADER(Engine/Core/Timer, TimeTracker.h)

namespace usg{

double net_get_time()
{
	return (double)TimeTracker::GetSystemTime() / (double)TimeTracker::GetTickFrequency();
}

sint16 net_get_endian_value()
{
	return 1;
}

}

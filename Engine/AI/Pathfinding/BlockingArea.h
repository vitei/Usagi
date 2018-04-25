/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Assign unique id to a shape
*****************************************************************************/

#ifndef __USG_AI_BLOCKINGAREA__
#define __USG_AI_BLOCKINGAREA__

#include "Shape.h"

namespace usg
{
	namespace ai
	{
		struct BlockingArea
		{
			OBB shape;
			uint32 uUID;
		};
	}
}

#endif

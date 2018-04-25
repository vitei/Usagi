#include "Engine/Common/Common.h"
#include "Engine/Framework/SystemId.h"

namespace usg
{
	namespace details
	{
		uint32 GenerateSystemId()
		{
			static uint32 uNextSystemID = 0;
			return uNextSystemID++;
		}
	}
}
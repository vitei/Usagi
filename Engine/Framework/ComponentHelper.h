#pragma once

#include "Engine/Common/Common.h"

namespace usg
{
	class ComponentEntity;
	class GFXDevice;
	class ProtocolBufferFile;

	typedef struct _ComponentHeader ComponentHeader;

	struct ComponentHelper
	{
		void(*Init)();
		void(*CallOnLoaded)(ComponentEntity* e, ComponentLoadHandles& handles);
		void(*PreloadComponentAssets)(const ComponentHeader& hdr, ProtocolBufferFile& file, ComponentLoadHandles& handles);
		void(*LoadAndAttachComponent)(ProtocolBufferFile& file, ComponentEntity* pEntity);
	};
}

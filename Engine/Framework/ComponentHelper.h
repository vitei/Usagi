#pragma once
#include "Engine/Core/stl/set.h"


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
		void(*PreloadComponentAssets)(const ComponentHeader& hdr, ProtocolBufferFile& file, ComponentLoadHandles& handles, usg::set<usg::string>& referencedEntities);
		void(*LoadAndAttachComponent)(ProtocolBufferFile& file, ComponentEntity* pEntity);
	};
}

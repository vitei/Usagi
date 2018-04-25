/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: A header for maintaining a list of resource paks we have
//	loaded so we don't try and reload (fixme: should probably also maintain
//	the headers when we support more dynamic loading)
*****************************************************************************/
#ifndef _USG_RESOURCE_PAK_HEADER_H_
#define _USG_RESOURCE_PAK_HEADER_H_
#include "Engine/Common/Common.h"
#include "Engine/Resource/ResourceBase.h"
#include "Engine/Resource/ResourcePakLoader.h"

namespace usg {

class ResourcePakHdr : public ResourceBase
{
public:
	ResourcePakHdr() { }
	virtual ~ResourcePakHdr() {}

	bool Init(const ResourcePakLoader& loader)
	{
		// TODO: Copy headers
		SetupHash(loader.GetName().CStr());
		return true;
	}

private:

};

}

#endif

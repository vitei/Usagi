/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef __ENTITYREF_H
#define __ENTITYREF_H

#include "Engine/Framework/ComponentEntity.h"

namespace usg {

struct EntityRef
{
	// The union is a hack so we can use this header to define a protocol
	// buffer message. The member entityID should ONLY be used when
	// loading. During loading we use this member to look up the entity, so
	// after loading completes ONLY entity should be used.
	union {
		uint64 entityID;
		Entity entity;
	};

	EntityRef()         : entity(NULL) {}
	EntityRef(Entity e) : entity(e) {}
	EntityRef& operator=(Entity e) { entity = e; return *this; }

	      Entity& operator*()       { return entity; }
	const Entity& operator*() const { return entity; }
	bool operator==(const Entity o) const { return entity == o; }
	bool operator!=(const Entity o) const { return entity != o; }
	operator Entity() const { return entity; }
};

}

#endif //__ENTITYREF_H

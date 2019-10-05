/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _COMPONENT_H
#define _COMPONENT_H


#include "ComponentEntity.h"
#include "HierarchyNode.h"

#include <Engine/Core/ProtocolBuffers.h>

namespace usg {

// Where to search for components.  OR these together to check both.
// FROM_PARENTS searches all parents up the hierarchy until the root.
static const uint32 FROM_SELF  = 1;
static const uint32 FROM_PARENTS = 2;
struct ComponentLoadHandles;

// Annoying that I have to do this but it makes the boilerplate tool easier.
struct FromSelf          { static const int value = FROM_SELF; };
struct FromParents       { static const int value = FROM_PARENTS; };
struct FromSelfOrParents { static const int value = FROM_SELF | FROM_PARENTS; };
template<typename Component>
struct FromParentWith    {};

typedef uint32 ModifyId;

// RuntimeData allows you to specify some data which is initialised in OnActivate/OnLoaded
// and exists alongside the component at runtime.  Simply specialise this template with
// whatever data you need.
template<typename T> struct RuntimeData {};

class ComponentType
{
public:
	ComponentType(uint32 typeID)
#ifdef DEBUG_BUILD
		: 
		m_uTypeID(typeID)
#else
		: m_uTypeID(typeID)
#endif
		, m_pPrevComponent(NULL)
		, m_pNextComponent(NULL)
		, m_uEntity(0)
		, m_modifyId(0)
		, m_bDidRunOnLoaded(false)
		, m_bFreeRequested(false)
	{}
	~ComponentType() {}

	uint32 GetTypeID() const { return m_uTypeID; }

	ComponentType* GetNextComponent() const;
	void           SetNextComponent(ComponentType* pComp);
	ComponentType* GetPrevComponent() const;
	void           SetPrevComponent(ComponentType* pComp);

	bool     WasLoaded() const { return m_bDidRunOnLoaded; }
	void     SetLoaded() { m_bDidRunOnLoaded = true; }
	bool	 WaitingOnFree() { return m_bFreeRequested; }
	void	 RequestFree();

	void     Init()
	{
		m_uEntity = 0;
		m_bDidRunOnLoaded = false;
		m_bFreeRequested = false;
	}

	virtual void Activate(Entity e);

	virtual void Free(ComponentLoadHandles& handles, bool bRemoveFromEntity = true) = 0;

	ModifyId GetModifyId() const
	{
		return m_modifyId;
	}

	Entity   GetEntity() const { return m_uEntity; }

private:
	const uint32      m_uTypeID;

protected:
	static uint32 GetNextTypeID();

	ComponentType* m_pPrevComponent;
	ComponentType* m_pNextComponent;

	Entity         m_uEntity;
	ModifyId       m_modifyId;
	bool           m_bDidRunOnLoaded;
	bool		   m_bFreeRequested;

};

template<typename T>
class Component : public ComponentType
{
public:
	Component() : ComponentType(Component<T>::GetTypeID()) {}
	~Component() {}

	void     Activate(Entity e);
	bool     IsActive() const;
	void     Deactivate(ComponentLoadHandles& handles, bool bRemoveFromEntity = true);
	virtual void Free(ComponentLoadHandles& handles, bool bRemoveFromEntity = true);
	void     Update() { m_uEntity->SetChanged(); }

	RuntimeData<T>& GetRuntimeData() { return m_runtimeData; }
	const RuntimeData<T>& GetRuntimeData() const { return m_runtimeData; }

	const T& operator*() const { return m_data; }
	const T* operator->() const { return &m_data; }

	T& Modify() { m_modifyId++; return m_data; }

	DEPRECATED("GetData() is no longer supported.  Use the dereference operators/Modify() instead.",
		T& GetData())
	{
		return m_data;
	}

	DEPRECATED("GetData() is no longer supported.  Use the dereference operators/Modify() instead.",
		const T& GetData() const)
	{
		return m_data;
	}

	static uint32 GetTypeID()
	{
		static const uint32 uTypeID = GetNextTypeID();
		return uTypeID;
	}

private:
	PRIVATIZE_COPY(Component)

		T              m_data;
	RuntimeData<T> m_runtimeData;
};

class ComponentWrapperBase
{
	friend bool GetComponent(Entity e, uint32 uComponentType, sint32 iSearchMask, ComponentWrapperBase& wrapper);
protected:
	ComponentType* m_pComponent;
	mutable ModifyId m_modifyAtRead;

	ComponentWrapperBase(ComponentType* pComponent, ModifyId modifyAtRead) : m_pComponent(pComponent), m_modifyAtRead(modifyAtRead)
	{

	}
};

template<typename T, typename SearchMask = FromSelf>
class OptionalWrapper : public ComponentWrapperBase
{
public:
	OptionalWrapper()                : ComponentWrapperBase(nullptr, USG_INVALID_ID) {}
	OptionalWrapper(Component<T>* c) : ComponentWrapperBase(c, USG_INVALID_ID) {}

	// Optional components are valid whether or not they exist
	bool IsValid() const { return true; }
	bool Exists() const  { return m_pComponent != NULL; }
	bool Dirty() const { return Exists() && m_modifyAtRead != m_pComponent->GetModifyId(); };

	// Safe component access (pass it a callback/functor to modify the component)
	// This will be a lot more convenient to use with C++ 11 lambdas
	void ifExists( void(*cb)(const Component<T>&) )    { if(Exists()) { cb(this->Force()); } }
	template<typename CB> void ifExists( CB cb ) { if(Exists()) { cb(this->Force()); } }

	// Unsafe component access.  Always check the component Exists() before forcing the value!
	const Component<T>& Force() const { return *((Component<T>*)m_pComponent); }
};

template<typename T, typename SearchMask = FromSelf>
class Optional : public OptionalWrapper<T>
{
public:
	Optional()                : OptionalWrapper<T>() {}
	Optional(Component<T>* c) : OptionalWrapper<T>(c) {}
};

template<typename T>
class Optional<T, FromSelf> : public OptionalWrapper<T>
{
public:
	Optional()                : OptionalWrapper<T>() {}
	Optional(Component<T>* c) : OptionalWrapper<T>(c) {}

	void ifExists( void(*cb)(Component<T>&) )    { if(this->Exists()) { cb(this->Force()); } }
	template<typename CB> void ifExists( CB cb ) { if(this->Exists()) { cb(this->Force()); } }

	const Component<T>& Force() const { ASSERT(this->Exists()); return *((Component<T>*)this->m_pComponent); }
	      Component<T>& Force()       { ASSERT(this->Exists()); return *((Component<T>*)this->m_pComponent); }

	void  Free() { ASSERT(this->Exists()); if(this->Exists()) { this->m_pComponent->Free();  this->m_pComponent = NULL; } }
};

template<typename T>
class RequiredWrapper : public ComponentWrapperBase
{
public:
	RequiredWrapper()                : ComponentWrapperBase(nullptr,USG_INVALID_ID) {}
	RequiredWrapper(Component<T>& c) : ComponentWrapperBase(&c,USG_INVALID_ID) {}

	const T& operator*()  const { ASSERT(m_pComponent != nullptr); return **((Component<T>*)m_pComponent); }
	const T* operator->() const { ASSERT(m_pComponent != nullptr); return &**((Component<T>*)m_pComponent); }

	// Required components are only valid if they exist
	bool IsValid() const { return ((Component<T>*)m_pComponent) != NULL; }
	Entity GetEntity() const { return ((Component<T>*)m_pComponent)->GetEntity(); }
	const RuntimeData<T>& GetRuntimeData() const { return ((Component<T>*)m_pComponent)->GetRuntimeData(); }
	bool Dirty() const
	{
		// Check if we're dirty and also update the modifiy id
		bool bDirty = m_modifyAtRead != ((Component<T>*)m_pComponent)->GetModifyId();
		m_modifyAtRead = ((Component<T>*)m_pComponent)->GetModifyId();
		return bDirty;
	};

	void Deactivate()       { ((Component<T>*)m_pComponent)->Deactivate(); }
	bool IsActive() const { return ((Component<T>*)m_pComponent)->IsActive(); }
};

template<typename T, typename SearchMask = FromSelf>
class Required : public RequiredWrapper<T>
{
public:
	Required()                : RequiredWrapper<T>() {}
	Required(Component<T>& c) : RequiredWrapper<T>(c) {}
};

template<typename T>
class Required<T, FromSelf> : public RequiredWrapper<T>
{
public:
	Required()                : RequiredWrapper<T>() {}
	Required(Component<T>& c) : RequiredWrapper<T>(c) {}

	T& Modify() { return ((Component<T>*)this->m_pComponent)->Modify(); }
	const RuntimeData<T>& GetRuntimeData() const { return ((Component<T>*)this->m_pComponent)->GetRuntimeData(); }
	// TODO: Maybe the const version should be modify runtime data?
	      RuntimeData<T>& GetRuntimeData()       { return ((Component<T>*)this->m_pComponent)->GetRuntimeData(); }
};

// Global functions
// These functions are provided to give easy access to a Component's methods
// through C++'s template type inference.
template<typename T>
void FreeComponent(Component<T>* t)
{
	t->Free();
}

template<typename T>
void InvalidateComponent(Component<T>* t)
{
	t->Invalidate();
}

}

namespace Systems {
// Expose these symbols in the global Systems namespace so we can use them
// easily in system definitions
using usg::FROM_SELF;
using usg::FROM_PARENTS;
using usg::FromSelf;
using usg::FromParents;
using usg::FromSelfOrParents;
using usg::FromParentWith;
}

#endif //_COMPONENT_H

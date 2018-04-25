/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
// Signals
// There is some magic going on here, so I'm going to make this file
// quite comment-heavy in an attempt to explain how it works.
// For further resources check the wiki.

#ifndef USAGI_FRAMEWORK_SIGNAL_H_
#define USAGI_FRAMEWORK_SIGNAL_H_

#include "Engine/Framework/SystemId.h"

namespace usg
{

// Signals can be triggered on the entity itself, on its parents, or
// on its children.
static const uint32 ON_ENTITY   = 0x1;
static const uint32 ON_PARENTS  = 0x2;
static const uint32 ON_CHILDREN = 0x4;

struct GenericInputOutputs;
class ComponentEntity;
using Entity = ComponentEntity*;

struct Signal
{
	Signal(uint32 uId) : uId(uId) { }
	const uint32 uId;
protected:
	struct SignalClosure
	{
		virtual void operator()(const Entity e, const void* inputs, void* outputs) = 0;
	};

	static GenericInputOutputs* GetRootSystem(const uint32 uSystemId);

	static void TriggerSignalFromRoot(GenericInputOutputs* root, SignalClosure& Signal);
	static void TriggerSignalOnEntity(Entity e, SignalClosure& Signal, uint32 systemID, uint32 targets = ON_ENTITY);

private:
	static void TriggerSignalOnChildEntities(Entity e, SignalClosure& Signal, uint32 systemID);
	static void TriggerSystemOnTargets(GenericInputOutputs* pSystem, SignalClosure& Signal, uint32 targets);
	static void TriggerSystemOnParents(GenericInputOutputs* pSystem, SignalClosure& Signal);
	static void TriggerSystemOnEntityAndChildren(GenericInputOutputs* pSystem, SignalClosure& Signal);
};

struct SignalRunner
{
	static const uint32 INVALID_SYSTEM_ID = 0xffffffff;
	SignalRunner() : systemID(INVALID_SYSTEM_ID), priority(0), Trigger(NULL), TriggerFromRoot(NULL) {}
	uint32 systemID;
	sint32 priority;
	void* userData; // Store whatever you want into this in FillSignalRunner
	void (*Trigger)(Entity e, void* signal, const uint32 uSystemId, uint32 targets, void* userData);
	void (*TriggerFromRoot)(Entity e, void* signal, const uint32 uSystemId, void* userData);
};

struct RunSignal : public Signal
{
	static constexpr uint32 ID = 0x78941da;

	float dt;
	RunSignal(float _dt) : Signal(ID), dt(_dt) {}

	template<typename System>
	static inline bool FillSignalRunner(SignalRunner& runner, uint32 systemID)
	{
		runner.systemID = systemID;
		runner.priority = (sint32)System::CATEGORY;
		runner.userData = (void*) &System::Run;
		runner.Trigger = Trigger;
		runner.TriggerFromRoot = TriggerFromRoot;
		return true;
	}

	static void Trigger(Entity e, void* signal, const uint32 uSystemId, uint32 targets, void* userData);
	static void TriggerFromRoot(Entity e, void* signal, const uint32 uSystemId, void* userData);

	struct RunClosure;
};

// The following two macros hide away a lot of the boilerplate above...
#define SIGNAL_RESPONDER(SIGNAL)\
	template<typename System> \
	static inline bool FillSignalRunner(SignalRunner& runner, uint32 systemID) { \
		runner.systemID = systemID; \
		runner.priority = (sint32)System::CATEGORY; \
		runner.Trigger = Trigger<System>; \
		runner.TriggerFromRoot = TriggerFromRoot<System>; \
		return true; } \
	template<typename System> \
	static void Trigger(Entity e, void* signal, const uint32 uSystemId, uint32 targets, void* userData) { \
		if(GetRootSystem(uSystemId) != nullptr) { \
			SIGNAL##Closure<System> closure((SIGNAL##Signal*)signal); \
			const uint32 systemID = GetSystemId<System>(); \
			TriggerSignalOnEntity(e, closure, systemID, targets); } } \
	template<typename System> \
	static void TriggerFromRoot(Entity e, void* signal, const uint32 uSystemId, void* userData) { \
		if(GetRootSystem(uSystemId) != nullptr) { \
			SIGNAL##Closure<System> closure((SIGNAL##Signal*)signal); \
			TriggerSignalFromRoot(GetRootSystem(uSystemId), closure); } }

struct LateUpdateSignal : public Signal
{
	static constexpr uint32 ID = 0x78941ba;

	float dt;
	LateUpdateSignal(float _dt) : Signal(ID), dt(_dt) {}

	template<typename System>
	static inline bool FillSignalRunner(SignalRunner& runner, uint32 systemID)
	{
		runner.systemID = systemID;
		runner.priority = (sint32)System::CATEGORY;
		runner.userData = (void*)&System::LateUpdate;
		runner.Trigger = Trigger;
		runner.TriggerFromRoot = TriggerFromRoot;
		return true;
	}

	static void Trigger(Entity e, void* signal, const uint32 uSystemId, uint32 targets, void* userData);
	static void TriggerFromRoot(Entity e, void* signal, const uint32 uSystemId, void* userData);

	struct LateUpdateClosure;
};

}

// Expose the signal target bitfield values globally
using usg::ON_ENTITY;
using usg::ON_PARENTS;
using usg::ON_CHILDREN;

#endif //USAGI_FRAMEWORK_SIGNAL_H_

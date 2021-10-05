/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
// System keys
// Allows you to define, statically at compile time, a set of "keys"
// for filtering systems.  For a good explanation of the concept,
// see this answer on StackOverflow:
//     http://gamedev.stackexchange.com/a/31491

// The basic idea is to define a bitfield of REQUIRED components
// for a particular system -- if an entity doesn't have these components,
// it won't even bother to run GetInputOutputs() to check whether
// it needs to run.
// OPTIONAL components -- those which might be NULL, should
// not be included in the list of required components.

#ifndef __SYSTEMKEY_H
#define __SYSTEMKEY_H


namespace usg {

static const unsigned int BITFIELD_LENGTH = 32;
template<typename SYSTEM> uint32 GetSystemKey(uint32 uOffset);

}

#endif //__SYSTEMKEY_H


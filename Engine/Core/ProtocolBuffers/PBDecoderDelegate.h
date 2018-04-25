/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Parent class for decoder delegates.
*****************************************************************************/
#ifndef _USG_CORE_PB_DECODER_DELEGATE_H_
#define _USG_CORE_PB_DECODER_DELEGATE_H_

#include "Engine/Common/Common.h"

namespace usg {

template <typename T>
class PBDecoderDelegate
{
public:
	typedef void Iterator;
	PBDecoderDelegate<T>() : InitializeFieldCB(NULL) {}
	void (*InitializeFieldCB)(T* field_to_initialize);
	void InitializeField(T* field_to_init)
	{
		if(InitializeFieldCB)
			InitializeFieldCB(field_to_init);
	}
};

}

#endif //_USG_CORE_PB_DECODER_DELEGATE_H_


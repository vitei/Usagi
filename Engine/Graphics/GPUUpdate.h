/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _GPUUPDATE_H
#define _GPUUPDATE_H
#include "Engine/Framework/Signal.h"

namespace usg {

struct GPUHandles
{
	GFXDevice* pDevice;
};

struct GPUUpdateSignal : public Signal
{
	static constexpr uint32 ID = 0x12941da;

	GPUHandles* handles;
	GPUUpdateSignal(GPUHandles* _handles) : Signal(ID), handles(_handles) {}

	SIGNAL_RESPONDER(GPUUpdate)

	template<typename System>
	struct GPUUpdateClosure : public SignalClosure
	{
		GPUUpdateSignal* signal;
		GPUUpdateClosure(GPUUpdateSignal* sig) : signal(sig) {}
		virtual void operator()(const Entity e, const void* in, void* out)
		{
			System::GPUUpdate(*(const typename System::Inputs*)in, *(typename System::Outputs*)out, signal->handles);
		}
	};
};

}

#endif //_GPUUPDATE_H


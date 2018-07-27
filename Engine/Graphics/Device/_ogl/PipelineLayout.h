/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_GRAPHICS_DEVICE_PC_PIPELINELAYOUT_H_
#define _USG_GRAPHICS_DEVICE_PC_PIPELINELAYOUT_H_
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Device/RenderState.h"
#include "Engine/Graphics/Device/PipelineLayoutBase.h"

namespace usg {

class PipelineLayout : public PipelineLayoutBase
{
public:
	PipelineLayout() {}
	~PipelineLayout() {}
	
	void Init(GFXDevice* pDevice, const PipelineLayoutDecl &decl, uint32 uId) { InitBase(decl); }
private:
};

}


#endif
/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_GRAPHICS_DEVICE_PIPELINELAYOUT_BASE_H_
#define _USG_GRAPHICS_DEVICE_PIPELINELAYOUT_BASE_H_

#include "Engine/Graphics/Device/RenderState.h"

namespace usg {

#ifdef DEBUG_BUILD
#define CONFIRM_PIPELINE_VALIDITY 1
#else
#define CONFIRM_PIPELINE_VALIDITY 0
#endif


class PipelineLayoutBase
{
public:
	PipelineLayoutBase() {}
	~PipelineLayoutBase() {}
	
	void InitBase(const PipelineLayoutDecl &decl)
	{
#if CONFIRM_PIPELINE_VALIDITY
		m_decl = decl;
#endif
	}

#if CONFIRM_PIPELINE_VALIDITY
	const PipelineLayoutDecl& GetDecl() { return m_decl; }
#endif
private:
#if CONFIRM_PIPELINE_VALIDITY
	PipelineLayoutDecl m_decl;
#endif
};

}


#endif
/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Interface for creating the various render states
//  FIXME: Move all of this into the graphics render context
*****************************************************************************/
#ifndef _USG_GRAPHICS_DEVICE_VULKAN_PIPELINE_STATE_PS_H_
#define _USG_GRAPHICS_DEVICE_VULKAN_PIPELINE_STATE_PS_H_
#include "Engine/Common/Common.h"
#include <vulkan/vulkan.h>

namespace usg
{

	class GFXDevice;

	class PipelineState_ps
	{
	public:
		PipelineState_ps();
		~PipelineState_ps();

		// Set up the defaults
		void Init(GFXDevice* pDevice, const struct PipelineInitData& decl);
		VkPipeline GetPipeline() const { return m_pipeline; }
		VkPipelineLayout GetLayout() const { return m_layout; }

	private:
		VkPipeline m_pipeline;
		VkPipelineLayout m_layout;
	};

}


#endif


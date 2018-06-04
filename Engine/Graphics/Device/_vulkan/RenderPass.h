/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_GRAPHICS_DEVICE_PC_RENDERPASS_H_
#define _USG_GRAPHICS_DEVICE_PC_RENDERPASS_H_
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Device/RenderState.h"
#include "Engine/Core/stl/vector.h"
#include OS_HEADER(Engine/Graphics/Device, VulkanIncludes.h)

namespace usg {

class RenderPass
{
public:
	RenderPass() {};
	~RenderPass() {};
	
	void Init(GFXDevice* pDevice, const class RenderPassInitData &decl, uint32 uId);
	const VkRenderPass& GetPass() const { return m_renderPass; }
	uint32 GetCRC() const { return m_uCRCForPass; }
	uint32 GetClearCount(uint32 uSubPass) const { return m_passClearData[uSubPass].uClearCount; }
	const uint32* GetClearIndices(uint32 uSubPass) const { return m_passClearData[uSubPass].uClearIndices; }
	bool ClearDepth(uint32 uSubPass) const { return m_passClearData[uSubPass].bClearDepth; }

private:
	// TODO: Would need multiple of these if we had subpasses
	struct PassClearData
	{
		uint32		 uClearCount;
		uint32		 uClearIndices[MAX_COLOR_TARGETS];
		bool		 bClearDepth;
	};
	
	vector<PassClearData> m_passClearData;
	VkRenderPass m_renderPass;
	uint32		 m_uCRCForPass;
};

}


#endif
/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2016
//	Description: Platform specific debug information rendering
*****************************************************************************/
#pragma once

#ifndef USG_DEBUG_STATS_VULKAN_H
#define USG_DEBUG_STATS_VULKAN_H
#include "Engine/Common/Common.h"
#include "Engine/Debug/Rendering/DebugRender.h"

namespace usg {

class DebugRender;

class DebugStats_ps
{
public:
	DebugStats_ps() {}
	~DebugStats_ps() {}

	void Init(GFXDevice* pDevice) {}
	void Update(float fElapsed) {}

	void Draw(DebugRender* pRender) {}
	void PreDraw(GFXDevice* pDevice) {}
	void PostDraw(GFXDevice* pDevice) {}


	uint32 GetPageCount() const { return 0; }
	void SetPage(uint32 uPage) { ASSERT(false); }
	void SetActive(bool bActive) { ASSERT(bActive==false); };

private:
};

} // namespace usagi

#endif // USG_DEBUG_STATS_VULKAN_H

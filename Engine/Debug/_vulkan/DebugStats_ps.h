/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2016
//	Description: Platform specific debug information rendering
*****************************************************************************/
#pragma once

#ifndef USG_DEBUG_STATS_VULKAN_H
#define USG_DEBUG_STATS_VULKAN_H

#include "Engine/Debug/Rendering/IDebugStatGroup.h"

namespace usg {

class DebugRender;

class DebugStats_ps : public IDebugStatGroup
{
public:
	DebugStats_ps() : IDebugStatGroup() {}
	~DebugStats_ps() {}

	void Init(GFXDevice* pDevice) {}
	void Update(float fElapsed) override {}

	void Draw(DebugRender* pRender) override {}
	void PreDraw(GFXDevice* pDevice) override {}
	void PostDraw(GFXDevice* pDevice) override {}


	uint32 GetPageCount() const { return 0; }

private:
};

} // namespace usagi

#endif // USG_DEBUG_STATS_VULKAN_H

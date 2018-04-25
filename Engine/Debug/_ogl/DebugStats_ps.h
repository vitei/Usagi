/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Platform specific debug information rendering
*****************************************************************************/
#ifndef __USG_DEBUG_STATS_PC_H__
#define __USG_DEBUG_STATS_PC_H__
#include "Engine/Common/Common.h"
#include "Engine/Debug/Rendering/IDebugStatGroup.h"

namespace usg {


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

	virtual uint32 GetPageCount() const { return 0; }


private:
};

}

#endif

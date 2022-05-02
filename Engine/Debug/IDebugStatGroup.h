/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Platform independent debug rendering code
*****************************************************************************/
#pragma once
#ifndef USG_DEBUG_STAT_GROUP_H
#define USG_DEBUG_STAT_GROUP_H

#ifndef FINAL_BUILD

namespace usg {

class DebugRender;
class GFXDevice;
class DebugStats;
class string;

class IDebugStatGroup
{
public:
	IDebugStatGroup() :  m_uActivePage(0), m_pOwner(nullptr) {}
	virtual ~IDebugStatGroup();

	virtual void Init(DebugStats* pOwner) { m_pOwner = pOwner; }
	virtual void Update(float fElapsed) = 0;

	virtual void Draw(DebugRender* pRender) = 0;
	virtual void PreDraw(GFXDevice* pDevice) = 0;
	virtual void PostDraw(GFXDevice* pDevice) = 0;

	void ClearOwner() { m_pOwner = nullptr; }
	virtual uint32 GetPageCount() const = 0;
	virtual void SetPage(uint32 uPage) { m_uActivePage = uPage; }
	virtual void AppendWarnings(usg::string& string) {}

	void SetActive(bool bActive) { }

protected:
	struct Warning
	{
		float	fTimeSinceActive;
		bool	bActive;
	};
	
	uint32		m_uActivePage;
	DebugStats*	m_pOwner;
private:
};

}

#endif

#endif // USG_DEBUG_STAT_GROUP_H

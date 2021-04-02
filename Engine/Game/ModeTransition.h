/****************************************************************************
//	Filename: ModeTransition.h
//	Description: We need to flush the GFXContext so it's not using any deleted data
//	before we move to the next mode. Simply clearing the screen presents a platform
//	independent method of accomplishing this
*****************************************************************************/
#ifndef USAGI_MODE_TRANSITION_H__
#define USAGI_MODE_TRANSITION_H__

#include "Mode.h"

namespace usg
{

	class ModeTransition : public Mode
	{
		typedef usg::Mode Inherited;
	public:
		ModeTransition();
		virtual ~ModeTransition();

		virtual void Init(usg::GFXDevice* pDevice, usg::ResourceMgr* pResMgr) override;
		virtual void Cleanup(usg::GFXDevice* pDevice) override {};
		virtual void Reset() { m_uActiveFrames = 0; }
		virtual bool Update(float fElapsed) override;
		virtual void PreDraw(usg::GFXDevice* pDevice, usg::GFXContext* pImmContext) override;
		virtual void Draw(usg::Display* pDisplay, usg::IHeadMountedDisplay* pHMD, usg::GFXContext* pImmContext) override;
		virtual void PostDraw(usg::GFXDevice* pDevice) override;
		virtual bool FinalTargetIsDisplay() const { return true; }

		virtual void SetNextModeReady(bool bReady) {}
		virtual bool ShouldHold() const { return false; }

	private:
		uint32 m_uActiveFrames;
	};

}

#endif 

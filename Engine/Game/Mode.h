/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: The base class for a mode with no functionallity or even
//  assumptions that we will be using the component systems
*****************************************************************************/
#ifndef __CLR_USAGI_MODE_H__
#define __CLR_USAGI_MODE_H__



namespace usg
{
	class GFXContext;
	class GFXDevice;
	class Display;
	class IHeadMountedDisplay;
	class UsagiNet;

	class Mode
	{
	public:
		Mode() {}
		virtual ~Mode() {}

		virtual void Init(GFXDevice* pDevice, ResourceMgr* pResMgr) = 0;
		virtual void Start() {}
		virtual void Cleanup(GFXDevice* pDevice) = 0;
		virtual bool Update(float fElapsed) = 0;

		virtual void PreDraw(GFXDevice* pDevice, GFXContext* pImmContext) {};
		virtual void Draw(Display* pDisplay, IHeadMountedDisplay* pHMD, GFXContext* pImmContext) = 0;
		virtual void PostDraw(GFXDevice* pDevice) {};

		virtual bool IsNetworked() { return false; }
		virtual void InitNetworking(UsagiNet& usagiNetwork) { ASSERT(false); }
		virtual bool AllowActivation() { return true; }
		virtual bool FinalTargetIsDisplay() const { return false; }

		virtual void NotifyResize(GFXDevice* pDevice, uint32 uDisplay, uint32 uWidth, uint32 uHeight) {}
	
	};
}

#endif 

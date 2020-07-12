/****************************************************************************
//	Filename: ModeInitThread.h
//	Description: Responsible for loading a new mode in the background
*****************************************************************************/
#ifndef __CLR_USAGI_MODE_INIT_THREAD_H__
#define __CLR_USAGI_MODE_INIT_THREAD_H__

#include "Engine/Core/Thread/Thread.h"
#include "Engine/Game/Mode.h"

namespace usg
{
	class GFXDevice;

	typedef bool(*ModeLoadFunc)(uint32 uMode, usg::Mode** ppLoadMode);

	class InitThread : public usg::Thread
	{
	public:
		InitThread();
		virtual ~InitThread();

		void Init(usg::GFXDevice* pDevice, ModeLoadFunc fnLoad);
		void SetNextMode(usg::Mode** ppLoadMode, uint32 uNextMode);
		virtual void Exec();
		void Run();

	private:
		usg::GFXDevice* m_pDevice;
		usg::Mode**		m_ppLoadMode;
		uint32			m_uNextMode;
		ModeLoadFunc	m_fnLoad;
	};
}

#endif 

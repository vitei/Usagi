#include "Engine/Common/Common.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "ModeTransition.h"
#include "Engine/Graphics/Device/GFXContext.h"

namespace usg
{

	ModeTransition::ModeTransition()
	{

	}

	ModeTransition::~ModeTransition()
	{

	}

	void ModeTransition::Init(GFXDevice* pDevice)
	{
		m_uActiveFrames = 0;
	}

	bool ModeTransition::Update(float fElapsed)
	{
		// Be really careful what you put in here, the loading thead is going on in the background
		m_uActiveFrames++;

		return m_uActiveFrames > 4;
	}

	void ModeTransition::PreDraw(usg::GFXDevice* pDevice, usg::GFXContext* pImmContext)
	{
	}

	void ModeTransition::Draw(usg::Display* pDisplay, usg::IHeadMountedDisplay* pHMD, usg::GFXContext* pImmContext)
	{
		// Draw directly to the screen, and clear it
		pImmContext->RenderToDisplay(pDisplay, RenderTarget::RT_FLAG_COLOR_0);
	}

	void ModeTransition::PostDraw(usg::GFXDevice* pDevice)
	{

	}

}

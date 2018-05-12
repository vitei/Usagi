/****************************************************************************
//	Filename: ModeGame.h
//	Description: The meat and potatoes of the game
*****************************************************************************/
#pragma once
#include "Engine/Common/Common.h"
#include "Engine/PostFX/PostFXSys.h"
#include "Engine/Game/SystemsMode.h"

namespace usg
{
	class Debug3D;
	class GameView;
}

class ModeGame : public usg::SystemsMode
{
	typedef usg::SystemsMode Inherited;
public:
	ModeGame();
	virtual ~ModeGame();

	void Init(usg::GFXDevice* pDevice) override;
	void CleanUp(usg::GFXDevice* pDevice) override;
	bool Update(float fElapsed) override;
	void PreDraw(usg::GFXDevice* pDevice, usg::GFXContext* pImmContext) override;
	void Draw(usg::Display* pDisplay, usg::IHeadMountedDisplay* pHMD, usg::GFXContext* pImmContext) override;
	void PostDraw(usg::GFXDevice* pDevice) override;

	void NotifyResize(usg::GFXDevice* pDevice, uint32 uDisplay, uint32 uWidth, uint32 uHeight) override;

private:

	void InitGameView(usg::GFXDevice* pDevice)
	void InitRoot();

	usg::PostFXSys		m_postFX;
	usg::GameView*		m_pGameView;
};


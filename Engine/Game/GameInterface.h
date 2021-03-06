/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: The main entry and game state manager.
*****************************************************************************/
#pragma once

#ifndef USG_GAME_INTERFACE_H
#define USG_GAME_INTERFACE_H



namespace usg {

class GameInterface;
class GFXDevice;

class GameInterface
{
public:
	GameInterface();
	virtual ~GameInterface();

	// Public functions to change game state (to allow conditional checking) should be implemented for all
	virtual bool IsRunning() const { return m_bIsRunning; }
	virtual bool ResetReq() const { return m_bReqReset; }
	virtual void PreGFXInit() = 0;
	virtual void Init(GFXDevice* pDevice, usg::ResourceMgr* pResMgr) = 0;
	virtual void Cleanup(GFXDevice* pDevice) = 0;
	virtual void Update(GFXDevice* pDevice) = 0;
	virtual void Draw(GFXDevice* pDevice) = 0;
	virtual void OnMessage(GFXDevice* const pDevice, const uint32 messageID, const void* const pParameters) = 0;
	void Quit() { m_bIsRunning = false; }
protected:
	bool				m_bIsRunning;
	bool				m_bReqReset;
};

usg::GameInterface*	CreateGame();
const char*		GetGameName();

} // namespace usg

#endif // USG_GAME_INTERFACE_H

/****************************************************************************
// 	FileName CustomUIItem.h
//  Description: A base class for non standard renderables in the UI
//  Note that no effects should override descriptor 0
****************************************************************************/
#pragma once

namespace usg
{
	class GFXDevice;
	class Vector2f;
	class Color;
	class GFXContext;

class CustomUIItem
{
public:
	CustomUIItem() { m_bRegistered = false;}
	virtual ~CustomUIItem() {}

	virtual void AddToUI(usg::GFXDevice* pDevice, const usg::RenderPassHndl& renderPass, const usg::Vector2f& vPos, const usg::Vector2f& vScale, const usg::Color& cColor) {}
	virtual void Cleanup(usg::GFXDevice* pDevice) {}

	virtual void Update(float fElapsed) {}
	virtual void GPUUpdate(usg::GFXDevice* pDevice) {}
	virtual void SetColor(const usg::Color& col) {}

	virtual bool Draw(usg::GFXContext* pContext) { return false; }

	void Registered() { m_bRegistered = true; }
	bool HasBeenRegistered() const { return m_bRegistered; }
private:
	bool m_bRegistered;
};

}

/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: A visible light
*****************************************************************************/
#ifndef _USG_GRAPHICS_LIGHT_LIGHT_H_
#define _USG_GRAPHICS_LIGHT_LIGHT_H_

#include "Engine/Graphics/Color.h"
#include "Engine/Graphics/RenderConsts.h"

namespace usg {

class LightMgr;
class GFXDevice;
class Scene;
class GFXContext;

enum LightType
{
	LIGHT_TYPE_DIR = 0,
	LIGHT_TYPE_POS,
	LIGHT_TYPE_SPOT,
	LIGHT_TYPE_PROJ
};

class Light
{
public:
	Light(LightType eType);
	virtual ~Light(void);

	virtual void	Init(GFXDevice* pDevice, Scene* pScene, bool bSupportsShadow);

	virtual void	Cleanup(GFXDevice* pDevice, Scene* pScene) {};

	void	SetDiffuse(const Color &diffuse);
	void	SetAmbient(const Color& ambient);
	void	SetSpecularColor(const Color& specular);
	void	SetName(const char* szName) { m_name = szName; }
	const usg::string& GetName() { return m_name; }

	const Color&	GetSpecular() const { return m_specular; }
	const Color&	GetDiffuse() const	{ return m_colour; }
	const Color&	GetAmbient() const	{ return m_ambient; }

	void SwitchOn(bool bOn);
	void EnableShadow(bool bEnable);
	bool IsActive() const { return m_bActive; }
	bool SupportsShadow() const { return m_bSupportsShadow; }
	bool GetShadowEnabled() const { return m_bShadowEnabled && m_bSupportsShadow; }

	LightType GetType() const { return m_eType; }

	virtual void GPUUpdate(GFXDevice* pDevice) {}
	virtual void SetShadowExcludeFlags(uint32 uFlags) {}
	virtual bool ShadowRender(GFXContext* pContext) { return false; }

	virtual void SetPosition(const Vector4f &position) { ASSERT(false); }
	virtual void SetDirection(const Vector4f &direction) { ASSERT(false); }
	virtual const Vector4f& GetPosition() const { ASSERT(false); return V4F_ZERO; }
	virtual const Vector4f& GetDirection() const { ASSERT(false); return V4F_ZERO; }

	// To avoid updating lights when not rendering
	void SetVisibleFrame(uint32 uFrame) { m_uVisibleFrame = uFrame; }
	uint32 GetVisibleFrame() { return m_uVisibleFrame; }
	uint32 GetShadowQuality() const { return m_uShadowQuality; }
protected:

	usg::string		m_name;
	uint32			m_uVisibleFrame;
	uint32			m_uShadowQuality;
	// RGB, specular power
	Color			m_colour;
	Color			m_ambient;
	Color			m_specular;

	bool			m_bActive;
	bool			m_bSupportsShadow;
	bool			m_bShadowEnabled;
	bool			m_bDirty;

	const LightType	m_eType;
};

}

#endif

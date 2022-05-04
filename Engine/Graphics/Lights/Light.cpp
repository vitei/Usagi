/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Lights/Light.h"
#include "Engine/Scene/Scene.h"

namespace usg {

Light::Light(LightType eType):
m_eType(eType)
{
	m_bActive = false;
	m_bDirty = false;
	m_uShadowQuality = 0;
	m_uVisibleFrame = USG_INVALID_ID;
	m_specular.Assign(0.0f, 0.0f, 0.0f, 0.0f);
	m_ambient.Assign(0.0f, 0.0f, 0.0f, 0.0f);
	m_bShadowEnabled = false;
	m_bSupportsShadow = false;
}

Light::~Light()
{

}

void Light::Init(GFXDevice* pDevice, Scene* pScene, bool bSupportsShadow)
{
	m_bShadowEnabled = bSupportsShadow;
	m_bSupportsShadow = bSupportsShadow;
}


void Light::SetAmbient(const Color &ambient)
{
	m_ambient = ambient;
	m_bDirty = true;
}

void Light::SetDiffuse(const Color &diffuse)
{
	m_colour = diffuse;
	m_bDirty = true;
}

void Light::SetSpecularColor(const Color& color)
{
	m_specular = color;
	m_bDirty = true;
}

void Light::SwitchOn(bool bOn)
{
	m_bActive = bOn;
}

void Light::EnableShadow(bool bEnable)
{
	ASSERT(m_bSupportsShadow);
	if (m_bSupportsShadow)
	{
		m_bShadowEnabled = bEnable;
	}
}

}

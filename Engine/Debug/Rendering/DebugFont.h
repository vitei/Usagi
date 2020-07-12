/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#pragma once

#include "Engine/Graphics/Textures/Texture.h"
#include "Engine/Graphics/Materials/Material.h"
#include "Engine/Core/File/File.h"

namespace usg {

class DebugFont
{
public:
	DebugFont(void);
	~DebugFont(void);

	bool	Load(GFXDevice* pDevice, ResourceMgr* pResMgr, const char* szName);

	float	GetStringWidth(const char* string, float height) const;

	TextureHndl GetTexture() { return m_pTexture; }

	inline void	GetCharacterUV(char c, float& u, float &v);

	float GetChrWidth() { return m_fTexelWidth; }
	float GetChrHeight() { return m_fTexelHeight; }
	
private:

	uint32						m_uChrPerLine;

	float32						m_fTexelWidth;
	float32						m_fTexelHeight;

	float32						m_fUVChrWidth;
	float32						m_fUVChrHeight;
	float32						m_fOffsetY;

	TextureHndl					m_pTexture;
};


inline void	DebugFont::GetCharacterUV(char c, float& u, float &v)
{
	// TODO: Move this to the debug font code
	int ci = c-' ';
	int cx = ci % m_uChrPerLine;
	int cy = ci / m_uChrPerLine;

	// TODO: Move this to the VS
	u = m_fUVChrWidth*((float)cx);
	v = m_fUVChrHeight*(m_fOffsetY-((float)cy));
}

}

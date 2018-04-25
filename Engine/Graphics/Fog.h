/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Fog settings
*****************************************************************************/
#ifndef _USG_GRAPHICS_FOG_H_
#define _USG_GRAPHICS_FOG_H_
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Color.h"
#include "Engine/Graphics/RenderConsts.h"

namespace usg {


class Fog
{
public:
	Fog();
	~Fog();

	void SetActive(bool bActive);
	void SetType(FogType etype);
	void SetMinDepth(float depth);
	void SetMaxDepth(float depth);
	
	bool GetActive() const { return m_bActive; }
	FogType GetType() const { return m_eType; }
	const Color& GetColor() const { return m_color; }
	float GetDensity() const { return m_fDensity; }
	float GetMinDepth() const { return m_fMinDepth; }
	float GetMaxDepth() const { return m_fMaxDepth; }

	const Color& GetColor() { return m_color;  }


private:
	PRIVATIZE_COPY(Fog)

	bool	m_bActive;
	FogType	m_eType;
	Color	m_color;
	float	m_fDensity;
	float	m_fMinDepth;
	float	m_fMaxDepth;
};

}

#endif

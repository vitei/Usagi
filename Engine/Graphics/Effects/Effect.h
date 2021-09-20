/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_GRAPHICS_EFFECT_
#define _USG_GRAPHICS_EFFECT_

#include "Engine/Graphics/Primitives/VertexDeclaration.h"
#include API_HEADER(Engine/Graphics/Effects, Effect_ps.h)
#include "Engine/Resource/ResourceBase.h"
#include "Engine/Resource/CustomEffectDecl.h"
#include "Engine/Resource/FileDependencies.h"


namespace usg {

class Texture;
class GFXDevice;

class Effect : public ResourceBase
{
public:
	Effect();
	virtual ~Effect();

	virtual bool Init(GFXDevice* pDevice, const PakFileDecl::FileInfo* pFileHeader, const FileDependencies* pDependencies, const void* pData) override;
	void Cleanup(GFXDevice* pDevice) { m_platform.Cleanup(pDevice); }
	//void Apply() const;

	Effect_ps& GetPlatform() { return m_platform; }
	const Effect_ps& GetPlatform() const { return m_platform; }

	const static ResourceType StaticResType = ResourceType::EFFECT;
	
	// For optional information about attributes and descriptors
	CustomEffectResHndl GetCustomEffect() const { return m_customFX; }

private:
	PRIVATIZE_RES_COPY(Effect)

	Effect_ps			m_platform;
	CustomEffectResHndl	m_customFX;
};


}
 
#endif

/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_GRAPHICS_EFFECT_
#define _USG_GRAPHICS_EFFECT_
#include "Engine/Common/Common.h"
#include "Engine/Core/String/U8String.h"
#include "Engine/Graphics/Primitives/VertexDeclaration.h"
#include API_HEADER(Engine/Graphics/Effects, Effect_ps.h)
#include "Engine/Resource/ResourceBase.h"
#include "Engine/Resource/PakFile.h"


namespace usg {

class Texture;
class GFXDevice;

class Effect : public ResourceBase
{
public:
	Effect();
	virtual ~Effect();

	bool Init(GFXDevice* pDevice, PakFile* pFile, const PakFileDecl::FileInfo* pFileHeader, const void* pData);
	void CleanUp(GFXDevice* pDevice) { m_platform.CleanUp(pDevice); }
	//void Apply() const;

	Effect_ps& GetPlatform() { return m_platform; }
	const Effect_ps& GetPlatform() const { return m_platform; }
	const U8String& GetName() const { return m_name; }

	const static ResourceType StaticResType = ResourceType::EFFECT;

private:
	PRIVATIZE_RES_COPY(Effect)

	U8String	m_name;
	Effect_ps	m_platform;

	CustomEffectDecl::Sampler* m_pSamplers;
	CustomEffectDecl::Attribute* m_pAttributes;
	CustomEffectDecl::ConstantSet* m_pConstantSets;
	PakFileDecl::EffectEntry* m_pHeader;

	void*	m_pBinary;
};


}
 
#endif

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
#include "Engine/Resource/ResourcePak.pb.h"

namespace usg {

class Texture;
class GFXDevice;

class Effect : public ResourceBase
{
public:
	Effect() { m_resourceType = ResourceType::EFFECT; }
	virtual ~Effect() {}

	void Init(GFXDevice* pDevice, const char* szEffectName);
	bool Init(GFXDevice* pDevice, PakFile* pFile, const PakFileDecl::FileInfo* pFileHeader, const void* pData);
	void CleanUp(GFXDevice* pDevice) { m_platform.CleanUp(pDevice); }
	//void Apply() const;

	Effect_ps& GetPlatform() { return m_platform; }
	const Effect_ps& GetPlatform() const { return m_platform; }
	const U8String& GetName() const { return m_name; }

private:
	PRIVATIZE_COPY(Effect)

	U8String	m_name;
	Effect_ps	m_platform;
};


inline void Effect::Init(GFXDevice* pDevice, const char* szEffectName)
{
	m_name = szEffectName;
	SetupHash( m_name.CStr() );
	m_platform.Init( pDevice, szEffectName );
	SetReady(true);
}

inline bool Effect::Init(GFXDevice* pDevice, PakFile* pFile, const PakFileDecl::FileInfo* pFileHeader, const void* pData)
{
	m_name = pFileHeader->szName;
	SetupHash(m_name.CStr());
	bool bLoaded = m_platform.Init(pDevice, pFile, pFileHeader, pData, pFileHeader->uDataSize);
	// FIXME: This should be done internally
	SetReady(true);
	return bLoaded;
}

}
 
#endif

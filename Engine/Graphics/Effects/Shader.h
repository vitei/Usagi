/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//  A shader for a specific pipeline stage - it is perfectly valid to not 
//  implement this on platforms which use combined effects
****************************************************************************/
#ifndef _USG_GRAPHICS_SHADER_
#define _USG_GRAPHICS_SHADER_

#include "Engine/Core/String/U8String.h"
#include API_HEADER(Engine/Graphics/Effects, Shader_ps.h)
#include "Engine/Resource/ResourceBase.h"

namespace usg {

class Texture;
class GFXDevice;

class Shader : public ResourceBase
{
public:
	Shader() : ResourceBase(StaticResType) {}
	virtual ~Shader() {}

	virtual bool Init(GFXDevice* pDevice, const PakFileDecl::FileInfo* pFileHeader, const class FileDependencies* pDependencies, const void* pData) override;
	void CleanUp(GFXDevice* pDevice) { m_platform.CleanUp(pDevice); }

	Shader_ps& GetPlatform() { return m_platform; }
	const Shader_ps& GetPlatform() const { return m_platform; }

	const static ResourceType StaticResType = ResourceType::SHADER;

private:
	PRIVATIZE_RES_COPY(Shader)

	Shader_ps	m_platform;
};


inline bool Shader::Init(GFXDevice* pDevice, const PakFileDecl::FileInfo* pFileHeader, const class FileDependencies* pDependencies, const void* pData)
{
	SetupHash(pFileHeader->szName);
	bool bLoaded = m_platform.Init(pDevice, pFileHeader, pDependencies, pData);
	// FIXME: This should be done internally
	SetReady(true);
	return bLoaded;
}

}
 
#endif

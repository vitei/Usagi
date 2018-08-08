/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_GRAPHICS_CONSTANT_SET_H_
#define _USG_GRAPHICS_CONSTANT_SET_H_
#include "Engine/Maths/Matrix4x4.h"
#include "Engine/Graphics/RenderConsts.h"
#include API_HEADER(Engine/Graphics/Effects, ConstantSet_ps.h)

#define SHADER_CONSTANT_ELEMENT( struct_name, element, type, count ) \
	{ #element, type, count, offsetof(struct_name, element), 0, NULL }


#define SHADER_CONSTANT_STRUCT_ARRAY( parent_struct_name, element, struct_decl, count) \
	{ #element, CT_STRUCT, count, offsetof(parent_struct_name, element), sizeof(((parent_struct_name*)0)->element[0]), struct_decl }

// Used to terminate a shader constant declaration
#define SHADER_CONSTANT_END() \
	{ "", usg::CT_INVALID, 0, 0, NULL }

namespace usg {

class ConstantSet
{
public:
	ConstantSet();
	~ConstantSet(); 

	// Only need to pass in an effect if this is a dynamic constant set
	void Init(class GFXDevice* pDevice, const ShaderConstantDecl* pDecl, GPUUsage eUsage = GPU_USAGE_DYNAMIC, void* pData = nullptr);
	void CleanUp(class GFXDevice* pDevice);
	bool IsValid() const { return m_uSize > 0; }

	uint32	GetSize() const { return m_uSize; }
	const void*	GetCPUData() const { return m_pCPUData; }
	const ShaderConstantDecl* GetDeclaration() const { return m_pDecl; }
	uint32 GetVarCount() const { return m_uVarCount; }

	template <class BufferType>
	inline BufferType* Lock() { return (BufferType*)Lock(sizeof(BufferType)); }
	void* Lock(uint32 uSize);
	void  Unlock();

	void UpdateData(GFXDevice* pDevice);

	ConstantSet_ps& GetPlatform() { return m_platform; }
	const ConstantSet_ps& GetPlatform() const { return m_platform; }

	bool GetDirty() const { return m_bDirty;  }
	uint32 GetLastUpdate() const { return m_uLastUpdate; }

private:
	PRIVATIZE_COPY(ConstantSet)

	void AppendDeclaration(const ShaderConstantDecl* pDecl);

	GPUUsage					m_eUsage;
	ConstantSet_ps				m_platform;

	const ShaderConstantDecl*	m_pDecl;
	uint32						m_uVarCount;
	void*						m_pCPUData;
	uint32						m_uSize;
	bool						m_bLocked;
	bool						m_bDirty;
	uint32						m_uLastUpdate;
};

}

#endif

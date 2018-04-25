/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: The runtime data (constant sets) for a data defined effect
*****************************************************************************/
#ifndef _USG_GRAPHICS_SCENE_CUSTOM_EFFECT_RUNTIME_H_
#define _USG_GRAPHICS_SCENE_CUSTOM_EFFECT_RUNTIME_H_
#include "Engine/Common/Common.h"
#include "Engine/Resource/ResourceDecl.h"

namespace usg{

class GFXDevice;
class Camera;

class CustomEffectRuntime
{
public:
	CustomEffectRuntime();
	~CustomEffectRuntime();
    
	void Init(GFXDevice* pDevice, const char* szName);
	void Init(GFXDevice* pDevice, const CustomEffectRuntime* pCopy);
	void CleanUp(GFXDevice* pDevice);
	void GPUUpdate(GFXDevice* pDevice);
	
	// FIXME: Set variables by CRC instead of string
	// Change start ID into an offset
	template <class VariableType>
	bool SetVariable(const char* szName, VariableType var, uint32 uStartId = 0)
	{
		return SetVariable(szName, (void*)&var, sizeof(VariableType), uStartId);
	}

	template <class VariableType>
	bool SetVariable(const char* szName, VariableType* pVar, uint32 uCount = 1, uint32 uStartId = 0) { return SetVariable(szName, (void*)pVar, sizeof(VariableType)*uCount, uStartId); }
	bool SetVariable(const char* szName, void* pData, uint32 uSize, uint32 uStartId);

	uint32 GetVariableCount(const char* szName) const;

	void SetSetData(uint32 uSet, void* pData, uint32 uSize);
    
	ConstantSet* GetConstantSet(uint32 uSet);
	const ConstantSet* GetConstantSet(uint32 uSet) const;

	const CustomEffectResHndl& GetResource() const { return m_resource; }

private:
	CustomEffectResHndl	m_resource;
	ConstantSet* 		m_pConstantSets;
	uint32				m_uConstantSets;
};

}

#endif


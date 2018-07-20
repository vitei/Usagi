/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: The binary and associated states for a data defined effect
//	declaration (e.g. for models and particle effects)
*****************************************************************************/
#ifndef _USG_RESOURCE_CUSTOM_EFFECT_RESOURCE_H_
#define _USG_RESOURCE_CUSTOM_EFFECT_RESOURCE_H_
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Device/GFXHandles.h"
#include "Engine/Resource/ResourceDecl.h"
#include "Engine/Resource/ResourceBase.h"
#include "CustomEffectDecl.h"

namespace usg
{

	class EffectPak : public ResourceBase
	{
	public:
		EffectPak();
		~EffectPak();

		void Init(GFXDevice* pDevice, const char* szFileName);
		void CleanUp(GFXDevice* pDevice);


	private:
		Effect* m_pEffects;
		uint32	m_uEffects;
		Shader*	m_pShaders;
		uint32	m_uShaders;
	};

}

#endif	// #ifndef _USG_GRAPHICS_SCENE_MODEL_H_

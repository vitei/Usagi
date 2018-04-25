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
	namespace CustomEffectDecl
	{
		struct Sampler;
		struct Attribute;
		struct CustomEffectDecl;
		struct Constant;
		struct Header;
	};

	class CustomEffectResource : public ResourceBase
	{
	public:
		CustomEffectResource();
		~CustomEffectResource();

		void Init(GFXDevice* pDevice, const char* szFileName);

		uint32 GetAttribBinding(const char* szAttrib) const;
		uint32 GetSamplerBinding(const char* szSampler) const;

		const ShaderConstantDecl* GetConstantDecl(uint32 uIndex) const { return &m_pShaderConstDecl[m_uConstDeclOffset[uIndex]]; }

		uint32 GetConstantSetCount() const;
		uint32 GetConstantCount(uint32 uSet) const;
		const CustomEffectDecl::Constant* GetConstant(uint32 uSet, uint32 uConstant) const;
		uint32 GetBindingPoint(uint32 uSet);

		void* GetDefaultData(uint32 uSet) const;

		const char* GetEffectName() const;
		const char* GetDeferredEffectName() const;
		const char* GetDepthEffectName() const;
		const char* GetOmniDepthEffectName() const;

	private:
		enum
		{
			MAX_CONSTANT_SETS = 4
		};
		void*							m_pBinary;
		ShaderConstantDecl*		 		m_pShaderConstDecl;
		uint32							m_uConstDeclOffset[MAX_CONSTANT_SETS];

		CustomEffectDecl::Header*		m_pHeader;
		CustomEffectDecl::ConstantSet*	m_pConstantSets;
		CustomEffectDecl::Sampler*		m_pSamplers;
		CustomEffectDecl::Attribute*	m_pAttributes;
	};

}

#endif	// #ifndef _USG_GRAPHICS_SCENE_MODEL_H_

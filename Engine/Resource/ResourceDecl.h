/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Forward declaration of all the resources to minimize headers
*****************************************************************************/
#ifndef _USG_RESOURCE_RESOURCE_DECL_H_
#define _USG_RESOURCE_RESOURCE_DECL_H_
#include "Engine/Common/Common.h"
#include "Engine/Core/Containers/ResourcePointer.h"
#include "Engine/Resource/ResourceBase.h"


namespace usg
{

class GFXDevice;
class Effect;
class Texture;
class ModelResource;
class Font;
class MaterialAnimationResource;
class SkeletalAnimationResource;
class ParticleEffectResource;
class ParticleEmitterResource;
class CollisionModelResource;
class ProtocolBufferFile;
class ConstantSet;
class CustomEffectResource;
class ResourceBase;
class ResourcePakHdr;

typedef ResourcePointer<const ResourceBase> BaseResHandle;

template <class AssetType>
const AssetType* GetAs(BaseResHandle& hndl)
{
	if (hndl.get() && hndl.get()->GetResourceType() == AssetType::StaticResType)
	{
		return (AssetType*)hndl.get();
	}
	return nullptr;
}

template <class HighLevelType, ResourceType CmpType>
class ResourceHandle : public BaseResHandle
{
public:
	// Override the accessors so we can grab the high level resource
	ResourceHandle() : ResourcePointer() {}
	~ResourceHandle() {}

	ResourceHandle(HighLevelType* pData) : ResourcePointer((ResourceBase*)pData) {}
	ResourceHandle(const BaseResHandle &rhs) : BaseResHandle(rhs)
	{
		ASSERT(!rhs.get() || rhs->GetResourceType() == CmpType);
	}

	HighLevelType* operator->() const
	{
		return (HighLevelType*)(m_pPointer);
	}
	HighLevelType& operator*() const { return *((HighLevelType*)m_pPointer); }
	HighLevelType* get() const { return (HighLevelType*)m_pPointer; }

	ResourceHandle& operator=(BaseResHandle &rhs) { ASSERT(rhs->GetResourceType() == CmpType); return *(BaseResHandle)this; }
};

typedef ResourceHandle<const CollisionModelResource, ResourceType::COLLISION> CollisionModelResHndl;
typedef ResourceHandle<const ModelResource, ResourceType::MODEL> ModelResHndl;
typedef ResourceHandle<const Effect, ResourceType::EFFECT> EffectHndl;
typedef ResourceHandle<const Texture, ResourceType::TEXTURE> TextureHndl;
typedef ResourceHandle<const Font, ResourceType::FONT> FontHndl;
typedef ResourceHandle<const CustomEffectResource, ResourceType::CUSTOM_EFFECT> CustomEffectResHndl;
typedef ResourceHandle<const SkeletalAnimationResource, ResourceType::SKEL_ANIM> SkeletalAnimationResHndl;
typedef ResourceHandle<const ParticleEffectResource, ResourceType::PARTICLE_EFFECT> ParticleEffectResHndl;
typedef ResourceHandle<const ParticleEmitterResource, ResourceType::PARTICLE_EMITTER> ParticleEmitterResHndl;
typedef ResourceHandle<const ResourcePakHdr, ResourceType::PAK_HEADER> ResourcePakHndl;


}

#endif

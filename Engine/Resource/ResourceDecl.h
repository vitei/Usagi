/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Forward declaration of all the resources to minimize headers
*****************************************************************************/
#ifndef _USG_RESOURCE_RESOURCE_DECL_H_
#define _USG_RESOURCE_RESOURCE_DECL_H_
#include "Engine/Common/Common.h"
#include "Engine/Core/Containers/SharedPointer.h"


namespace usg
{

class GFXDevice;
class Effect;
class Texture;
class ModelResource;
class Font;
class LookupTable;
class MaterialAnimationResource;
class SkeletalAnimationResource;
class ParticleEffectResource;
class ParticleEmitterResource;
class CollisionModelResource;
class ProtocolBufferFile;
class ConstantSet;
class CustomEffectResource;

typedef SharedPointer<const CollisionModelResource> CollisionModelResHndl;
typedef SharedPointer<const ModelResource> ModelResHndl;
typedef SharedPointer<const Effect> EffectHndl;
typedef SharedPointer<const Texture> TextureHndl;
typedef SharedPointer<const Font> FontHndl;
typedef SharedPointer<const CustomEffectResource> CustomEffectResHndl;
typedef SharedPointer<const LookupTable> LookupTableHndl;
typedef SharedPointer<const SkeletalAnimationResource> SkeletalAnimationResHndl;
typedef SharedPointer<const ParticleEffectResource> ParticleEffectResHndl;
typedef SharedPointer<const ParticleEmitterResource> ParticleEmitterResHndl;


}

#endif

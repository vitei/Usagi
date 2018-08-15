/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Forward declaration of all the resources to minimize headers
*****************************************************************************/
#ifndef _USG_RESOURCE_RESOURCE_DECL_H_
#define _USG_RESOURCE_RESOURCE_DECL_H_
#include "Engine/Common/Common.h"
#include "Engine/Core/Containers/ResourcePointer.h"


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

typedef ResourcePointer<const CollisionModelResource> CollisionModelResHndl;
typedef ResourcePointer<const ModelResource> ModelResHndl;
typedef ResourcePointer<const Effect> EffectHndl;
typedef ResourcePointer<const Texture> TextureHndl;
typedef ResourcePointer<const Font> FontHndl;
typedef ResourcePointer<const CustomEffectResource> CustomEffectResHndl;
typedef ResourcePointer<const LookupTable> LookupTableHndl;
typedef ResourcePointer<const SkeletalAnimationResource> SkeletalAnimationResHndl;
typedef ResourcePointer<const ParticleEffectResource> ParticleEffectResHndl;
typedef ResourcePointer<const ParticleEmitterResource> ParticleEmitterResHndl;


}

#endif

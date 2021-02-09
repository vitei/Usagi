/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_MATERIAL_ANIMATION_RESOURCE_H_
#define _USG_MATERIAL_ANIMATION_RESOURCE_H_

#include "Engine/Resource/CurveAnimResourceBase.h"

namespace usg {

class Model;

class MaterialAnimationResource : public CurveAnimResourceBase
{
public:
	MaterialAnimationResource() : CurveAnimResourceBase(StaticResType) {}
	virtual ~MaterialAnimationResource() {}

	void ApplyAllMaterialAnimations( float frame, Model& model ) const;
	
	const static ResourceType StaticResType = ResourceType::MAT_ANIM;
private:
	void ApplyAnimation( float frame, Model& model, Member* pMember ) const;
	void ApplyScale( float frame, Model& model, Member* pMember ) const;
	void ApplyRotate( float frame, Model& model, Member* pMember ) const;
	void ApplyTranslate( float frame, Model& model, Member* pMember ) const;
	void ApplyColor( float frame, Model& model, Member* pMember, const char* szName ) const;

};

}

#endif // _USG_MATERIAL_ANIMATION_RESOURCE_H_

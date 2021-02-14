/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Scene/Model/MaterialAnimation.pb.h"
#include "Engine/Scene/Model/Model.h"
#include "Engine/Scene/Model/UVMapper.h"
#include "MaterialAnimationResource.h"

namespace usg
{

void MaterialAnimationResource::ApplyAllMaterialAnimations( float frame, Model& model ) const
{
	if(m_header.isLoop)
	{
		uint32 uRepeat = (uint32)(frame / m_header.frameSize);
		frame -= ((float)uRepeat) * m_header.frameSize;
	}
	for( int setNo = 0; setNo < m_header.memberSetsNum; ++setNo )
	{
		MemberSet& memberSet = m_memberSets[setNo];
		for( int memberNo = 0; memberNo < memberSet.data.membersNum; ++memberNo )
		{
			Member* pMember = memberSet.member( memberNo );
			ApplyAnimation( frame, model, pMember );
		}
	}
}


void MaterialAnimationResource::ApplyAnimation( float frame, Model& model, Member* pMember ) const
{
	switch( pMember->data.type )
	{
		case exchange::MaterialAnimationMemberType_SCALE:
			ApplyScale( frame, model, pMember );
			break;
		case exchange::MaterialAnimationMemberType_ROTATE:
			ApplyRotate( frame, model, pMember );
			break;
		case exchange::MaterialAnimationMemberType_TRANSLATE:
			ApplyTranslate( frame, model, pMember );
			break;

		case exchange::MaterialAnimationMemberType_COLOR_EMISSION:
			ApplyColor(frame, model, pMember, "emission");
			break;
		case exchange::MaterialAnimationMemberType_COLOR_AMBIENT:
			ApplyColor(frame, model, pMember, "ambient");
			break;
		case exchange::MaterialAnimationMemberType_COLOR_DIFFUSE:
			ApplyColor(frame, model, pMember, "diffuse");
			break;
		case exchange::MaterialAnimationMemberType_COLOR_SPECULAR_0:
			ApplyColor(frame, model, pMember, "specular");
			break;
		default:
			ASSERT( 0 );
			break;
	}
}

void MaterialAnimationResource::ApplyScale( float frame, Model& model, Member* pMember ) const
{
	const int axisCount = 2; // XY
	const Curve* pCurves[axisCount] = {};
	CollectCurves( pCurves, axisCount, pMember );

	float valX = pCurves[0] ? Calc( frame, pCurves[0] ) : 0.0f;
	float valY = pCurves[1] ? Calc( frame, pCurves[1] ) : 0.0f;
	model.SetTextureScale( pMember->data.targetName, pMember->data.targetID, valX, valY );
}

void MaterialAnimationResource::ApplyRotate( float frame, Model& model, Member* pMember ) const
{
	uint32 meshIndex = model.GetMeshIndex( pMember->data.targetName, Model::IDENTIFIER_MATERIAL );
	UVMapper* pMapper =
		meshIndex != USG_INVALID_ID ? model.GetUVMapper( meshIndex, pMember->data.targetID ) : NULL;

	if( pMapper )
	{
		ASSERT( pMember->data.curveNum > 0 );
		const Curve* pCurve = pMember->curve( 0 );

		float rot = Calc( frame, pCurve );
		pMapper->SetUVRotation( rot );
	}
}

void MaterialAnimationResource::ApplyTranslate( float frame, Model& model, Member* pMember ) const
{
	uint32 meshIndex = model.GetMeshIndex( pMember->data.targetName, Model::IDENTIFIER_MATERIAL );
	UVMapper* pMapper =
		meshIndex != USG_INVALID_ID ? model.GetUVMapper( meshIndex, pMember->data.targetID ) : NULL;

	if( pMapper )
	{
		const int axisCount = 2; // XY
		const Curve* pCurves[axisCount] = {};
		CollectCurves( pCurves, axisCount, pMember );

		float valX = pCurves[0] ? Calc( frame, pCurves[0] ) : 0.0f;
		float valY = pCurves[1] ? Calc( frame, pCurves[1] ) : 0.0f;
		pMapper->SetUVTranslation( Vector2f( valX, valY ) );
	}
}


void MaterialAnimationResource::ApplyColor(float frame, Model& model, Member* pMember, const char* szName) const
{
	Color color;

	const int axisCount = 4; // RGBA
	const Curve* pCurves[axisCount] = {};
	CollectCurves( pCurves, axisCount, pMember );

	if( pCurves[0] )
	{ 
		color.r() = Calc( frame, pCurves[0] ); 
	}
	if( pCurves[1] )
	{
		color.g() = Calc( frame, pCurves[1] );
	}
	if( pCurves[2] )
	{
		color.b() = Calc( frame, pCurves[2] );
	}
	if( pCurves[3] )
	{
		color.a() = Calc( frame, pCurves[3] );
	}
	model.OverrideVariable(szName, color );
}

}
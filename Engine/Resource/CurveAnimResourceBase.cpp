/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Maths/spline.h"
#include "CurveAnimResourceBase.h"

namespace usg
{
	
bool CurveAnimResourceBase::Load( const char* szName )
{
	ProtocolBufferFile file( szName );
		
	if( !file.Read( &m_header ) ) {
		return false;
	}

	m_memberSets = vnew( ALLOC_OBJECT ) MemberSet[m_header.memberSetsNum];

	bool bSucceeded = true;
	for( int i = 0; i < m_header.memberSetsNum; ++i )
	{
		bSucceeded = LoadMemberSet( file, &m_memberSets[i] );
		if( !bSucceeded )
		{
			break;
		}
	}

	SetupHash(szName);
	SetReady(true);
	
	return bSucceeded;
}

bool CurveAnimResourceBase::LoadMemberSet( ProtocolBufferFile& file, MemberSet* memberSet )
{
	if( !file.Read( &memberSet->data ) )
	{
		return false;
	}

	memberSet->AllocArray( memberSet->data.membersNum );
	
	bool bSucceeded = true;
	for( int i = 0; i < memberSet->data.membersNum; ++i )
	{
		bSucceeded = LoadMember( file, memberSet->member(i) );
		if( !bSucceeded )
		{
			break;
		}
	}

	return bSucceeded;
}

bool CurveAnimResourceBase::LoadMember( ProtocolBufferFile& file, Member* member )
{
	if( !file.Read( &member->data ) )
	{
		return false;
	}

	member->AllocArray( member->data.curveNum );

	bool bSucceeded = true;
	for( int i = 0; i < member->data.curveNum; ++i )
	{
		bSucceeded = LoadCurve( file, member->curve(i) );
		if( !bSucceeded )
		{
			break;
		}
	}

	return bSucceeded;
}

bool CurveAnimResourceBase::LoadCurve( ProtocolBufferFile& file, Curve* curve )
{
	if( !file.Read( &curve->data ) )
	{
		return false;
	}

	curve->AllocArray( curve->data.keyFrameNum );

	bool bSucceeded = true;
	for( int i = 0; i < curve->data.keyFrameNum; ++i )
	{
		bSucceeded = file.Read( &curve->keyFrame(i)->data );
		if( !bSucceeded )
		{
			break;
		}
	}
	return bSucceeded;
}

void CurveAnimResourceBase::CollectCurves( const Curve* pCurves[], int curvesCount, Member* pMember ) const
{
	for( int i = 0; i < pMember->data.curveNum; ++i )
	{
		int axis = pMember->curve( i )->data.axis;
		ASSERT( 0 <= axis && axis < curvesCount );
		pCurves[axis] = pMember->curve( i );
	}
}

float CurveAnimResourceBase::Calc( float frame, const Curve* pCurve ) const
{
	int keyCount = pCurve->data.keyFrameNum;
	const KeyFrame* keys = pCurve->array();

	// handle frames outside of the curve
	if( keyCount == 1 || frame <= keys[0].data.frame )
	{
		return keys[0].data.value;
	}
	else if( frame >= keys[keyCount - 1].data.frame )
	{
		return keys[keyCount - 1].data.value;
	}

	// find two keyframes
	int indexL = 0, indexR = keyCount - 1;

	while( indexL != indexR - 1 && indexL != indexR )
	{
		int indexCenter = ( indexL + indexR ) / 2;

		if( frame <= keys[indexCenter].data.frame )
		{
			indexR = indexCenter;
		}
		else
		{
			indexL = indexCenter;
		}
	}
	const exchange::CurveKeyFrame& key0 = keys[indexL].data;
	const exchange::CurveKeyFrame& key1 = keys[indexR].data;

	float ret = 0.0f;
	switch( key0.type )
	{
		case exchange::CurveKeyFrameType_HERMITE:
			ret = spline::CalcHermite( frame, key0.frame, key0.value, key0.outSlope,
									   key1.frame, key1.value, key1.inSlope );
			break;
		case exchange::CurveKeyFrameType_LINEAR:
			ret = spline::CalcLinear( frame, key0.frame, key0.value,
							  key1.frame, key1.value );
			break;
		case exchange::CurveKeyFrameType_STEP:
			ret = key0.value;
			break;
		default:
			// Invalid type
			ASSERT( 0 );
			break;
	}
	return ret;
}

}

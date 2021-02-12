#include "Engine/Common/Common.h"
#include "MaterialAnimation.h"


void MaterialAnimation::AddMemberSet( void )
{
	// the tail element is always current
	m_memberSets.push_back( MemberSet() );
}

void MaterialAnimation::AddMember( void )
{
	GetCurrentMemberSet().members.push_back( Member() );
}


void MaterialAnimation::AddCurve( const MaterialAnimation::Curve& newCurve )
{
	GetCurrentMember().curves.push_back( newCurve );
}


void MaterialAnimation::AddKeyFrame( const usg::exchange::CurveKeyFrame& keyFrame )
{
	GetCurrentCurve().keyFrames.push_back( keyFrame );
}

void MaterialAnimation::FinalizeParsing( void )
{
	m_header.memberSetsNum = m_memberSets.size();

	for( auto& set : m_memberSets )
	{
		for( auto& member : set.members )
		{
			member.data.curveNum = member.curves.size();
			for( auto& curve : member.curves )
			{
				curve.curve.keyFrameNum = curve.keyFrames.size();
			}
		}
	}
}

void MaterialAnimation::Dump()
{
	DEBUG_PRINT( "DUMP=====\n" );
	DEBUG_PRINT( "frameSize: %d\n", m_header.frameSize );
	DEBUG_PRINT( "isLoop: %d\n", m_header.isLoop );
	DEBUG_PRINT( "memberSetsNum: %d\n", m_header.memberSetsNum );

	for( auto set : m_memberSets )
	{
		for( auto member : set.members )
		{
			DEBUG_PRINT( "Member=====\n" );
			DEBUG_PRINT( "targetName: %s\n", member.data.targetName );
			DEBUG_PRINT( "targetID:%d(0x%x)\n", member.data.targetID, member.data.targetID );
			DEBUG_PRINT( "curveNum: %d\n", member.data.curveNum );

			for( auto curve : member.curves )
			{
				DEBUG_PRINT( "Curve=====\n" );
				DEBUG_PRINT( "start: %d, end: %d\n", curve.curve.start, curve.curve.end );
				DEBUG_PRINT( "axis: %d\n", curve.curve.axis );
				DEBUG_PRINT( "keyFrameNum: %d\n", curve.curve.keyFrameNum );

				for( const auto &key : curve.keyFrames )
				{
					DEBUG_PRINT( "Key=====\n" );
					DEBUG_PRINT( "frame: %f, value: %f, type: %d\n", key.frame, key.value, key.type );
					DEBUG_PRINT( "inSlope: %f, outSlope: %f\n", key.inSlope, key.outSlope );
				}
			}
		}
	}

}

MaterialAnimation::MemberSet& MaterialAnimation::GetCurrentMemberSet( void )
{
	ASSERT( m_memberSets.size() > 0 );

	// the tail element is current
	return m_memberSets.back();
}

MaterialAnimation::Member& MaterialAnimation::GetCurrentMember( void )
{
	ASSERT( GetCurrentMemberSet().members.size() > 0 );

	// the tail element is current
	return GetCurrentMemberSet().members.back();
}

MaterialAnimation::Curve& MaterialAnimation::GetCurrentCurve( void )
{
	ASSERT( GetCurrentMember().curves.size() > 0 );

	// the tail element is current
	return GetCurrentMember().curves.back();
}

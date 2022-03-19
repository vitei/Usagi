#include "Engine/Common/Common.h"
#include "Tools/Source/Ayataka/common.h"
#include "MaterialAnimation.h"

namespace exchange
{

	template<typename T>
	void StoreProtocolBuffer(FILE* pHandle, T* pPB, size_t pbSize, const pb_field_t pbFields[])
	{
		void* pBuf = aya::Alloc(pbSize);

		pb_ostream_t streamOut = pb_ostream_from_buffer((uint8_t*)pBuf, pbSize);
		pb_encode(&streamOut, pbFields, pPB);
		fwrite(pBuf, streamOut.bytes_written, 1, pHandle);

		aya::Free(pBuf);
	}

	void StorePBDelimiter(FILE* pHandle, char delimiter)
	{
		fwrite(&delimiter, 1, 1, pHandle);
	}

void MaterialAnimation::SetName(const char* p)
{
	strncpy_s(m_header.name, ARRAY_SIZE(m_header.name), p, strlen(p) + 1);
}


const char* MaterialAnimation::GetName(void)
{
	return m_header.name;
}


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
	m_header.memberSetsNum = (int)m_memberSets.size();

	for( auto& set : m_memberSets )
	{
		for( auto& member : set.members )
		{
			member.data.curveNum = (int)member.curves.size();
			for( auto& curve : member.curves )
			{
				curve.curve.keyFrameNum = (int)curve.keyFrames.size();
			}
		}
	}
}

void MaterialAnimation::Dump()
{
	DEBUG_PRINT( "DUMP=====\n" );
	DEBUG_PRINT( "frameSize: %d\n", m_header.frameCount );
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

void MaterialAnimation::InitTiming(uint32 uFrameCount, float fFrameRate)
{
	m_header.frameRate = fFrameRate;
	m_header.frameCount = uFrameCount;
	//m_animFrames.resize(uFrameCount);
}

void MaterialAnimation::Export(void* pDest, size_t destSize)
{
	// TODO: implment me
	ASSERT(false);
}

size_t MaterialAnimation::GetBinarySize() const
{
	// TODO: implment me
	ASSERT(false);

	return 0;
}


void MaterialAnimation::Export(const char* path)
{
	const char DELIMITER = '\0';
	FILE* handle = fopen(path, "wb");

	StoreProtocolBuffer(handle, &m_header, AnimationHeader_size, usg::exchange::AnimationHeader_fields);
	StorePBDelimiter(handle, DELIMITER);

	for (auto set : m_memberSets)
	{
		usg::exchange::AnimationMemberSet memberSet;
		memberSet.membersNum = (int)set.members.size();
		StoreProtocolBuffer(handle, &memberSet,
			AnimationMemberSet_size,
			usg::exchange::AnimationMemberSet_fields);
		StorePBDelimiter(handle, DELIMITER);

		for (auto member : set.members)
		{
			StoreProtocolBuffer(handle, &member.data,
				AnimationMember_size,
				usg::exchange::AnimationMember_fields);
			StorePBDelimiter(handle, DELIMITER);

			for (auto curve : member.curves)
			{
				StoreProtocolBuffer(handle, &curve.curve,
					AnimationCurve_size,
					usg::exchange::AnimationCurve_fields);
				StorePBDelimiter(handle, DELIMITER);

				for (auto key : curve.keyFrames)
				{
					StoreProtocolBuffer(handle, &key,
						CurveKeyFrame_size,
						usg::exchange::CurveKeyFrame_fields);
					StorePBDelimiter(handle, DELIMITER);
				}
			}
		}
	}

	fclose(handle);
}

void MaterialAnimation::ReverseCoordinate(void)
{
	for (auto& set : m_memberSets)
	{
		for (auto& member : set.members)
		{
			for (auto& curve : member.curves)
			{

				const int ROT_Y = usg::exchange::MaterialAnimationMemberType_ROTATE * 3 + 1;
				const int ROT_Z = usg::exchange::MaterialAnimationMemberType_ROTATE * 3 + 2;
				const int TRANS_X = usg::exchange::MaterialAnimationMemberType_TRANSLATE * 3;

				switch (curve.curve.axis)
				{
				case ROT_Y:
				case ROT_Z:
				case TRANS_X:
					ReverseCurve(curve);
					break;
				}
			}
		}
	}
}

void MaterialAnimation::ReverseCurve(MaterialAnimation::Curve& curve)
{
	for (auto& key : curve.keyFrames)
	{
		key.value *= -1.0f;
		key.inSlope *= -1.0f;
		key.outSlope *= -1.0f;
	}
}

}
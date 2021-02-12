#include "Engine/Common/Common.h"
#include "MaterialAnimationConverter.h"
#include "common.h"

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

void MaterialAnimationConverter::ExportFile( const char* path )
{
	const char DELIMITER = '\0';
	FILE* handle = fopen( path, "wb" );

	StoreProtocolBuffer( handle, &m_materialAnimation.Header(), AnimationHeader_size, usg::exchange::AnimationHeader_fields );
	StorePBDelimiter( handle, DELIMITER );

	for( auto set : m_materialAnimation.GetMemberSetArray() )
	{
		usg::exchange::AnimationMemberSet memberSet;
		memberSet.membersNum = set.members.size();
		StoreProtocolBuffer( handle, &memberSet,
							  AnimationMemberSet_size,
							  usg::exchange::AnimationMemberSet_fields);
		StorePBDelimiter( handle, DELIMITER );

		for( auto member : set.members )
		{
			StoreProtocolBuffer( handle, &member.data,
								  AnimationMember_size,
								  usg::exchange::AnimationMember_fields );
			StorePBDelimiter( handle, DELIMITER );

			for( auto curve : member.curves )
			{
				StoreProtocolBuffer( handle, &curve.curve,
									  AnimationCurve_size,
									  usg::exchange::AnimationCurve_fields );
				StorePBDelimiter( handle, DELIMITER );

				for( auto key: curve.keyFrames )
				{
					StoreProtocolBuffer( handle, &key,
										  CurveKeyFrame_size,
										  usg::exchange::CurveKeyFrame_fields );
					StorePBDelimiter( handle, DELIMITER );
				}
			}
		}
	}
	
	fclose( handle );
}

void MaterialAnimationConverter::ReverseCoordinate( void )
{
	for( auto& set : m_materialAnimation.GetMemberSetArray() )
	{
		for( auto& member : set.members )
		{
			for( auto& curve : member.curves )
			{
				
				const int ROT_Y = usg::exchange::MaterialAnimationMemberType_ROTATE * 3 + 1;
				const int ROT_Z = usg::exchange::MaterialAnimationMemberType_ROTATE * 3 + 2;
				const int TRANS_X = usg::exchange::MaterialAnimationMemberType_TRANSLATE * 3;

				switch( curve.curve.axis )
				{
					case ROT_Y:
					case ROT_Z:
					case TRANS_X:
						ReverseCurve( curve );
					break;
				}
			}
		}
	}
}

void MaterialAnimationConverter::ReverseCurve( MaterialAnimation::Curve& curve )
{
	for( auto& key : curve.keyFrames )
	{
		key.value *= -1.0f;
		key.inSlope *= -1.0f;
		key.outSlope *= -1.0f;
	}
}

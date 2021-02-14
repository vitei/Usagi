/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_CURVE_ANIMATION_RESOURCE_BASE_H_
#define _USG_CURVE_ANIMATION_RESOURCE_BASE_H_

#include "Engine/Resource/ResourceBase.h"
#include "Engine/Scene/Model/MaterialAnimation.pb.h"
#include "Engine/Core/ProtocolBuffers/ProtocolBufferFile.h"
#include "Engine/Core/Utility.h"
#include "Engine/Core/String/U8String.h"

namespace usg {

class CurveAnimResourceBase : public ResourceBase
{
public:
	CurveAnimResourceBase(ResourceType eType) : ResourceBase(eType), m_header(), m_memberSets(NULL) {}
	virtual ~CurveAnimResourceBase() {
		utl::SafeArrayDelete( &m_memberSets );
	}

	bool Load( const char* szName );

	uint32 GetFrameCount( void ) const { return m_header.frameCount; }

	bool IsLoop( void ) const {	return m_header.isLoop;	}

	float GetFrameRate() const { return m_header.frameRate; }

	int GetMemberSetNum() const { return m_header.memberSetsNum; }
	int GetMemberCount(int iSet) const { return m_memberSets[iSet].data.membersNum; }
	const char* GetMemberName(int iSet, int iMember) const { return m_memberSets[iSet].member(iMember)->data.targetName; }

protected:
	template <typename T>
	class Element {
	public:
		Element() {
			ts = NULL;
		}
		virtual ~Element() {
			utl::SafeArrayDelete( &ts );
		}
		void AllocArray( size_t i ) {
			ASSERT( ts == NULL );
			ts = vnew( ALLOC_OBJECT ) T[i];
		}
		const T* array( void ) const {
			return ts;
		}
	protected:
		T* ts;
	};

	// Animation KeyFrame
	class KeyFrame{
	public:
		KeyFrame() {
			exchange::CurveKeyFrame_init( &data );
		}
		exchange::CurveKeyFrame data;
	};

	// Animation Curve
	class Curve : public Element<KeyFrame> {
	public:
		Curve() {
			exchange::AnimationCurve_init( &data );
		}
		exchange::AnimationCurve data;
		KeyFrame* keyFrame( int i ) const {
			ASSERT( 0 <= i && i < data.keyFrameNum );
			return &ts[i];
		}
	};

	// Animation Member
	class Member : public Element<Curve> {
	public:
		Member() {
			exchange::AnimationMember_init( &data );
		}
		exchange::AnimationMember data;
		Curve* curve( int i ) const {
			ASSERT( 0 <= i && i < data.curveNum );
			return &ts[i];
		}
	};

	// Animation MemberSet
	class MemberSet : public Element<Member> {
	public:
		MemberSet() {
			exchange::AnimationMemberSet_init( &data );
		}
		exchange::AnimationMemberSet data;
		Member* member( int i ) const {
			ASSERT( 0 <= i && i < data.membersNum );
			return &ts[i];
		}
	};

	// Functions
	bool LoadMemberSet( ProtocolBufferFile& file, MemberSet* memberSet );
	bool LoadMember( ProtocolBufferFile& file, Member* member );
	bool LoadCurve( ProtocolBufferFile& file, Curve* curve );
	void CollectCurves( const Curve* pCurves[], int curvesCount, Member* pMember ) const;

	float Calc( float frame, const Curve* pCurve ) const;

	exchange::AnimationHeader m_header;
	MemberSet* m_memberSets;
};

}

#endif // _USG_CURVE_ANIMATION_RESOURCE_BASE_H_

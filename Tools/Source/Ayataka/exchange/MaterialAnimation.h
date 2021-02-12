#ifndef TexSrtAnim_h__
#define TexSrtAnim_h__

#include <vector>

#include "OwnSTLDecl.h"
#include "Engine/Scene/Model/MaterialAnimation.pb.h"

class MaterialAnimation
{
public:
	// a curve
	class Curve
	{
	public:
		Curve()
		{
			usg::exchange::AnimationCurve_init( &curve );
		}
		usg::exchange::AnimationCurve curve;

		// keyframes
		typedef std::vector< usg::exchange::CurveKeyFrame, aya::Allocator<usg::exchange::CurveKeyFrame> > KeyFrameArray;
		KeyFrameArray keyFrames;
	};

	// an animation element
	class Member
	{
	public:
		Member()
		{
			usg::exchange::AnimationMember_init( &data );
		}
		usg::exchange::AnimationMember data;

		typedef std::vector< Curve, aya::Allocator<Curve> > CurveArray;
		CurveArray curves;
	};

	// a set of animation elements
	class MemberSet
	{
	public:
		typedef std::vector< Member, aya::Allocator<Member> > MemberArray;
		MemberArray members;
	};
	typedef std::vector< MemberSet, aya::Allocator<MemberSet> > MemberSetArray;

	MaterialAnimation()
	{
		usg::exchange::AnimationHeader_init( &m_header );
	}

	virtual ~MaterialAnimation() {}

	void AddMemberSet( void );
	void AddMember( void );
	void AddCurve( const MaterialAnimation::Curve& newCurve );
	void AddKeyFrame( const usg::exchange::CurveKeyFrame& keyFrame );

	void FinalizeParsing( void );
	void Dump();

	usg::exchange::AnimationHeader& Header( void )
	{
		return m_header;
	}

	MemberSetArray& GetMemberSetArray( void )
	{
		return m_memberSets;
	}

	usg::exchange::AnimationMember& GetCurrentMemberData( void )
	{
		return GetCurrentMember().data;
	}

private:
	MemberSet& GetCurrentMemberSet( void );
	Member& GetCurrentMember( void );
	Curve& GetCurrentCurve( void );
	
	usg::exchange::AnimationHeader	m_header;

	MemberSetArray					m_memberSets;
};

#endif // TexSrtAnim_h__

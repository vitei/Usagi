#ifndef TexPatAnim_h__
#define TexPatAnim_h__

#include "OwnSTLDecl.h"
#include "Engine/Graphics/Animation/TexPatAnim.pb.h"

class TexPatAnim
{
public:
	TexPatAnim() {
		TexPatAnimInfo_init( &mInfo );
	}
	virtual ~TexPatAnim() {}

	void reserveTexPatArraySize( uint32_t length );
	_TexPat& getTexPat( uint32_t index );

	void reserveTexPatAnimArraySize( uint32_t length );
	_TexPatMatAnim& getTexPatMatAnim( uint32_t index );

	_TexPatAnimInfo& pb( void ) { return mInfo; }
	const _TexPatAnimInfo& pb( void ) const { return mInfo; }

private:
	_TexPatAnimInfo mInfo;

	typedef std::vector<_TexPat, aya::Allocator<_TexPat>> TexPatArray;
	TexPatArray mTexPatArray;

	typedef std::vector<_TexPatMatAnim, aya::Allocator<_TexPatMatAnim>> TexPatMatAnimArray;
	TexPatMatAnimArray mTexPatMatAnimArray;
};

#endif // TexPatAnim_h__

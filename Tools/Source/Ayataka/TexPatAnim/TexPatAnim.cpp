#include "TexPatAnim.h"

void TexPatAnim::reserveTexPatArraySize( uint32_t length )
{
	mTexPatArray.resize( length );
	for( uint32_t i = 0; i < length; ++i ) {
		TexPat_init( &mTexPatArray[i] );
	}
}

_TexPat& TexPatAnim::getTexPat( uint32_t index )
{
	return mTexPatArray.at( index );
}

void TexPatAnim::reserveTexPatAnimArraySize( uint32_t length )
{
	mTexPatMatAnimArray.resize( length );
	for( uint32_t i = 0; i < length; ++i ) {
		TexPatMatAnim_init( &mTexPatMatAnimArray[i] );
	}
}

_TexPatMatAnim& TexPatAnim::getTexPatMatAnim( uint32_t index )
{
	return mTexPatMatAnimArray.at( index );
}

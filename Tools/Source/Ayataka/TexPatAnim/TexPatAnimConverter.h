#ifndef TexPatAnimConverter_h__
#define TexPatAnimConverter_h__

#include "TexPatAnim.h"

class TexPatAnimConverter
{
public:
	TexPatAnimConverter() {}
	virtual ~TexPatAnimConverter() {}

	int load( const char* path );

private:
	TexPatAnim mTexPatAnim;
};

#endif // TexPatAnimConverter_h__

#include "TexPatAnimConverter.h"

#include "FtpaLoader.h"

int TexPatAnimConverter::load( const char* path )
{
	FtpaLoader loader;

	int ret = loader.load( mTexPatAnim, path );

	return ret;
}

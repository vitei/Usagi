#ifndef FtpaLoader_h__
#define FtpaLoader_h__

#include "TexPatAnim.h"
#include "pugi_util.h"
#include "common/LoaderBase.h"

class FtpaLoader : public LoaderBase<TexPatAnim>
{
public:
	FtpaLoader() {
	}
	virtual ~FtpaLoader() {}

	int load( TexPatAnim& texPatAnim, const char* path );

private:
	void _loadTexPatAnim( TexPatAnim& texPatAnim, const pugi::xml_document& doc, const char* xpath );
};

#endif // FtpaLoader_h__

#ifndef MaterialAnimationConverter_h__
#define MaterialAnimationConverter_h__

#include "MaterialAnimation.h"

namespace exchange
{

class MaterialAnimationConverter
{
public:
	MaterialAnimationConverter() {}
	virtual ~MaterialAnimationConverter() {}

	template <typename T>
	int Load( const char* path )
	{
		T loader;

		int ret = loader.load( m_materialAnimation, path );

		return ret;
	}

private:
	MaterialAnimation m_materialAnimation;
};

}

#endif // MaterialAnimationConverter_h__

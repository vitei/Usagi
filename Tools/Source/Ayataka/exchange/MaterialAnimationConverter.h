#ifndef MaterialAnimationConverter_h__
#define MaterialAnimationConverter_h__

#include "MaterialAnimation.h"

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
	void ExportFile( const char* path );
	void ReverseCoordinate( void );
	void ReverseCurve( MaterialAnimation::Curve& curve );
private:
	MaterialAnimation m_materialAnimation;
};

#endif // MaterialAnimationConverter_h__

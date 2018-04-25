#ifndef MODELCONVERTERBASE_H
#define MODELCONVERTERBASE_H

#include "cmdl/Cmdl.h"

class DependencyTracker;

class ModelConverterBase
{
public:
	ModelConverterBase();
	virtual ~ModelConverterBase();

	virtual int  Load( const aya::string& path, bool bAsCollision, DependencyTracker* pDependencies ) = 0;
	virtual void Process( void ) = 0;
	void Store( size_t alignment, bool bSwapEndian );
	void StoreCollisionBinary( bool bBigEndian );
	void ExportStoredBinary( const aya::string& path );
	void ExportBoneHierarchy(const aya::string& path);
	void ExportAnimations(const aya::string& path);
	void DumpStoredBinary( void );

	void ReverseCoordinate( void ); // reverse coordinate system between Right-handed and Left-handed
	void CalculatePolygonNormal( void );
	void FlipUV( void );

protected:
	void SetNameFromPath( const char* path );

	Cmdl mCmdl;

	struct StoredBinary {
		void* pBinary;
		void* pHead;
		size_t size;
	};

	enum eSection {
		eSECTION_MAIN,
		eSECTION_NAMES,
		eSECTION_NUM
	};
	StoredBinary mSections[eSECTION_NUM];
};

#endif // MODELCONVERTERBASE_H

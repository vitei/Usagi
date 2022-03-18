#ifndef MODELCONVERTERBASE_H
#define MODELCONVERTERBASE_H

#include "cmdl/Cmdl.h"

class DependencyTracker;

class ModelConverterBase
{
public:
	ModelConverterBase();
	virtual ~ModelConverterBase();

	virtual int  Load( const aya::string& path, bool bAsCollision, bool bSkeletonOnly, DependencyTracker* pDependencies ) = 0;
	virtual void Process( void ) = 0;
	void Store( size_t alignment, bool bSwapEndian );
	void StoreCollisionBinary( bool bBigEndian );
	void ExportStoredBinary( const aya::string& path );
	void ExportStoredBinary(void* pDest, size_t destSize);
	size_t GetBinarySize() const;
	void ExportBoneHierarchy(const aya::string& path);
	void ExportAnimations(const aya::string& path);
	void DumpStoredBinary( void );

	void ReverseCoordinate( void ); // reverse coordinate system between Right-handed and Left-handed
	void CalculatePolygonNormal( void );
	void FlipUV( void );

	uint32 GetAnimationCount() const;
	size_t GetAnimBinarySize(uint32 uAnim) const;
	const char* GetAnimName(uint32 uAnim) const;
	void ExportAnimation(uint32 uAnim, void* pData, size_t destSize);

	std::vector< std::string > GetTextureNames() const;

protected:
	void SetNameFromPath( const char* path );

	Cmdl m_cmdl;

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
	StoredBinary m_sections[eSECTION_NUM];
};

#endif // MODELCONVERTERBASE_H

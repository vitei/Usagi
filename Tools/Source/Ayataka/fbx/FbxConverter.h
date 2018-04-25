#ifndef FBXCONVERTER_H
#define FBXCONVERTER_H

#include "common/ModelConverterBase.h"
#include "cmdl/Cmdl.h"

class FbxConverter : public ModelConverterBase
{
public:
	FbxConverter();
	virtual ~FbxConverter();

	virtual int  Load(const aya::string& path, bool bAsCollision, DependencyTracker* pDependencies);
	void Process( void );

private:
	void _duplicateMeshForShadow( Cmdl & cmdl );

};

#endif // FMDACONVERTER_H

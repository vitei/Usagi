#include <stdlib.h>
#include "OwnSTLDecl.h"

#include "fbx/FbxConverter.h"
#include "MaterialDefinition/MaterialDefinitionExporter.h"
#include "Dependencies/DependencyTracker.h"
#include "Engine/Core/Timer/ProfilingTimer.h"
#include "./StringUtil.h"

#include "common/performance.h"
#include "common/printer.h"

#include "Engine/Memory/Mem.h"

#include <stdlib.h>

struct ConvertSettings
{
	aya::string			dependencyPath;
	aya::string			skeletonPath;
	aya::string			animPath;
	bool				bDependencies;
	int					alignment;
	bool				bBigEndian;
	bool				bLeftHand;
};

bool checkArgument(aya::string& target, const aya::string& argument) {
	if (startsWith(target, argument)) {
		remove(target, 0, argument.length());
		return true;
	}
	else {
		return false;
	}
}

int convertModelData(const aya::string& outputPath, const aya::string& inputPath, const aya::StringVector& argumentsVector, const ConvertSettings& settings)
{
	aya::string shaderOutputDir, effectOutputDir, vsBaseInputPath, psBaseInputPath;
	bool bAsInstance = false;
	bool bAsCollision = false;
	bool bFlipUV = false;
	bool bSkeletonOnly = false;

	int size = (int)argumentsVector.size();
	for (int n = 1; n < size; ++n) {
		aya::string arg = argumentsVector.at(n);

		if (checkArgument(arg, "--sh-out-dir")) {
			if (!endsWith(arg, "/")) {
				arg.append("/");
			}
			shaderOutputDir = arg;
		}
		else if (checkArgument(arg, "--fx-out-dir")) {
			if (!endsWith(arg, "/")) {
				arg.append("/");
			}
			effectOutputDir = arg;
		}
		else if (checkArgument(arg, "--instance")) {
			bAsInstance = true;
		}
		else if (checkArgument(arg, "--collision")) {
			bAsCollision = true;
		}
		else if (checkArgument(arg, "--flip-uv")) {
			bFlipUV = true;
		}
		else if (checkArgument(arg, "--skeleton-only")) {
			bSkeletonOnly = true;
		}
	}

	ModelConverterBase* pConverter = NULL;
	if (endsWith(inputPath, "fbx")) 
	{
		pConverter = vnew(usg::ALLOC_OBJECT) FbxConverter;
	}

	DependencyTracker dependencies;
	if (settings.bDependencies)
	{
		dependencies.StartRecord(settings.dependencyPath.c_str(), outputPath.c_str());
	}

	int ret = pConverter->Load( inputPath, bAsCollision, bSkeletonOnly, &dependencies);
	if( ret != 0 ) {
		SAFE_DELETE( pConverter );
		return ret;
	} 


	if (!bSkeletonOnly)
	{
		if (settings.bDependencies)
		{
			dependencies.ExportDependencies();
		}

		if (bAsCollision) {
			pConverter->CalculatePolygonNormal();
		}

		if (settings.bLeftHand) {
			pConverter->ReverseCoordinate();
		}

		if (bFlipUV) {
			pConverter->FlipUV();
		}

		pConverter->Process();

		if (bAsCollision) {
			pConverter->StoreCollisionBinary(settings.bBigEndian);
		}
		else {
			pConverter->Store(settings.alignment, settings.bBigEndian);
		}

		pConverter->ExportStoredBinary(outputPath);
		pConverter->ExportAnimations(settings.animPath);
	}
	pConverter->ExportBoneHierarchy( settings.skeletonPath );

	SAFE_DELETE( pConverter );

	return 0;
}


int ConvertMaterialDefinition(aya::string outputPath, aya::string inputPath)
{
	MaterialDefinitionExporter exporter;
	exporter.Load(inputPath.c_str());
	exporter.ExportFile(outputPath.c_str());

	return 0;
}


int main(int argc, char *argv[])
{
#ifdef PLATFORM_PC
	usg::mem::InitialiseDefault();
	usg::mem::setConventionalMemManagement( true );
#endif

	//usg::mem::InitialiseDefault();

	perfInit();
	printerInit();

	aya::StringVector argumentsVector;
	for( int i = 0; i < argc; ++i ) {
		argumentsVector.push_back( aya::string( argv[i] ) );
	}

	// analyze arguments
	int size = (int)argumentsVector.size();
	if( size < 2 ) {
		printf( "usage\n" );
		printf( "==========\n" );
		printf( "-a<16|32|64|128>: alignment\n" );
		printf( "-o<path>: output path\n" );
		printf( "-d<path>: dependency path (if blank output file .d is used) \n");
		printf( "-h<path>: bone hierarchy path (if blank output file .xml is used) \n");
		printf( "-lh: swap X-axis for LH coordinate\n" );
		printf( "-be: swap endian\n" );
		printf( "--sh-out-dir<path>: shader output dir (optional)\n" );
		printf( "--fx-out-dir<path>: effect output dir (optional)\n" );
		printf( "--instance: output as an instance\n" );
		printf( "--collision: output as a collision mesh\n" );

		return 0;
	}

	// first of all, have to specify the input and output paths.
	aya::string outputPath, inputPath;
	ConvertSettings settings;
	settings.alignment = DEFAULT_ALIGNMENT;
	settings.bBigEndian = false;
	settings.bLeftHand = false;
	settings.bDependencies = false;
	for( int n = 1; n < size; ++n ) {
		aya::string arg = argumentsVector.at( n );

		if( checkArgument( arg, "-o" ) ) {
			outputPath = arg;
		}
		else if( arg.at(0) != '-' ) {
			// no prefix argument is maybe input path
			inputPath = arg;
		}
		else if (checkArgument(arg, "-d"))
		{
			settings.bDependencies = true;
			settings.dependencyPath = arg;
		}
		else if (checkArgument(arg, "-h"))
		{
			settings.skeletonPath = arg;
		}
		else if( checkArgument( arg, "-be" ) ) {
			settings.bBigEndian = true;
		}
		else if( checkArgument( arg, "-a" ) ) {
			settings.alignment = atoi( arg.c_str() );
		}
		else if( checkArgument( arg, "-lh" ) ) {
			settings.bLeftHand = true;
		}
		else if (checkArgument(arg, "-sk"))
		{
			settings.animPath = arg;
		}
	}

	size_t found = inputPath.find_last_of( "." );
	if( found == aya::string::npos ) {
		// wrong input path
		return -1;
	}
	
	if (settings.dependencyPath.length() == 0)
	{
		settings.dependencyPath = outputPath + ".d";
	}

	if (settings.skeletonPath.length() == 0)
	{
		settings.skeletonPath = outputPath + ".xml";
	}

	aya::string ext = inputPath.substr( found + 1 );

	// convert model data
	int ret = 0;
	if( ext == "cmdl" || ext == "fmda" || ext == "fbx" ) {
		usg::ProfilingTimer timer;
		timer.Start();
		ret = convertModelData( outputPath, inputPath, argumentsVector, settings );
		timer.Stop();
		printf("%f\n", timer.GetTotalMilliSeconds()); 
	}
	else if (ext == "yml")
	{
		ret = ConvertMaterialDefinition(outputPath, inputPath);
	}
	else {
		printf( "That format is not supported!\n" );
		ret = -1;
	}

	printerTerm();
	perfTerm();

	if( ret != 0 ) {
		printf( "ERROR!\n" );
	}
	return ret;
}


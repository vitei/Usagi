#include "FbxConverter.h"

#include "common.h"
//#include "pugi_util.h"

#include "FbxLoad.h"
#include <fbxsdk.h>
#include <fbxsdk/utils/fbxgeometryconverter.h>

class AxisOverride : public FbxAxisSystem
{
public:
	AxisOverride()
	{
		// Matching what comes out of defiances exports
		mUpVector.mAxis = AxisDef::eYAxis;
		mUpVector.mSign = 1;
		mFrontVector.mAxis = AxisDef::eZAxis;
		mFrontVector.mSign = -1;
		mCoorSystem.mAxis = AxisDef::eXAxis;
		mCoorSystem.mSign = -1;
	}
protected:
};

void InitializeSdkObjects(FbxManager*& pManager, FbxScene*& pScene)
{
	//The first thing to do is to create the FBX Manager which is the object allocator for almost all the classes in the SDK
	pManager = FbxManager::Create();
	if (!pManager)
	{
		FBXSDK_printf("Error: Unable to create FBX Manager!\n");
		exit(1);
	}
	else FBXSDK_printf("Autodesk FBX SDK version %s\n", pManager->GetVersion());

	//Create an IOSettings object. This object holds all import/export settings.
	FbxIOSettings* ios = FbxIOSettings::Create(pManager, IOSROOT);
	pManager->SetIOSettings(ios);

	//Load plugins from the executable directory (optional)
	FbxString lPath = FbxGetApplicationDirectory();
	pManager->LoadPluginsDirectory(lPath.Buffer());

	//Create an FBX scene. This object holds most objects imported/exported from/to files.
	pScene = FbxScene::Create(pManager, "My Scene");
	if (!pScene)
	{
		FBXSDK_printf("Error: Unable to create FBX scene!\n");
		exit(1);
	}
}


FbxConverter::FbxConverter()
{
}

FbxConverter::~FbxConverter()
{
}

int FbxConverter::Load(const aya::string& path, bool bAsCollisionModel, bool bSkeletonOnly, DependencyTracker* pDependencies)
{
	FbxManager*		sdkManager;
	FbxScene*		scene;
	FbxImporter*	importer;

	InitializeSdkObjects(sdkManager, scene);
	importer = FbxImporter::Create(sdkManager, "");
	bool bStatus = importer->Initialize(path.c_str());
	 
	if (!bStatus)
	{
		fbxsdk::FbxStatus::EStatusCode eCode = importer->GetStatus().GetCode();
		DEBUG_PRINT("Call to FbxImporter::Initialize() failed.\n");
		DEBUG_PRINT("Error returned: %s\n\n", importer->GetStatus().GetErrorString());
		// open failed
		return -1;
	}

	importer->Import(scene);

	FbxGeometryConverter converter(sdkManager);
	scene->GetGlobalSettings().SetOriginalSystemUnit(FbxSystemUnit::m);
	double scale = scene->GetGlobalSettings().GetSystemUnit().GetConversionFactorTo(FbxSystemUnit::m);
	FbxAxisSystem axisSystem = scene->GetGlobalSettings().GetAxisSystem();
	// To reduce complication I worked with the co-ordinate system from DefinaceIndustries exported models, any model not
	// matching that system I convert. Ayataka always expected RH input which is manually turned to left-handed, the complexity
	// could be reduced further by converting directly with the fbx sdk
	AxisOverride axisOverride;
	axisOverride.ConvertScene(scene);
	FbxSystemUnit::ConversionOptions options = FbxSystemUnit::DefaultConversionOptions;
	// We don't use the FBX convert scene as it messes with scales rather than positions
	//FbxSystemUnit::m.ConvertScene(scene);

	if (!bSkeletonOnly)
	{
		converter.Triangulate(scene, true);
		converter.SplitMeshesPerMaterial(scene, true);
	}

	FbxLoad fbxLoader;
	// Manually converting the scene used the scale instead of adjusting translations directly, so we just apply the scale during conversion
	//FbxAxisSystem::DirectX.ConvertScene(scene);
	fbxLoader.SetAppliedScale(scale);
	fbxLoader.Load( mCmdl, scene, bSkeletonOnly, pDependencies );
	// Because the convert scene updates the scale of bones, not the translation which isn't what we wan

	SetNameFromPath( path.c_str() );

	if (scene)
	{
		scene->Destroy();
	}

	if (sdkManager)
	{
		sdkManager->Destroy();
	}


	return 0;
}

void FbxConverter::Process( void )
{
	// FIXME: Not currently working
//	_duplicateMeshForShadow( mCmdl );
}

void FbxConverter::_duplicateMeshForShadow( Cmdl & cmdl )
{
	int meshNum = cmdl.GetMeshNum();
	for( int i = 0; i < meshNum; ++i ) {
		// TODO
		if( 0 ) { continue; }

		// copy source shape
		::exchange::Mesh* pMesh = cmdl.GetMeshPtr( i );
		::exchange::Shape* pSrcShape = cmdl.GetShapePtr( pMesh->GetShapeRefIndex() );

		// create copy destination shape
		::exchange::Shape* pDestShape = vnew(ALLOC_OBJECT) ::exchange::Shape();

		uint32_t flag = ( 1 << usg::exchange::VertexAttribute_POSITION );
		::exchange::Shape::Copy( *pDestShape, *pSrcShape, flag );

		pMesh->pb().shapeShadowRefIndex = cmdl.GetShapeNum();
		cmdl.AddShape( pDestShape );
	}
}

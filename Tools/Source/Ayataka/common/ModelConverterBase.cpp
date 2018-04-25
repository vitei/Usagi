#include "ModelConverterBase.h"
#include "exchange/Skeleton.h"
#include "common.h"
#include "pugixmlUtil.h"
#include "Engine/Scene/Model/Skeleton.pb.h"

#include "cmdl/CmdlBinaryStore.h"
#include "common/CollisionStore.h"
#include "exchange/LoaderUtil.h"
#include "exchange/Animation.h"
#include "StringUtil.h"

inline void writeTextToFile( const aya::string& path, const aya::string& text )
{
	FILE* fp = fopen( path.c_str(), "wb" );
	if( !fp ) {
		// open failed
		return;
	}
	fwrite( text.c_str(), text.length(), 1, fp );
	fclose( fp );
}

ModelConverterBase::ModelConverterBase()
{
	for( int i = 0; i < eSECTION_NUM; ++i ) {
		mSections[i].pBinary = NULL;
		mSections[i].pHead = NULL;
		mSections[i].size = 0;
	}
}

ModelConverterBase::~ModelConverterBase()
{
	for( int i = 0; i < eSECTION_NUM; ++i ) {
		aya::Free( mSections[i].pBinary );
	}
}

void ModelConverterBase::Store( size_t alignment, bool bSwapEndian )
{
	CmdlBinaryStore cmdlBinaryStore( bSwapEndian );
	size_t binarySize = cmdlBinaryStore.calcStoredBinarySize( mCmdl, alignment );

	// main section
	mSections[eSECTION_MAIN].pBinary = aya::Alloc( binarySize + alignment );
	mSections[eSECTION_MAIN].pHead = offsetAndAlignAddress( mSections[eSECTION_MAIN].pBinary, 0, alignment );
	mSections[eSECTION_MAIN].size= binarySize;

	cmdlBinaryStore.store( mSections[eSECTION_MAIN].pHead, mSections[eSECTION_MAIN].size, mCmdl, alignment );
}

void ModelConverterBase::StoreCollisionBinary(bool bBigEndian)
{
	CollisionStore collisionStore( bBigEndian );
	size_t binarySize = collisionStore.CalcStoredBinarySize( mCmdl );

	// main section
	StoredBinary& mainSec = mSections[eSECTION_MAIN];
	mainSec.pBinary = mainSec.pHead = aya::Alloc( binarySize );
	mainSec.size = binarySize;

	collisionStore.Store( mainSec.pHead, mainSec.size, mCmdl );

	// names section
	binarySize = collisionStore.CalcNamesSectionBinarySize( mCmdl );
	StoredBinary& namesSec = mSections[eSECTION_NAMES];
	namesSec.pBinary = namesSec.pHead = aya::Alloc( binarySize );
	namesSec.size = binarySize;

	collisionStore.StoreNamesSection( namesSec.pHead, namesSec.size, mCmdl );
}

void ModelConverterBase::ExportAnimations(const aya::string& path)
{
	for (uint32 i = 0; i < mCmdl.GetAnimationNum(); i++)
	{
		::exchange::Animation* pAnim = mCmdl.GetAnimation(i);
		aya::string fileName = path + pAnim->GetName();
		fileName += ".vskla";
		pAnim->Export(fileName);
	}

}

void ModelConverterBase::ExportStoredBinary(const aya::string& path)
{
	FILE* fp = fopen( path.c_str(), "wb" );
	if( !fp ) {
		// open failed
		return;
	}

	for( int i = 0; i < eSECTION_NUM; ++i ) {
		if( mSections[i].pBinary ) {
			fwrite( mSections[i].pHead, mSections[i].size, 1, fp );
		}
	}
	fclose( fp );
}

void ModelConverterBase::ExportBoneHierarchy(const aya::string& path)
{
	pugi::xml_document skeletonDocument;
	pugi::xml_node skeletonNode = skeletonDocument.append_child("skeleton");
	::exchange::Skeleton* pSkeleton = mCmdl.GetSkeleton();
	pugi::xml_node boneSet = skeletonNode.append_child("bone_array");
	pugi::xml_attribute arrayLength = boneSet.append_attribute("length");
	arrayLength.set_value((unsigned int)pSkeleton->Bones().size());
	

	pugi::xpath_node_set boneArray;

	for (uint32 i = 0; i < pSkeleton->Bones().size(); i++)
	{
		pugi::xml_node bone = boneSet.append_child("bone");
		usg::exchange::Bone& exBone = pSkeleton->Bones()[i];
		pugi::xml_attribute index = bone.append_attribute("index");
		index.set_value((unsigned int)i);
		pugi::xml_attribute name = bone.append_attribute("name");
		name.set_value(exBone.name);
		pugi::xml_attribute parentName = bone.append_attribute("parent_name");
		parentName.set_value(exBone.parentName);

		pugi::xml_attribute billboard = bone.append_attribute("billboard");
		switch (pSkeleton->Bones()[i].billboardMode)
		{
		case usg::exchange::Bone_BillboardMode_OFF:
			billboard.set_value("none"); break;
		case usg::exchange::Bone_BillboardMode_WORLD:
			billboard.set_value("world"); break;
		case usg::exchange::Bone_BillboardMode_WORLDVIEWPOINT:
			billboard.set_value("worldviewpint"); break;
		case usg::exchange::Bone_BillboardMode_SCREEN:
			billboard.set_value("screen"); break;
		case usg::exchange::Bone_BillboardMode_SCREENVIEWPOINT:
			billboard.set_value("screenviewpoint"); break;
		case usg::exchange::Bone_BillboardMode_YAXIAL:
			billboard.set_value("yaxial"); break;
		case usg::exchange::Bone_BillboardMode_YAXIALVIEWPOINT:
			billboard.set_value("yaxialviewpoint"); break;
		default:
			ASSERT(false);
		};

		char buff[100];
		snprintf(buff, sizeof(buff), "%f %f %f", exBone.translate.x, exBone.translate.y, exBone.translate.z);
		pugi::xml_attribute translate = bone.append_attribute("translate");
		translate.set_value(buff);

		snprintf(buff, sizeof(buff), "%f %f %f %f", exBone.rotate.x, exBone.rotate.y, exBone.rotate.z, 1.0f);
		pugi::xml_attribute rotate = bone.append_attribute("rotate");
		rotate.set_value(buff);

		snprintf(buff, sizeof(buff), "%f %f %f", exBone.scale.x, exBone.scale.y, exBone.scale.z);
		pugi::xml_attribute scale = bone.append_attribute("scale");
		scale.set_value(buff);

	}

	skeletonDocument.save_file(path.c_str());
}

void ModelConverterBase::DumpStoredBinary()
{
	if( !mSections[eSECTION_MAIN].pHead) {
		return;
	}
}


void ModelConverterBase::ReverseCoordinate()
{
	mCmdl.ReverseCoordinate();
}

void ModelConverterBase::CalculatePolygonNormal()
{
	mCmdl.CalculatePolygonNormal();
}

void ModelConverterBase::FlipUV( void )
{
	LoaderUtil::filpUV( mCmdl );
}

void ModelConverterBase::SetNameFromPath(const char *path)
{
	aya::string basename;
	if( contains( path, aya::string( "\\" ) ) ) {
		// windows style
		aya::StringVector list = split( path, aya::string( "\\" ) );
		basename = list.back();
		basename = split( basename, aya::string( "." ) ).at(0);
	}
	else {
		// unix style
		aya::StringVector list = split( path, aya::string( "/" ) );
		basename = list.back();
		basename = split( basename, aya::string( "." ) ).at(0);
	}
	mCmdl.SetName( basename );
}

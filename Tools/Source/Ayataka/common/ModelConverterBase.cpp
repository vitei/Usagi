#include "Engine/Common/Common.h"
#include "ModelConverterBase.h"
#include "exchange/Skeleton.h"
#include "common.h"
#include "pugixmlUtil.h"
#include "Engine/Scene/Model/Skeleton.pb.h"

#include "cmdl/CmdlBinaryStore.h"
#include "common/CollisionStore.h"
#include "exchange/LoaderUtil.h"
#include "exchange/Animation.h"
#include "exchange/MaterialAnimation.h"
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
		m_sections[i].pBinary = NULL;
		m_sections[i].pHead = NULL;
		m_sections[i].size = 0;
	}
}

ModelConverterBase::~ModelConverterBase()
{
	for( int i = 0; i < eSECTION_NUM; ++i ) {
		aya::Free( m_sections[i].pBinary );
	}
}

void ModelConverterBase::Store( size_t alignment, bool bSwapEndian )
{
	CmdlBinaryStore cmdlBinaryStore( bSwapEndian );
	size_t binarySize = cmdlBinaryStore.calcStoredBinarySize( m_cmdl, alignment );

	// main section
	m_sections[eSECTION_MAIN].pBinary = aya::Alloc( binarySize + alignment );
	m_sections[eSECTION_MAIN].pHead = offsetAndAlignAddress( m_sections[eSECTION_MAIN].pBinary, 0, alignment );
	m_sections[eSECTION_MAIN].size= binarySize;

	cmdlBinaryStore.store( m_sections[eSECTION_MAIN].pHead, m_sections[eSECTION_MAIN].size, m_cmdl, alignment );
}

void ModelConverterBase::StoreCollisionBinary(bool bBigEndian)
{
	CollisionStore collisionStore( bBigEndian );
	size_t binarySize = collisionStore.CalcStoredBinarySize( m_cmdl );

	// main section
	StoredBinary& mainSec = m_sections[eSECTION_MAIN];
	mainSec.pBinary = mainSec.pHead = aya::Alloc( binarySize );
	mainSec.size = binarySize;

	collisionStore.Store( mainSec.pHead, mainSec.size, m_cmdl );

	// names section
	binarySize = collisionStore.CalcNamesSectionBinarySize( m_cmdl );
	StoredBinary& namesSec = m_sections[eSECTION_NAMES];
	namesSec.pBinary = namesSec.pHead = aya::Alloc( binarySize );
	namesSec.size = binarySize;

	collisionStore.StoreNamesSection( namesSec.pHead, namesSec.size, m_cmdl );
}

uint32 ModelConverterBase::GetAnimationCount() const
{
	return m_cmdl.GetAnimationNum() + m_cmdl.GetMatAnimationNum();
}

std::string ModelConverterBase::GetAnimName(uint32 uAnim) const
{
	if (uAnim < m_cmdl.GetAnimationNum())
	{
		::exchange::Animation* pAnim = m_cmdl.GetAnimation(uAnim);
		return std::string(pAnim->GetName()) + ".vskla";
	}
	else
	{
		uAnim -= m_cmdl.GetAnimationNum();
		::exchange::MaterialAnimation* pAnim = m_cmdl.GetMatAnimation(uAnim);
		return std::string(pAnim->GetName()) + ".vmata";
	}
}

size_t ModelConverterBase::GetAnimBinarySize(uint32 uAnim) const
{
	if (uAnim < m_cmdl.GetAnimationNum())
	{
		::exchange::Animation* pAnim = m_cmdl.GetAnimation(uAnim);
		return pAnim->GetBinarySize();
	}
	else
	{
		uAnim -= m_cmdl.GetAnimationNum();
		::exchange::MaterialAnimation* pAnim = m_cmdl.GetMatAnimation(uAnim);
		return pAnim->GetBinarySize();
	}
}

void ModelConverterBase::ExportAnimation(uint32 uAnim, void* pData, size_t destSize)
{
	if (uAnim < m_cmdl.GetAnimationNum())
	{
		::exchange::Animation* pAnim = m_cmdl.GetAnimation(uAnim);
		pAnim->Export(pData, destSize);
	}
	else
	{
		uAnim -= m_cmdl.GetAnimationNum();
		::exchange::MaterialAnimation* pAnim = m_cmdl.GetMatAnimation(uAnim);
		return pAnim->Export(pData, destSize);
	}
}

void ModelConverterBase::ExportAnimations(const aya::string& path)
{
	for (uint32 i = 0; i < m_cmdl.GetAnimationNum(); i++)
	{
		::exchange::Animation* pAnim = m_cmdl.GetAnimation(i);
		aya::string fileName = path + pAnim->GetName();
		fileName += ".vskla";
		pAnim->Export(fileName);
	}

	for (uint32 i = 0; i < m_cmdl.GetMatAnimationNum(); i++)
	{
		::exchange::MaterialAnimation* pAnim = m_cmdl.GetMatAnimation(i);
		aya::string fileName = path + pAnim->GetName();
		fileName += ".vmata";
		pAnim->Export(fileName.c_str());
	}

}

void ModelConverterBase::ExportStoredBinary(const aya::string& path)
{
	FILE* fp = fopen( path.c_str(), "wb" );
	if( !fp ) {
		// open failed
		return;
	}

	for( int i = 0; i < eSECTION_NUM; ++i )
	{
		if( m_sections[i].pBinary )
		{
			fwrite( m_sections[i].pHead, m_sections[i].size, 1, fp );
		}
	}
	fclose( fp );
}

const char* ModelConverterBase::GetTextureFormat(std::string texName)
{
	for (uint32 i = 0; i < m_cmdl.GetMaterialNum(); i++)
	{
		for (uint32 uTex = 0; uTex < usg::exchange::Material::textures_max_count; uTex++)
		{
			if (m_cmdl.GetMaterialPtr(i)->pb().textures[uTex].textureName[0] != '\0')
			{
				std::string name = m_cmdl.GetMaterialPtr(i)->pb().textures[uTex].textureName;
				if (name == texName)
				{
					const char* szHint = "BC3-srgb";
					const char* szName = m_cmdl.GetMaterialPtr(i)->pb().textures[uTex].textureHint;
					if (strcmp(szName, "NormalMap") == 0)
					{
						szHint = "BC5";
					}
					else if (strcmp(szName, "SpecularColor") == 0)
					{
						szHint = "BC3";
					}
					return szHint;
				}
			}
		}
	}
	return "BC3-srgb";
}

std::vector< std::string > ModelConverterBase::GetTextureNames() const
{
	std::vector< std::string > namesOut;
	for (uint32 i = 0; i < m_cmdl.GetMaterialNum(); i++)
	{
		for (uint32 uTex = 0; uTex < usg::exchange::Material::textures_max_count; uTex++)
		{
			if (m_cmdl.GetMaterialPtr(i)->pb().textures[uTex].textureName[0] != '\0')
			{
				std::string name = m_cmdl.GetMaterialPtr(i)->pb().textures[uTex].textureName;
				if (std::find(namesOut.begin(), namesOut.end(), name) == namesOut.end())
				{
					namesOut.push_back(name);
				}
			}
		}
	}
	return namesOut;
}

void ModelConverterBase::ExportStoredBinary(void* pDest, size_t destSize)
{
	ASSERT(destSize >= GetBinarySize());

	uint8* pCurr = (uint8*)pDest;
	for (int i = 0; i < eSECTION_NUM; ++i)
	{
		if (m_sections[i].pBinary)
		{
			memcpy(pCurr, m_sections[i].pHead, m_sections[i].size);
			pCurr += m_sections[i].size;
		}
	}
}

size_t ModelConverterBase::GetBinarySize() const
{
	size_t size = 0;
	for (int i = 0; i < eSECTION_NUM; ++i)
	{
		if (m_sections[i].pBinary)
		{
			size += m_sections[i].size;
		}
	}
	return size;
}

void ModelConverterBase::ExportBoneHierarchy(const aya::string& path)
{
	pugi::xml_document skeletonDocument;
	pugi::xml_node hierarchyNode = skeletonDocument.append_child("hierarchy");
	::exchange::Skeleton* pSkeleton = m_cmdl.GetSkeleton();
	pugi::xml_node boneSet = hierarchyNode.append_child("bone_array");
	pugi::xml_attribute arrayLength = boneSet.append_attribute("length");
	arrayLength.set_value((unsigned int)pSkeleton->Bones().size());
	

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

		pugi::xml_attribute neededRendering = bone.append_attribute("needed_rendering");
		neededRendering.set_value(exBone.isNeededRendering ? "true" : "false" );

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

	if(m_cmdl.GetLightNum() > 0)
	{
	//	pugi::xml_node lightNode = skeletonDocument.append_child("lighting");
		pugi::xml_node lightSet = hierarchyNode.append_child("light_array");

		pugi::xml_attribute lightArrayLength = lightSet.append_attribute("length");
		lightArrayLength.set_value((unsigned int)m_cmdl.GetLightNum());

		for (uint32 i = 0; i < m_cmdl.GetLightNum(); i++)
		{
			pugi::xml_node light = lightSet.append_child("light");
			const Cmdl::Light* pLight = m_cmdl.GetLight(i);

			pugi::xml_attribute name = light.append_attribute("name");
			name.set_value(pLight->name.c_str());
			pugi::xml_attribute parentName = light.append_attribute("parent_name");
			parentName.set_value(pLight->parentBone.c_str());

			pugi::xml_attribute type = light.append_attribute("type");
			switch (pLight->spec.base.kind)
			{
			case LightKind_DIRECTIONAL:
				type.set_value("directional");
				break;
			case LightKind_SPOT:
				type.set_value("spot");
				break;
			case LightKind_POINT:
				type.set_value("point");
				break;
			case LightKind_PROJECTION:
				type.set_value("projection");
				break;
			}

			char buff[100];
			snprintf(buff, sizeof(buff), "%f %f %f", pLight->position.x, pLight->position.y, pLight->position.z);
			pugi::xml_attribute position = light.append_attribute("position");
			position.set_value(buff);

			snprintf(buff, sizeof(buff), "%f %f %f", pLight->spec.direction.x, pLight->spec.direction.y, pLight->spec.direction.z);
			pugi::xml_attribute direction = light.append_attribute("direction");
			direction.set_value(buff);

			pugi::xml_attribute shadow = light.append_attribute("has_shadow");
			shadow.set_value(pLight->spec.base.bShadow);

			pugi::xml_attribute ambient = light.append_attribute("ambient");
			pugi::xml_attribute diffuse = light.append_attribute("diffuse");
			pugi::xml_attribute specular = light.append_attribute("specular");
			snprintf(buff, sizeof(buff), "%.2f %.2f %.2f", pLight->spec.base.ambient.r(), pLight->spec.base.ambient.g(), pLight->spec.base.ambient.b());
			ambient.set_value(buff);
			snprintf(buff, sizeof(buff), "%.2f %.2f %.2f", pLight->spec.base.diffuse.r(), pLight->spec.base.diffuse.g(), pLight->spec.base.diffuse.b());
			diffuse.set_value(buff);
			snprintf(buff, sizeof(buff), "%.2f %.2f %.2f", pLight->spec.base.specular.r(), pLight->spec.base.specular.g(), pLight->spec.base.specular.b());
			specular.set_value(buff);


			pugi::xml_attribute attenEnabled = light.append_attribute("atten_enabled");
			pugi::xml_attribute attenStart = light.append_attribute("atten_start");
			pugi::xml_attribute attenEnd = light.append_attribute("atten_end");
			attenEnabled.set_value(pLight->spec.atten.bEnabled);
			attenStart.set_value(pLight->spec.atten.fNear);
			attenEnd.set_value(pLight->spec.atten.fFar);

			pugi::xml_attribute inner_angle = light.append_attribute("inner_angle");
			inner_angle.set_value(pLight->spec.spot.fInnerAngle);

			pugi::xml_attribute outer_angle = light.append_attribute("outer_angle");
			outer_angle.set_value(pLight->spec.spot.fOuterAngle);
		}
	}

	if (m_cmdl.GetCameraNum() > 0)
	{
		//	pugi::xml_node lightNode = skeletonDocument.append_child("lighting");
		pugi::xml_node cameraSet = hierarchyNode.append_child("camera_array");

		pugi::xml_attribute cameraArrayLength = cameraSet.append_attribute("length");
		cameraArrayLength.set_value((unsigned int)m_cmdl.GetCameraNum());

		for (uint32 i = 0; i < m_cmdl.GetCameraNum(); i++)
		{
			pugi::xml_node camera = cameraSet.append_child("camera");
			const Cmdl::Camera* pCamera = m_cmdl.GetCamera(i);

			pugi::xml_attribute name = camera.append_attribute("name");
			name.set_value(pCamera->name.c_str());
			pugi::xml_attribute parentName = camera.append_attribute("parent_name");
			parentName.set_value(pCamera->parentBone.c_str());

			char buff[100];
			snprintf(buff, sizeof(buff), "%f %f %f", pCamera->position.x, pCamera->position.y, pCamera->position.z);
			pugi::xml_attribute position = camera.append_attribute("position");
			position.set_value(buff);

			snprintf(buff, sizeof(buff), "%f %f %f", pCamera->rotate.x, pCamera->rotate.y, pCamera->rotate.z);
			pugi::xml_attribute rotate = camera.append_attribute("rotate");
			rotate.set_value(buff);

			pugi::xml_attribute fov = camera.append_attribute("fov");
			fov.set_value(pCamera->fov);

			pugi::xml_attribute nearPlane = camera.append_attribute("near");
			nearPlane.set_value(pCamera->nearPlane);

			pugi::xml_attribute farPlane = camera.append_attribute("far");
			farPlane.set_value(pCamera->farPlane);
		}
	}


	skeletonDocument.save_file(path.c_str());
}

void ModelConverterBase::DumpStoredBinary()
{
	if( !m_sections[eSECTION_MAIN].pHead) {
		return;
	}
}


void ModelConverterBase::ReverseCoordinate()
{
	m_cmdl.ReverseCoordinate();
}

void ModelConverterBase::CalculatePolygonNormal()
{
	m_cmdl.CalculatePolygonNormal();
}

void ModelConverterBase::FlipUV( void )
{
	LoaderUtil::filpUV( m_cmdl );
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
	m_cmdl.SetName( basename );
}

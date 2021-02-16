#include "Engine/Common/Common.h"
#include "FbxLoad.h"
#include "common.h"
//#include "pugi_util.h"
#include "Engine/Core/stl/map.h"
#include "StringUtil.h"
#include "exchange/MaterialAnimation.h"
#include "exchange/Animation.h"
#include "exchange/LoaderUtil.h"

static const FbxTime::EMode FRAME_MODE = FbxTime::eFrames30;

Matrix4x4 ToMat4x4(FbxAMatrix in)
{
	Matrix4x4 mMatUsg;
	for (uint32 i = 0; i < 4; i++)
	{
		for (uint32 j = 0; j < 4; j++)
		{
			mMatUsg.M[i][j] = (float)in.Get(i, j);
		}
	}
	return mMatUsg;
}

FbxAMatrix ToFbxMat(Matrix4x4 in)
{
	FbxAMatrix mRet;
	for (uint32 i = 0; i < 4; i++)
	{
		for (uint32 j = 0; j < 4; j++)
		{
			mRet[i][j] = in.M[i][j];
		}
	}
	return mRet;
}

Vector4f ToVec4(FbxVector4 in)
{
	return Vector4f((float)in[0], (float)in[1], (float)in[2], (float)in[3]);
}

FbxLoad::FbxLoad()
{
	m_uMeshMaterialOffset = 0;
	m_bHasNormalMap = false;
	m_bHasDefaultStaticBone = false;
	m_pParentBoneNode = nullptr;
	m_appliedScale = 1.0;
	m_fAttenScale = 1.0;
}

void FbxLoad::AddIdentityBone(::exchange::Skeleton* pSkeleton)
{
	const char* pRootBoneName = m_pScene->GetName();
	usg::exchange::Skeleton& rSkeleton = pSkeleton->pb();
	STRING_COPY(rSkeleton.rootBoneName, pRootBoneName);
	rSkeleton.bonesNum = 1;

	usg::exchange::Bone bone;
	usg::exchange::Bone_init(&bone);
	bone.isNeededRendering = false;

	// Transform
	bone.scale.Assign(1.0f, 1.0f, 1.0f);
	bone.rotate.Clear();
	bone.translate.Clear();
	LoaderUtil::setupTransformMatrix(bone.transform, bone.scale, bone.rotate, bone.translate);

	// Name
	STRING_COPY(bone.name, pRootBoneName);
	memset(bone.parentName, 0, sizeof(bone.parentName));
	m_bHasDefaultStaticBone = true;

	pSkeleton->Bones().push_back(bone);

	FbxAMatrix Identity; 
	Identity.SetIdentity();
	m_globalBonePoses.push_back(Identity);
}


void FbxLoad::AddCamera(Cmdl& cmdl, FbxNode* pNode)
{
	FbxCamera* pFBXCamera = pNode->GetCamera();
	if (!pFBXCamera)
		return;

	Cmdl::Camera* pCamera = vnew(ALLOC_OBJECT) Cmdl::Camera();

	FbxNode* pParentBone = pNode->GetParent();
	FbxNodeAttribute::EType attribType = pParentBone && pParentBone->GetNodeAttribute() ? pParentBone->GetNodeAttribute()->GetAttributeType() : FbxNodeAttribute::EType::eUnknown;
	while (pParentBone && 
		!(attribType == FbxNodeAttribute::eSkeleton || attribType == FbxNodeAttribute::eNull || attribType == FbxNodeAttribute::eMarker) )
	{
		pParentBone = pParentBone->GetParent();
		attribType = (pParentBone && pParentBone->GetNodeAttribute() ? pParentBone->GetNodeAttribute()->GetAttributeType() : FbxNodeAttribute::EType::eUnknown);
	}

	if (pParentBone &&
		(attribType == FbxNodeAttribute::eSkeleton || attribType == FbxNodeAttribute::eNull || attribType == FbxNodeAttribute::eMarker)  )
	{
		const char* pBoneName = pParentBone->GetName();
		pCamera->parentBone = pBoneName;
	}
	else
	{
		pCamera->parentBone = cmdl.GetSkeleton()->pb().rootBoneName;
	}


	FbxAMatrix globalPoseMatrix = GetGlobalPoseMatrix(pNode);

	FbxVector4 position = pFBXCamera->EvaluatePosition();//pFBXCamera->Position.Get();
	FbxVector4 interestPos = pFBXCamera->EvaluateLookAtPosition();//pFBXCamera->InterestPosition.Get();
	FbxVector4 up = pFBXCamera->EvaluateUpDirection(position, interestPos);
	FbxVector4 forward = interestPos - position;
	up[3] = 0.0f; forward[3] = 0.0f;
	forward.Normalize();

	// TODO: Handle roll
	Matrix4x4 cameraMat;
	FbxVector4 right = forward.CrossProduct(up);
	right[3] = 0.0f;
	cameraMat.ModelMatrix(ToVec4(right), ToVec4(up), ToVec4(forward), ToVec4(position));
	globalPoseMatrix = ToFbxMat(cameraMat);

	globalPoseMatrix = GetGlobalPoseMatrix(pNode);
	FbxAMatrix rot;
	rot.SetR(FbxVector4(0.0f, 90.0f, 0.0f));
	globalPoseMatrix = globalPoseMatrix * rot;
	FbxAMatrix localPoseMatrix = GetLocalPoseMatrix(cmdl, globalPoseMatrix, pCamera->parentBone.c_str());



	FbxVector4 rotate = localPoseMatrix.GetR();
	FbxVector4 translate = localPoseMatrix.GetT();

	FbxCamera::EApertureMode eApMode = pFBXCamera->GetApertureMode();
	pCamera->name = pFBXCamera->GetName();
	pCamera->fov = (real)pFBXCamera->FieldOfViewY.Get();
	pCamera->nearPlane = (real)(pFBXCamera->GetNearPlane() * m_appliedScale);
	pCamera->farPlane = (real)(pFBXCamera->GetFarPlane() * m_appliedScale);
	pCamera->rotate.Assign(Math::DegreesToRadians((float)rotate[0]), Math::DegreesToRadians((float)rotate[1]), Math::DegreesToRadians((float)rotate[2]));
	pCamera->position.Assign((float)(translate[0] * m_appliedScale), (float)(translate[1] * m_appliedScale), (float)(translate[2] * m_appliedScale));

	cmdl.AddCamera(pCamera);

}

void FbxLoad::AddLight(Cmdl& cmdl, FbxNode* pNode)
{
	FbxLight* pFBXLight = pNode->GetLight();

	Cmdl::Light* pLight = vnew(ALLOC_OBJECT) Cmdl::Light();

	usg::LightSpec_init(&pLight->spec);

	FbxNode* pParentBone = pNode;
	while (pParentBone && !IsBone(pParentBone))
	{
		pParentBone = pParentBone->GetParent();
	}

	if (pParentBone && IsBone(pParentBone))
	{
		const char* pBoneName = pParentBone->GetName();
		pLight->parentBone = pBoneName;
	}
	else
	{
		pLight->parentBone = cmdl.GetSkeleton()->pb().rootBoneName;
	}

	FbxAMatrix mGeometry;
	Matrix4x4 mMatUsg;
	mGeometry = GetCombinedMatrixForNode(pNode, pParentBone);//pNode->EvaluateLocalTransform(FBXSDK_TIME_INFINITE);//GetCombinedMatrixForNode(pNode);

	// Undo the root transform that conversion puts in
	{
		FbxNode* Root = nullptr;
		FbxNode* Tmp = pNode;
		while (Tmp->GetParent())
		{
			if (IsBone(Tmp))
			{
				Root = Tmp;
			}
			Tmp = Tmp->GetParent();
		}
	
		if (Root)
		{
			FbxAMatrix RootPose = Root->EvaluateGlobalTransform();;
			RootPose.SetT(FbxVector4(0.0f, 0.f, 0.f, 1.f));
			mGeometry = mGeometry * RootPose.Inverse();
		}
	}

	for (uint32 i = 0; i < 4; i++)
	{
		for (uint32 j = 0; j < 4; j++)
		{
			mMatUsg.M[i][j] = (float)mGeometry.Get(i, j);
		}
	}
	
	pLight->name = pFBXLight->GetName();

	switch (pFBXLight->LightType.Get())
	{
	case FbxLight::eSpot:
		pLight->spec.base.kind = usg::LightKind_SPOT;
		pLight->spec.spot.fInnerAngle = (float)pFBXLight->InnerAngle.Get();
		pLight->spec.spot.fOuterAngle = (float)pFBXLight->OuterAngle.Get();
		pLight->spec.direction = mMatUsg.vFace().v3().GetNormalised();
		pLight->position = mMatUsg.vPos().v3() * (float)m_appliedScale;
		break;
	//case FbxLight::eDirectional:
//		pLight->spec.base.kind = usg::LightKind_DIRECTIONAL;
	//	pLight->spec.direction = mMatUsg.vFace().v3().GetNormalised();
//		break;
	case FbxLight::ePoint:
		pLight->spec.base.kind = usg::LightKind_POINT;
		pLight->position = mMatUsg.vPos().v3() * (float)m_appliedScale;
		break;
	default:
		// Unhandled
		delete pLight;
		return;
	}

	float fIntensity = 1.0f;// pFBXLight->Intensity.Get() / 100.0f;
	usg::Color color((float)pFBXLight->Color.Get().mData[0], (float)pFBXLight->Color.Get().mData[1],
		(float)pFBXLight->Color.Get().mData[2]);
	pLight->spec.base.ambient = color * fIntensity * 0.2f;
	pLight->spec.base.diffuse = color * fIntensity * 1.0f;
	pLight->spec.base.specular = color * fIntensity * 1.0f;
	pLight->spec.base.bShadow = pFBXLight->CastShadows;


	// Matching our simplified settings
	// TODO: We should really should support these falloff settings in the engine
	float fFarEnd = 100.f;
	float fAttenuationStart = 0.0f;
	if (pFBXLight->EnableNearAttenuation.Get())
	{
		fAttenuationStart = (float)pFBXLight->NearAttenuationStart;
	}
	if (pFBXLight->EnableFarAttenuation.Get()) 
	{
		fFarEnd = (float)pFBXLight->FarAttenuationEnd.Get();
	}
	else
	{
		// Calculate the point where the light drops to near zero

		fAttenuationStart = usg::Math::Max((float)pFBXLight->DecayStart.Get(), fAttenuationStart);

		// Light will be culled after it should drop to 1/fFarIntensityFrac
		const float fFarIntensityFrac = 100.f;	
		switch (pFBXLight->DecayType.Get()) {
		case FbxLight::eLinear:
			fFarEnd = fFarIntensityFrac *fIntensity;
			break;
		case FbxLight::eQuadratic:
			fFarEnd = fFarIntensityFrac *sqrtf(fIntensity);
			break;
		case FbxLight::eCubic:
			fFarEnd = pow(fFarIntensityFrac*fIntensity, 1.0f / 3.0f);
			break;
		case FbxLight::eNone:
			// We don't support no attenuation; set range to 1.0f by default
			fFarEnd = 1000.0f;
			break;
		}

		fFarEnd += fAttenuationStart;
	}

	pLight->spec.atten.bEnabled = pFBXLight->LightType.Get() != FbxLight::eDirectional;
	pLight->spec.atten.fNear = fAttenuationStart * (float)m_fAttenScale;
	pLight->spec.atten.fFar = fFarEnd * (float)m_fAttenScale;

	cmdl.AddLight(pLight);

}


FbxAMatrix FbxLoad::GetGlobalPoseMatrix(FbxNode* pNode)
{
	// First try and find a bind pose matrix for the bone
	bool bFoundBindMatrix = false;
	bool bBindFromPose = false;
	FbxAMatrix globalPoseMatrix;
	FbxAMatrix globalTransform = pNode->EvaluateGlobalTransform();
	for (uint32 uPose = 0; uPose < m_bindPoses.size(); uPose++)
	{
		FbxPose* pPose = m_bindPoses[uPose];
		int nodeIndex = pPose->Find(pNode);
		if (nodeIndex >= 0)
		{
			FbxMatrix poseMatrix = pPose->GetMatrix(nodeIndex);
			FbxAMatrix poseMatrixA = *(FbxAMatrix*)&poseMatrix;

			globalPoseMatrix = poseMatrixA;
			bFoundBindMatrix = true;
			break;
		}
	}

	// Next try and get the bind matrix from the clusters
	if (!bFoundBindMatrix)
	{
		for (uint32 uCluster = 0; uCluster < m_clusters.size(); uCluster++)
		{
			FbxCluster* Cluster = m_clusters[uCluster];
			if (Cluster->GetLink() == pNode)
			{
				Cluster->GetTransformLinkMatrix(globalPoseMatrix);
				bFoundBindMatrix = true;
				break;
			}
		}
	}

	// Fallback
	if (!bFoundBindMatrix)
	{
		globalPoseMatrix = pNode->EvaluateGlobalTransform();
	}

	return globalPoseMatrix;
}

FbxAMatrix FbxLoad::GetLocalPoseMatrix(Cmdl& cmdl, FbxAMatrix globalPose, const char* szParentName)
{
	if (!m_bHasDefaultStaticBone)
	{
		uint32 boneIndex = FindBone(*cmdl.GetSkeleton(), szParentName);
		if (boneIndex != USG_INVALID_ID)
		{
			return m_globalBonePoses[boneIndex].Inverse() * globalPose;
		}
	}
	return globalPose;
}
void FbxLoad::AddBone(::exchange::Skeleton* pSkeleton, FbxNode* pNode, bool bIsNeededRendering)
{
	usg::exchange::Skeleton& rSkeleton = pSkeleton->pb();
	const char* pBoneName = pNode->GetName();
	// Remove duplicate names
	int loopIdx = 0;
	char name[256];
	strcpy_s(name, pNode->GetName());
	bool nameUnique = true;
	do
	{
		nameUnique = true;
		for (size_t i = 0; i < pSkeleton->Bones().size(); i++)
		{
			if (strcmp(pSkeleton->Bones()[i].name, name) == 0)
			{
				nameUnique = false;
				break;
			}
		}
		if (!nameUnique)
		{
			sprintf_s(name, "%s.%03i", pNode->GetName(), ++loopIdx);
		}
	} while (!nameUnique);

	pNode->SetName(name);
	pBoneName = pNode->GetName();

	FbxNode* pParent = pNode->GetParent();
	while (pParent && !IsBone(pParent))
	{
		pParent = pParent->GetParent();
	}

	if (!pParent && !m_bHasDefaultStaticBone)
	{
		STRING_COPY(rSkeleton.rootBoneName, pBoneName);
	}
	rSkeleton.bonesNum++;

	usg::exchange::Bone bone;
	usg::exchange::Bone_init(&bone);
	bone.isNeededRendering = bIsNeededRendering;

	

	FbxAMatrix globalPoseMatrix = GetGlobalPoseMatrix(pNode);
	FbxAMatrix localPoseMatrix = globalPoseMatrix;


	if (pParent)
	{
		int iParentIdx = FindBone(*pSkeleton, pParent->GetName());
		// Name
		STRING_COPY(bone.parentName, pParent->GetName());
		localPoseMatrix = m_globalBonePoses[iParentIdx].Inverse() * localPoseMatrix;
		if (pNode->GetChildCount() == 0 && !bIsNeededRendering)
		{
			FbxNode* Root = nullptr;
			FbxNode* Tmp = pNode;
			while (Tmp->GetParent())
			{
				if (IsBone(Tmp))
				{
					Root = Tmp;
				}
				Tmp = Tmp->GetParent();
			}
			// Convenience hack for markers to match the orientation
			if (Root)
			{
				FbxAMatrix RootPose = Root->EvaluateGlobalTransform();;
				RootPose.SetT(FbxVector4(0.0f, 0.f, 0.f, 1.f));
				localPoseMatrix = localPoseMatrix * RootPose.Inverse();
			}
		}
		bone.parentIndex = iParentIdx;
	}
	else
	{
		if (m_bHasDefaultStaticBone)
		{
			STRING_COPY(bone.parentName, rSkeleton.rootBoneName);
		}
		else
		{
			memset(bone.parentName, 0, sizeof(bone.parentName));
		}
	}

	FbxVector4 scale = localPoseMatrix.GetS();
	FbxVector4 rotate = localPoseMatrix.GetR();
	FbxVector4 translate = localPoseMatrix.GetT();

	// FIXME: For some reason the conversion unit doesn't seem to impact the bind pose so we have to do that manually
	// Transform
	bone.scale.Assign((float)(scale[0]), (float)(scale[1]), (float)(scale[2]));
	bone.rotate.Assign(Math::DegreesToRadians((float)rotate[0]), Math::DegreesToRadians((float)rotate[1]), Math::DegreesToRadians((float)rotate[2]));
	bone.translate.Assign((float)(translate[0] * m_appliedScale), (float)(translate[1] * m_appliedScale), (float)(translate[2] * m_appliedScale));

	LoaderUtil::setupTransformMatrix(bone.transform, bone.scale, bone.rotate, bone.translate);

	// Name
	STRING_COPY(bone.name, pBoneName);


	pSkeleton->Bones().push_back(bone);
	// Store this matrix so we can calculate the local pose on our children
	m_globalBonePoses.push_back(globalPoseMatrix);
}

::exchange::Skeleton* FbxLoad::NewSkeleton()
{
	::exchange::Skeleton* pNewSkeleton = vnew(ALLOC_OBJECT) ::exchange::Skeleton();

	return pNewSkeleton;
}

::exchange::Shape* FbxLoad::NewShape(Cmdl& cmdl, FbxNode* shapeNode)
{
	::exchange::Shape* pNewShape = vnew(ALLOC_OBJECT) ::exchange::Shape();


	return pNewShape;
}

::exchange::Mesh* FbxLoad::NewMesh(Cmdl& cmdl, FbxNode* shapeNode)
{
	::exchange::Mesh* pNewMesh = vnew(ALLOC_OBJECT) ::exchange::Mesh();

	// mesh's name
	const char* pMeshName = shapeNode->GetName();
	pNewMesh->SetName(pMeshName);

	// shape reference index
	pNewMesh->SetShapeRefIndex(cmdl.GetShapeNum() - 1);

	// material reference name
	const char* pMaterialName = cmdl.GetMaterialPtr(m_uMeshMaterialTmp)->pb().materialName;
	pNewMesh->SetMaterialRefName(pMaterialName);

	pNewMesh->SetMaterialRefIndex(m_uMeshMaterialTmp);

	return pNewMesh;
}


bool FbxLoad::GetTextureIndex(const FbxTexture& textureInfo, const char* szTexName, ::exchange::Material* pMaterial, uint32& uIndex)
{
	std::string textureType = textureInfo.GetName();

	if (!pMaterial->IsCustomFX())
	{
		return pMaterial->GetCustomFX(0).GetTextureIndex(szTexName, uIndex) && pMaterial->pb().textures[uIndex].textureName[0] == '\0';
	}

	return pMaterial->GetCustomFX(0).GetTextureIndex(szTexName, uIndex) && pMaterial->pb().textures[uIndex].textureName[0] == '\0';
}


usg::SamplerWrap GetWrap(FbxTexture::EWrapMode WrapMode)
{
	switch (WrapMode)
	{
	case FbxTexture::EWrapMode::eRepeat:
		return usg::SAMP_WRAP_REPEAT;
	case FbxTexture::EWrapMode::eClamp:
		return usg::SAMP_WRAP_CLAMP;
	default:
		return usg::SAMP_WRAP_REPEAT;
	}
}

void FbxLoad::AddMaterialTextures(FbxSurfaceMaterial* pFBXMaterial, ::exchange::Material* pNewMaterial)
{
	FbxProperty property = pFBXMaterial->GetFirstProperty();

	if (pFBXMaterial->sNormalMap || pFBXMaterial->sBump)
	{
		m_bHasNormalMap = true;
	}

	while (property.IsValid())
	{
		uint32 uTextureCount = property.GetSrcObjectCount<FbxTexture>();
		const char* szName = property.GetNameAsCStr();
		for (uint32 i = 0; i < uTextureCount; ++i)
		{
			FbxLayeredTexture* layeredTexture = property.GetSrcObject<FbxLayeredTexture>(i);

			ASSERT_MSG((layeredTexture == nullptr), "Layered texture not supported");

			FbxTexture* pTexture = property.GetSrcObject<FbxTexture>(i);
			if (pTexture)
			{
				uint32 uTexIndex = 0;
				FbxFileTexture* fileTexture = FbxCast<FbxFileTexture>(pTexture);
				if (GetTextureIndex(*pTexture, property.GetNameAsCStr(), pNewMaterial, uTexIndex))
				{
					std::string textName = fileTexture->GetRelativeFileName();
					const char* szExt = strrchr(textName.c_str(), '.');
					int length = (int)(szExt != nullptr ? szExt - textName.c_str() : strlen(textName.c_str()));
					textName = textName.substr(0, length);
					const char* drive = strrchr(textName.c_str(), ':');

					//if (drive != nullptr)
					{
						// We just ignore directories for our models textures
						size_t pos = textName.find_last_of("\\/");
						// Relative path is absolute :(
						if (pos != std::string::npos)
						{
							textName = textName.substr(pos+1);
						}
					}
					 
					if (textName.size() >= sizeof(pNewMaterial->pb().textures[uTexIndex].textureName))
					{
						printf("Texture name %s too long\n", textName.c_str());
						continue;
					}

					strncpy_s(pNewMaterial->pb().textures[uTexIndex].textureName, textName.c_str(), sizeof(pNewMaterial->pb().textures[uTexIndex].textureName));
					
					uint32 uCoordinatorIndex = pNewMaterial->pb().textureCoordinators_count;
					std::string textureType = property.GetNameAsCStr();

					usg::exchange::Texture& tex = pNewMaterial->pb().textures[uTexIndex];

					tex.wrapS = GetWrap(pTexture->WrapModeU);
					tex.wrapT = GetWrap(pTexture->WrapModeV);

					// Now relying on the custom effect for these values
					//tex.magFilter = usg::SAMP_FILTER_LINEAR;
					//tex.mipFilter = usg::MIP_FILTER_LINEAR;
					//tex.minFilter = usg::SAMP_FILTER_LINEAR;
					//tex.lodBias = 0.0f;
					//tex.lodMinLevel = 0;
					//tex.anisoLevel = usg::ANISO_LEVEL_16;	// Assume max as nothing has been asked for
					strcpy_s(tex.textureHint, sizeof(tex.textureHint), property.GetNameAsCStr());

					
					//tex.minFilter = fileTexture->UseMipMap ? usg::exchange::Texture_Filter_linear_mipmap_linear : usg::exchange::Texture_Filter_linear;

					usg::exchange::TextureCoordinator& texCo = pNewMaterial->pb().textureCoordinators[uTexIndex];
					texCo.sourceCoordinate = 0; // FIXME: Should probably switch to named UV channels
					texCo.method = usg::exchange::TextureCoordinator_MappingMethod_UV_COORDINATE; // TODO: Support other UV types
					texCo.scale.Assign((float)pTexture->GetScaleU(), (float)pTexture->GetScaleV());
					texCo.translate.Assign((float)pTexture->GetTranslationU(), (float)pTexture->GetTranslationV());
					texCo.rotate = (float)pTexture->GetRotationU();
				}
			}
		}
		
		property = pFBXMaterial->GetNextProperty(property);
	}

}



void FbxLoad::SetBoolBasedOnTexture(::exchange::Material* pNewMaterial, const char* szTexName, const char* szBoolName)
{
	uint32 uIndex;
	if (pNewMaterial->GetCustomFX(0).GetTextureIndex(szTexName, uIndex))
	{
		if (pNewMaterial->pb().textures[uIndex].textureName[0] != '\0')
		{
			pNewMaterial->SetVariable(szBoolName, true);
		}
	}
}




bool FbxLoad::SetDefaultMaterialVariables(FbxSurfaceMaterial* pFBXMaterial, ::exchange::Material* pMaterial)
{
	SetBoolBasedOnTexture(pMaterial, "DiffuseColor", "bDiffuseMap");
	SetBoolBasedOnTexture(pMaterial, "NormalMap", "bBumpMap");
	SetBoolBasedOnTexture(pMaterial, "SpecularColor", "bSpecMap");
	SetBoolBasedOnTexture(pMaterial, "EmissiveFactor", "bEmissiveMap");
	SetBoolBasedOnTexture(pMaterial, "Reflection", "bReflectionMap");

	for (uint32 i = 0; i < pMaterial->GetCustomFX(0).GetTextureCount(); i++)
	{
		uint32 uTexIndex = i;
		pMaterial->GetCustomFX(0).GetTextureIndex(i, uTexIndex);
		if (pMaterial->pb().textures[uTexIndex].textureName[0] == '\0')
		{
			// Add a dummy texture
			const char* szTextName = pMaterial->GetCustomFX(0).GetDefaultTexName(i);
			int length = (int)(strlen(szTextName));
			length = Math::Min(length, (int)sizeof(pMaterial->pb().textures[uTexIndex].textureName));
			memset(pMaterial->pb().textures[uTexIndex].textureName, 0, sizeof(pMaterial->pb().textures[uTexIndex].textureName));
			strncpy(pMaterial->pb().textures[uTexIndex].textureName, szTextName, length);
		}
	}

	FbxDouble3 double3;
	FbxDouble double1;
	bool bTransparent = false;

	// TODO: Should probably select different effects based on the material type
	if (pFBXMaterial->GetClassId().Is(FbxSurfaceLambert::ClassId))
	{
		// Amibent Color
		double3 = reinterpret_cast<FbxSurfaceLambert *>(pFBXMaterial)->Ambient;
		usg::Color ambient((float)double3.mData[0], (float)double3.mData[1], (float)double3.mData[2], 1.0f);
		pMaterial->SetVariableArray("ambient", ambient.rgba(), 4);

		// Diffuse Color
		double3 = reinterpret_cast<FbxSurfaceLambert *>(pFBXMaterial)->Diffuse;
		double1 = reinterpret_cast<FbxSurfaceLambert *>(pFBXMaterial)->DiffuseFactor;

		usg::Color diffuse((float)double3.mData[0], (float)double3.mData[1], (float)double3.mData[2], 1.0f);
	//	diffuse *= double1;
		pMaterial->SetVariableArray("diffuse", diffuse.rgba(), 4);


		// Emissive Color
		double3 = reinterpret_cast<FbxSurfaceLambert *>(pFBXMaterial)->Emissive;
		usg::Color emissive((float)double3.mData[0], (float)double3.mData[1], (float)double3.mData[2], 1.0f);

		// Emissive Factor
		double1 = reinterpret_cast<FbxSurfaceLambert *>(pFBXMaterial)->EmissiveFactor;
	//	emissive *= double1;
		//pMaterial->SetVariableArray("emission", emissive.rgba(), 4);
		
		// Transparency Factor
		double3 = reinterpret_cast<FbxSurfaceLambert *>(pFBXMaterial)->TransparentColor;
		double1 = reinterpret_cast<FbxSurfaceLambert *>(pFBXMaterial)->TransparencyFactor;
		// Ignoring 3 channel transparency, so just taking the green
		// Maya always outputs 1 for the transparency factor so we can't rely on that alone
		float transparency = (float)(double1*double3[1]);
		pMaterial->SetVariable("alpha", 1.0f- transparency);

		bTransparent = transparency > 0.0f;
	}

	if (pFBXMaterial->GetClassId().Is(FbxSurfacePhong::ClassId))
	{
		// Specular Color
		double3 = reinterpret_cast<FbxSurfacePhong *>(pFBXMaterial)->Specular;
		usg::Color specular((float)double3.mData[0], (float)double3.mData[1], (float)double3.mData[2], 1.0f);

		// Shininess
		double1 = reinterpret_cast<FbxSurfacePhong *>(pFBXMaterial)->Shininess;
		pMaterial->SetVariable("specularpow", (float)Math::Max(double1, 0.01));

		if (double1 > 0.01)
		{
			// Specular Factor
			double1 = reinterpret_cast<FbxSurfacePhong *>(pFBXMaterial)->SpecularFactor;
			//		specular *= double1;
			pMaterial->SetVariableArray("specular", specular.rgba(), 4);
		}
		else
		{
			pMaterial->SetVariableArray("specular", usg::Color::Black.rgba(), 4);
		}


		// Reflection
		double3 = reinterpret_cast<FbxSurfacePhong *>(pFBXMaterial)->Reflection;
		usg::Color reflection((float)double3.mData[0], (float)double3.mData[1], (float)double3.mData[2], 1.0f);
		pMaterial->SetVariableArray("reflection", reflection.rgba(), 4);

		// Reflection Factor
		double1 = reinterpret_cast<FbxSurfacePhong *>(pFBXMaterial)->ReflectionFactor;
		pMaterial->SetVariable("reflectionfactor", (float)double1);
	}
	
	return bTransparent;
}



void FbxLoad::SetRenderState(::exchange::Material* pNewMaterial, FbxSurfaceMaterial* pFBXMaterial, bool bTransparent) const
{
	usg::exchange::Rasterizer& rRasterizer = pNewMaterial->pb().rasterizer;

	// cull face
	rRasterizer.cullFace = usg::CULL_FACE_BACK;

	// blend mode
	rRasterizer.blendEnabled = false;
	rRasterizer.alphaState.rgbOp = BLEND_EQUATION_ADD;
	rRasterizer.alphaState.alphaOp = BLEND_EQUATION_ADD;
	if (bTransparent)
	{
		rRasterizer.blendEnabled = true;
		rRasterizer.alphaState.rgbSrcFunc = BLEND_FUNC_SRC_ALPHA;
		rRasterizer.alphaState.rgbDestFunc = BLEND_FUNC_ONE_MINUS_SRC_ALPHA;
		rRasterizer.alphaState.alphaSrcFunc = BLEND_FUNC_ONE;
		rRasterizer.alphaState.alphaDestFunc = BLEND_FUNC_ZERO;
	}

	// depth test
	turnOnFlag(rRasterizer.attribute, 1 << usg::exchange::Rasterizer_Attribute_DEPTH_TEST_ENABLE);
	turnOnFlag(rRasterizer.attribute, 1 << usg::exchange::Rasterizer_Attribute_DEPTH_TEST_WRITE);
	rRasterizer.depthTestFunc = usg::DEPTH_TEST_LEQUAL;
}


::exchange::Material* FbxLoad::NewMaterial(FbxSurfaceMaterial* pFBXMaterial, bool bSkin)
{
	::exchange::Material* pNewMaterial = vnew(ALLOC_OBJECT) ::exchange::Material();

	pNewMaterial->SetIsCustomFX(false);

	std::vector<std::string> defines;

	uint32 uTexIndex = 0;
	if (bSkin)
	{
		defines.push_back("skel");
	}
	if (pFBXMaterial->sNormalMap || pFBXMaterial->sBump)
	{
		FbxProperty property = pFBXMaterial->GetFirstProperty();

		while (property.IsValid())
		{
			uint32 uTextureCount = property.GetSrcObjectCount<FbxTexture>();
			const char* szName = property.GetNameAsCStr();
			for (uint32 i = 0; i < uTextureCount; ++i)
			{
				FbxLayeredTexture* layeredTexture = property.GetSrcObject<FbxLayeredTexture>(i);

				ASSERT_MSG((layeredTexture == nullptr), "Layered texture not supported");

				FbxTexture* pTexture = property.GetSrcObject<FbxTexture>(i);
				if (pTexture && pTexture->GetName() == "NormalMap")
				{
					defines.push_back("bump");
					break;
				}
			}
			property = pFBXMaterial->GetNextProperty(property);
		}
	}

	// material name
	const char* pMaterialName = pFBXMaterial->GetName();
	strncpy(pNewMaterial->pb().materialName, pMaterialName, strlen(pMaterialName) + 1);

	m_pOverrides->InitDefault(pNewMaterial->pb().materialName, defines, pNewMaterial);

	AddMaterialTextures(pFBXMaterial, pNewMaterial);
	bool bTransparent = SetDefaultMaterialVariables(pFBXMaterial, pNewMaterial);
	SetRenderState(pNewMaterial, pFBXMaterial, bTransparent);
	pNewMaterial->pb().attribute.translucencyKind = bTransparent ? usg::exchange::Translucency_Type_TRANSLUCENCY_TRANSLUCENT : usg::exchange::Translucency_Type_TRANSLUCENCY_OPAQUE;


	return pNewMaterial;
}


::exchange::Material* FbxLoad::DummyMaterial(bool bSkinned)
{
	::exchange::Material* pNewMaterial = vnew(ALLOC_OBJECT) ::exchange::Material();

	// FIXME: Get custom effect definitions
	const char* szUsagiPath = getenv("USAGI_DIR");
	std::string emuPath = szUsagiPath;
	// FIXME: Hunt for a matching material setting file
	emuPath += "\\Data\\GLSL\\effects\\Model.yml";
	pNewMaterial->SetIsCustomFX(false);

	std::vector<std::string> defines;
	if (bSkinned)
	{
		defines.push_back("skel");
	}

	m_pDependencies->LogDependency(emuPath.c_str());
	pNewMaterial->InitCustomMaterial(emuPath.c_str(), "FBXDefault", defines);

	// material name
	const char* pMaterialName = "Dummy";
	strncpy(pNewMaterial->pb().materialName, pMaterialName, strlen(pMaterialName) + 1);

	for (uint32 i = 0; i < pNewMaterial->GetCustomFX(0).GetTextureCount(); i++)
	{
		// Set up the default textures first
		uint32 uTexIndex = i;
		pNewMaterial->GetCustomFX(0).GetTextureIndex(i, uTexIndex);
		const char* szTextName = pNewMaterial->GetCustomFX(0).GetDefaultTexName(i);
		int length = (int)(strlen(szTextName));
		length = Math::Min(length, (int)sizeof(pNewMaterial->pb().textures[uTexIndex].textureName));
		memset(pNewMaterial->pb().textures[uTexIndex].textureName, 0, sizeof(pNewMaterial->pb().textures[uTexIndex].textureName));
		strncpy(pNewMaterial->pb().textures[uTexIndex].textureName, szTextName, length);
	}

	pNewMaterial->SetVariable("ambient", Color::White);
	pNewMaterial->SetVariable("diffuse", Color::White);
	pNewMaterial->SetVariable("emission", Color::White);
	pNewMaterial->SetVariable("alpha", 1.0f);

	bool bTransparent = false;
	pNewMaterial->pb().attribute.translucencyKind = usg::exchange::Translucency_Type_TRANSLUCENCY_OPAQUE;

	usg::exchange::Rasterizer& rRasterizer = pNewMaterial->pb().rasterizer;

	// cull face
	rRasterizer.cullFace = usg::CULL_FACE_BACK;

	// blend mode
	rRasterizer.blendEnabled = 0;
	rRasterizer.alphaState.rgbOp = BLEND_EQUATION_ADD;
	rRasterizer.alphaState.alphaOp = BLEND_EQUATION_ADD;
	if (bTransparent)
	{
		rRasterizer.blendEnabled = 1;
		rRasterizer.alphaState.rgbSrcFunc = BLEND_FUNC_SRC_ALPHA;
		rRasterizer.alphaState.rgbDestFunc = BLEND_FUNC_ONE_MINUS_SRC_ALPHA;
		rRasterizer.alphaState.alphaSrcFunc = BLEND_FUNC_ONE;
		rRasterizer.alphaState.alphaDestFunc = BLEND_FUNC_ZERO;
	}

	// depth test
	turnOnFlag(rRasterizer.attribute, 1 << usg::exchange::Rasterizer_Attribute_DEPTH_TEST_ENABLE);
	turnOnFlag(rRasterizer.attribute, 1 << usg::exchange::Rasterizer_Attribute_DEPTH_TEST_WRITE);
	rRasterizer.depthTestFunc = usg::DEPTH_TEST_LEQUAL;


	return pNewMaterial;
}

void FbxLoad::Load(Cmdl& cmdl, FbxScene* modelScene, bool bSkeletonOnly, bool bCollisionModel, DependencyTracker* pDependencies, MaterialOverrides* pOverrides)
{
	m_pOverrides = pOverrides;
	m_pDependencies = pDependencies;
	m_pScene = modelScene;
	m_bCollisionMesh = bCollisionModel;

	FbxNode* pRootNode = modelScene->GetRootNode();
	::exchange::Skeleton* pSkeleton = NewSkeleton();

	FbxPose* pPose = nullptr;
	for (uint32 i = 0; i < (uint32)modelScene->GetPoseCount(); i++)
	{
		pPose = modelScene->GetPose(i);
		if (pPose->IsBindPose())
		{
			m_bindPoses.push_back(pPose);
		}
	}

	if (pRootNode)
	{
		if (bCollisionModel || IsIdentityBoneRequired(pRootNode))
		{
			AddIdentityBone(pSkeleton);
		}

		if (!bCollisionModel)
		{
			ReadSkeleton(pSkeleton, pRootNode);
		}

		cmdl.SetSkeleton(pSkeleton);
		if (!bCollisionModel)
		{
			ReadLightsRecursive(cmdl, pRootNode);
			ReadCamerasRecursive(cmdl, pRootNode);
		}
		if (!bSkeletonOnly)
		{
			if (!bCollisionModel)
			{
				ReadAnimations(cmdl, modelScene);
			}
			ReadMeshRecursive(cmdl, pRootNode, bCollisionModel);
		}
	}

	if (!bSkeletonOnly)
	{
		// Calculate the bounding sphere sizes
		for (uint32 n = 0; n < cmdl.GetShapeNum(); n++)
		{
			// AABB
			::exchange::Shape* pShape = cmdl.GetShapePtr(n);
			Vector3 vMin, vMax;
			LoaderUtil::setupShapeAABB(vMin, vMax, cmdl.GetStreamPtr(pShape->GetPositionStreamRefIndex()));
			pShape->SetAABB(vMin, vMax);

			// AABB -> BoundingSphere
			Vector3 center = (vMax + vMin) / 2.0f;
			::usg::exchange::Sphere& sphere = pShape->pb().boundingSphere;
			sphere.center.x = center.x;
			sphere.center.y = center.y;
			sphere.center.z = center.z;
			Vector3 vRadius = vMax - center;			
			sphere.radius = vRadius.calcLength();
		}
	}
	if (PostProcessSkeleton(cmdl))
	{
		//mError = -1;
	}

	if (!bSkeletonOnly)
	{
		PostProcessing(cmdl);
	}


}

uint32 FbxLoad::FindBoneRenderingId(Cmdl& cmdl, const char* szName)
{
	uint32 index = 0;
	for (uint32 i = 0; i < cmdl.GetSkeleton()->Bones().size(); i++)
	{
		if (cmdl.GetSkeleton()->Bones()[i].isNeededRendering)
		{
			if (strcmp(cmdl.GetSkeleton()->Bones()[i].name, szName) == 0)
			{
				return index;
			}
			index++;
		}
	}
	ASSERT(false);
	return 0;
}

uint32 FbxLoad::FindBone(::exchange::Skeleton& skel, const char* szName)
{
	for (uint32 i = 0; i < skel.Bones().size(); i++)
	{
		if (strcmp(skel.Bones()[i].name, szName) == 0)
		{
			return i;
		}
	}
	ASSERT(false);
	return 0;
}

void FbxLoad::AddStreams(Cmdl& cmdl, ::exchange::Shape* pShape, FbxNode* ppNode, FbxMesh* pFbxMesh)
{
	uint32 uStreamCount = (uint32_t)m_activeVerts[0].elements.size();
	for (uint32 i = 0; i < uStreamCount; i++)
	{
		::exchange::Stream* pNewStream = vnew(ALLOC_OBJECT) ::exchange::Stream();

		uint32 uColumnNum = m_activeVerts[0].elements[i].uCount;
		switch (m_activeVerts[0].elements[i].eElementType)
		{
		case usg::VE_FLOAT:
		case usg::VE_INT:
		case usg::VE_UINT:
		{
			pNewStream->allocate<float>((uint32_t)m_activeVerts.size()*uColumnNum, uColumnNum);
			float* pData = (float*)pNewStream->GetStreamArrayPtr();

			usg::vector<TempVertex>::iterator it;
			for (it = m_activeVerts.begin(); it != m_activeVerts.end(); ++it)
			{
				uint32 uCount = (*it).elements[i].uCount;
				ASSERT(uCount == uColumnNum);
				for (uint32 element = 0; element < uCount; element++)
				{
					*pData++ = (*it).elements[i].elements[element];
				}
				ASSERT((*it).elements[i].type == m_activeVerts[0].elements[i].type);
			}
			break;
		}
		case usg::VE_UBYTE:
		case usg::VE_BYTE:
		{
			pNewStream->allocate<uint8>((uint32_t)m_activeVerts.size()*uColumnNum, uColumnNum);
			uint8* pData = (uint8*)pNewStream->GetStreamArrayPtr();

			usg::vector<TempVertex>::iterator it;
			for (it = m_activeVerts.begin(); it != m_activeVerts.end(); ++it)
			{
				uint32 uCount = (*it).elements[i].uCount;
				ASSERT(uCount == uColumnNum);
				for (uint32 element = 0; element < uCount; element++)
				{
					*pData++ = (*it).elements[i].uElements[element];
				}
			}
			break;
		}
		default:
			ASSERT(false);
		}


		cmdl.AddStream(pNewStream);

		usg::exchange::VertexStreamInfo info;
		VertexStreamInfo_init(&info);
		info.elementType = m_activeVerts[0].elements[i].eElementType;
		info.scaling = 1.0f;
		info.attribute = m_activeVerts[0].elements[i].type;
		strcpy_s(info.usageHint, sizeof(info.usageHint), m_activeVerts[0].elements[i].hint.c_str());
		info.columns = uColumnNum;
		info.refIndex = cmdl.GetStreamNum() - 1;

		pShape->AddVertexStreamInfo(info);

	}

	// We now handle default streams in the custom effect definition
#if 0
	// Single attribute, for now always add colour data if we don't have it
	if (pShape->SearchStream(usg::exchange::VertexAttribute_COLOR) == UINT32_MAX)
	{
		usg::Vector4f vec(1.0f, 1.0f, 1.0f, 1.0f);
		usg::exchange::Shape& rShape = pShape->pb();
		size_t count = rShape.singleAttributes_count;
		rShape.singleAttributes[count].attribute = usg::exchange::VertexAttribute_COLOR;
		rShape.singleAttributes[count].value = vec;
		rShape.singleAttributes[count].columns = 4;
		strcpy_s(rShape.singleAttributes[count].usageHint, "color");

		ASSERT_MSG(count + 1 <= usg::exchange::Shape::singleAttributes_max_count, "Single attribute over flow");
		rShape.singleAttributes_count = (pb_size_t)(count + 1);
	}
#endif

	// Add the index stream
	::exchange::Stream* pNewStream = vnew(ALLOC_OBJECT) ::exchange::Stream();

	pNewStream->allocate<uint32>((uint32_t)m_indicesTmp.size(), 3);
	uint32* pData = (uint32*)pNewStream->GetStreamArrayPtr();

	usg::vector<uint32>::iterator it;
	for (it = m_indicesTmp.begin(); it != m_indicesTmp.end(); ++it)
	{
		*pData++ = (*it);
	}
	cmdl.AddStream(pNewStream);

	const bool bHasSkin = pFbxMesh->GetDeformerCount(FbxDeformer::eSkin) > 0;

	usg::exchange::SkinningType eSkinType;

	if (bHasSkin)
	{
		// TODO: Add primitive info
		FbxSkin * pSkinDeformer = (FbxSkin *)pFbxMesh->GetDeformer(0, FbxDeformer::eSkin);
		FbxSkin::EType eSkinningType = pSkinDeformer->GetSkinningType();
		if (eSkinningType == FbxSkin::eRigid)
		{
			eSkinType = usg::exchange::SkinningType_RIGID_SKINNING;
		}
		else// if (eSkinningType == FbxSkin::eBlend || eSkinningType == FbxSkin::eLinear || eSkinningType == FbxSkin::eDualQuaternion)
		{
			eSkinType = usg::exchange::SkinningType_SMOOTH_SKINNING;
		}
	}
	else
	{
		eSkinType = usg::exchange::SkinningType_NO_SKINNING;
	}

	uint32 uNumOfDeformers = pFbxMesh->GetDeformerCount();

	::exchange::PrimitiveInfo& info = pShape->GetPrimitiveInfo();
	::exchange::PrimitiveInfo_init(info);
	info.lodLevel = 0;
	info.eSkinType = eSkinType;
	info.adjacencyStreamRefIndex = 0;
	info.indexStreamRefIndex = cmdl.GetStreamNum() - 1;
	info.indexStreamFormatSize = sizeof(uint32);

	for (uint32 uDeformer = 0; uDeformer < uNumOfDeformers; ++uDeformer)
	{
		FbxSkin* pSkinnedDeformer = reinterpret_cast<FbxSkin*>(pFbxMesh->GetDeformer(uDeformer, FbxDeformer::eSkin));
		if (!pSkinnedDeformer)
		{
			continue;
		}
		uint32 uNumOfClusters = pSkinnedDeformer->GetClusterCount();
		// FIXME: We should only have one bone set per model
		for (uint32 uClusterIndex = 0; uClusterIndex < uNumOfClusters; ++uClusterIndex)
		{
			FbxCluster* pCurrCluster = pSkinnedDeformer->GetCluster(uClusterIndex);
			std::string currJointName = pCurrCluster->GetLink()->GetName();

			std::string boneName = currJointName;
			size_t lodPos = boneName.find("_LOD");
			if ( std::string::npos != lodPos )
			{
				// Don't use the LOD name
				boneName[lodPos] = '\0';
			}

			RegisterBoneUsage(cmdl, boneName.c_str(), eSkinType);

			strncpy(info.rootBoneName, boneName.c_str(), sizeof(info.rootBoneName) - 1);
			info.rootBoneIndex = FindBone(*cmdl.GetSkeleton(), boneName.c_str());

		}  
	}

	FbxNode* pParent = ppNode;
	while (pParent && !IsBone(pParent))
	{
		pParent = pParent->GetParent();
	}
	if (eSkinType == usg::exchange::SkinningType_NO_SKINNING && !m_bCollisionMesh)
	{
		std::string boneName = pParent->GetName();
		strncpy(info.rootBoneName, boneName.c_str(), sizeof(info.rootBoneName) - 1);
		info.rootBoneIndex = FindBone(*cmdl.GetSkeleton(), boneName.c_str());
	}

	if (info.rootBoneName[0] == '\0')
	{
		strncpy(info.rootBoneName, cmdl.GetSkeleton()->Bones()[0].name, sizeof(info.rootBoneName) - 1);
		info.rootBoneIndex = 0;

	}

}

void FbxLoad::RegisterBoneUsage(Cmdl& cmdl, const char* szBoneName, usg::exchange::SkinningType eSkinType)
{
	if (eSkinType == usg::exchange::SkinningType_NO_SKINNING)
		return;	// We don't need to index this

	vector<BoneInfo>& searchSet = eSkinType == usg::exchange::SkinningType_SMOOTH_SKINNING ? m_skinnedBoneIndices : m_rigidBoneIndices;


	uint32 uIdInSkeleton = FindBoneRenderingId(cmdl, szBoneName);

	for (auto itr = searchSet.begin(); itr != searchSet.end(); ++itr)
	{
		if ( (*itr).uOriginalMapping == uIdInSkeleton )
		{
			return;
		}
	}

	BoneInfo boneInfo;
	boneInfo.uOriginalMapping = uIdInSkeleton;
	strcpy_s(boneInfo.szName, sizeof(boneInfo.szName), szBoneName);
	searchSet.push_back(boneInfo);
}


void FbxLoad::ReadMeshRecursive(Cmdl& cmdl, FbxNode* pNode, bool bStatic)
{
	m_pParentBoneNode = nullptr;
	if (pNode->GetNodeAttribute())
	{
		auto type = pNode->GetNodeAttribute()->GetAttributeType();
		switch (type)
		{
		case FbxNodeAttribute::eMesh:

			FbxMesh* pMesh = (FbxMesh*)pNode;
			if (pMesh->GetDeformerCount(FbxDeformer::eSkin) == 0)
			{
				FbxNode* pParent = pNode;
				while (pParent && !IsBone(pParent))
				{
					pParent = pParent->GetParent();
				}
				m_pParentBoneNode = bStatic ? nullptr : pParent;
			}

			// Materials need to know if they are skinned
			bool bSkinned = false;
			for (uint32 uAttribId = 0; uAttribId < (uint32)pNode->GetNodeAttributeCount(); uAttribId++)
			{
				if (pNode->GetNodeAttributeByIndex(uAttribId)->GetAttributeType() == FbxNodeAttribute::eMesh)
				{
					FbxMesh* pFbxMesh = (FbxMesh*)pNode->GetNodeAttributeByIndex(uAttribId);
					if (pFbxMesh->GetPolygonCount() > 0)
					{
						bSkinned |= pFbxMesh->GetDeformerCount(FbxDeformer::eSkin) > 0;
					}
				}
			}

			m_uMeshMaterialOffset = cmdl.GetMaterialNum();
			AddMaterials(cmdl, pNode, bSkinned);
			
			for (uint32 uAttribId = 0; uAttribId < (uint32)pNode->GetNodeAttributeCount(); uAttribId++)
			{
				if (pNode->GetNodeAttributeByIndex(uAttribId)->GetAttributeType() == FbxNodeAttribute::eMesh)
				{
					FbxMesh* pAttribMesh = (FbxMesh*)pNode->GetNodeAttributeByIndex(uAttribId);
					if (pAttribMesh->GetPolygonCount() > 0)
					{
						::exchange::Shape* pShape = NewShape(cmdl, pNode);
						AddMesh(cmdl, pShape, pNode, (FbxMesh*)pNode->GetNodeAttributeByIndex(uAttribId), bStatic);
						RemoveDuplicateVertices();
						// Now add the verts and indices
						AddStreams(cmdl, pShape, pNode, (FbxMesh*)pNode->GetNodeAttributeByIndex(uAttribId));
						cmdl.AddShape(pShape);
						::exchange::Mesh* pMesh = NewMesh(cmdl, pNode);
						cmdl.AddMesh(pMesh);
					}
				}
			}
			break;
		}
	}

	for (int i = 0; i < pNode->GetChildCount(); ++i)
	{
		ReadMeshRecursive(cmdl, pNode->GetChild(i), bStatic);
	}
}


void FbxLoad::ReadSkeleton(::exchange::Skeleton* pSkeleton, FbxNode* pRootNode)
{
	ReadDeformersRecursive(pSkeleton, pRootNode);

	ReadBonesRecursive(pSkeleton, pRootNode);
}

int FbxLoad::GetParentBoneCountRecursive(FbxNode* pNode, int Count)
{
	for (int i = 0; i < pNode->GetChildCount(); i++)
	{
		Count = GetParentBoneCountRecursive(pNode->GetChild(i), Count);
	}
	if (IsBone(pNode))
	{
		FbxNode* pParent = pNode->GetParent();
		bool bHasParent = false;
		while (pParent)
		{
			if (IsBone(pParent))
			{
				bHasParent = true;
				break;
			}
			pParent = pParent->GetParent();
		}
		if (!bHasParent)
		{
			Count++;
		}
	}
	return Count;
}

bool FbxLoad::IsIdentityBoneRequired(FbxNode* pNode)
{
	int BoneCount = GetParentBoneCountRecursive(pNode, 0);
	
	return BoneCount != 1;
}


void FbxLoad::ReadDeformersRecursive(::exchange::Skeleton* pSkeleton, FbxNode* pNode)
{
	if (pNode->GetNodeAttribute() && pNode->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eMesh)
	{
		FbxMesh* pCurrMesh = pNode->GetMesh();
		uint32 uDeformers = pCurrMesh->GetDeformerCount();

		for (uint32 uDeformer = 0; uDeformer < uDeformers; ++uDeformer)
		{
			FbxSkin* pCurrSkin = (FbxSkin*)(pCurrMesh->GetDeformer(uDeformer, FbxDeformer::eSkin));
			if (!pCurrSkin)
			{
				continue;
			}

			uint32 uClusters = pCurrSkin->GetClusterCount();
			for (uint32 uCluster = 0; uCluster < uClusters; ++uCluster)
			{
				m_clusters.push_back(pCurrSkin->GetCluster(uCluster));
			}
		}
	}

	for (int i = 0; i < pNode->GetChildCount(); i++)
	{
		ReadDeformersRecursive(pSkeleton, pNode->GetChild(i));
	}
}


void FbxLoad::ReadCamerasRecursive(Cmdl& cmdl, FbxNode* pNode)
{
	if (pNode->GetNodeAttribute() && pNode->GetNodeAttribute()->GetAttributeType())
	{
		if (pNode->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eCamera
			|| pNode->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eCameraStereo)
		{
			AddCamera(cmdl, pNode);
		}
	}
	for (int i = 0; i < pNode->GetChildCount(); i++)
	{
		ReadCamerasRecursive(cmdl, pNode->GetChild(i));
	}
}

void FbxLoad::ReadLightsRecursive(Cmdl& cmdl, FbxNode* pNode)
{
	if (pNode->GetNodeAttribute() && pNode->GetNodeAttribute()->GetAttributeType())
	{
		if (pNode->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eLight)
		{
			AddLight(cmdl, pNode);
		}
	}
	for (int i = 0; i < pNode->GetChildCount(); i++)
	{
		ReadLightsRecursive(cmdl, pNode->GetChild(i));
	}
}

bool FbxLoad::IsBone(FbxNode* pNode)
{
	if (pNode->GetNodeAttribute() && pNode->GetNodeAttribute()->GetAttributeType())
	{
		FbxNodeAttribute::EType attribType = pNode->GetNodeAttribute()->GetAttributeType();
		if (attribType == FbxNodeAttribute::eSkeleton || attribType == FbxNodeAttribute::eNull || attribType == FbxNodeAttribute::eMarker
			|| pNode->GetParent() == m_pScene->GetRootNode() )
		{
			if (attribType == FbxNodeAttribute::eMesh && ((FbxMesh*)pNode->GetNodeAttribute())->GetDeformerCount() > 0)
			{
				return false;
			}
			return true;
		}
	}
	return false;
}

void FbxLoad::ReadBonesRecursive(::exchange::Skeleton* pSkeleton, FbxNode* pNode)
{
	if (IsBone(pNode))
	{
		FbxNodeAttribute::EType attribType = pNode->GetNodeAttribute()->GetAttributeType();
		AddBone(pSkeleton, pNode, attribType == FbxNodeAttribute::eSkeleton || pNode->GetParent() == m_pScene->GetRootNode());
	}
	for (int i = 0; i < pNode->GetChildCount(); i++)
	{
		ReadBonesRecursive(pSkeleton, pNode->GetChild(i));
	}
}


uint32 FbxLoad::GetBlendWeightsAndIndices(Cmdl& cmdl, FbxNode* pNode, FbxMesh* pCurrMesh)
{
	// Clear the existing weights and re-size to match the number of vertices
	m_activeWeights.clear();

	uint32 uNumOfDeformers = pCurrMesh->GetDeformerCount();
	uint32 uMaxWeights = 0;

	if (uNumOfDeformers > 0)
	{
		// Only allocate space for the weights if this mesh has any deformers
		m_activeWeights.resize(pCurrMesh->GetControlPointsCount());
	}

	// Usually only one deformer per mesh
	for (uint32 uDeformerIndex = 0; uDeformerIndex < uNumOfDeformers; ++uDeformerIndex)
	{
		FbxSkin* pSkinDeformer = (FbxSkin*)(pCurrMesh->GetDeformer(uDeformerIndex, FbxDeformer::eSkin));
		if (!pSkinDeformer)
		{
			// Not a skinned deformer
			continue;
		}

		uint32 uNumOfClusters = pSkinDeformer->GetClusterCount();

		for (uint32 uClusterIndex = 0; uClusterIndex < uNumOfClusters; ++uClusterIndex)
		{
			FbxCluster* currCluster = pSkinDeformer->GetCluster(uClusterIndex);
			const char* currJointName = currCluster->GetLink()->GetName();
			uint32 uJointIndex = FindBoneRenderingId(cmdl, currJointName);

			uint32 uNumOfIndices = currCluster->GetControlPointIndicesCount();
			uint32 uVertexCount = (uint32)m_activeVerts.size();
			BoneWeight weight;
			weight.index = uJointIndex;
			for (uint32 i = 0; i < uNumOfIndices; ++i)
			{
				weight.fValue = (float)currCluster->GetControlPointWeights()[i];
				m_activeWeights[currCluster->GetControlPointIndices()[i]].weights.push_back(weight);
			}
		}
	}

	for (auto itr = m_activeWeights.begin(); itr != m_activeWeights.end(); ++itr)
	{
		if (itr->weights.size() > 4)
		{
			std::sort(itr->weights.begin(), itr->weights.end());
			itr->weights.resize(4);
			// Now re-calculate the weighting
			float fTotalWeight = 0.0f;
			auto weightItr = itr->weights.begin();
			for (; weightItr != itr->weights.end(); ++weightItr)
			{
				fTotalWeight += (*weightItr).fValue;
			}

			ASSERT(usg::Math::IsEqual(fTotalWeight, 1.0f, 0.01f));
			for (weightItr = itr->weights.begin(); weightItr != itr->weights.end(); ++weightItr)
			{
				(*weightItr).fValue /= fTotalWeight;
			}
		}
		uMaxWeights = usg::Math::Max((uint32)itr->weights.size(), uMaxWeights); 
	}


	return uMaxWeights;
}

FbxAMatrix FbxLoad::GetCombinedMatrixForNode(FbxNode* pNode, FbxNode* pParent, FbxTime pTime)
{
	if (pParent != nullptr )
	{
		FbxAMatrix mWorld = pNode->EvaluateGlobalTransform(pTime);
		FbxAMatrix mSkeletonWorld = pParent->EvaluateGlobalTransform(pTime);
		FbxAMatrix mCombined = mSkeletonWorld.Inverse() * mWorld;
		return mCombined;
	}

	FbxAMatrix mGeometry;
	mGeometry.SetT(pNode->GetGeometricTranslation(FbxNode::eSourcePivot));
	mGeometry.SetR(pNode->GetGeometricRotation(FbxNode::eSourcePivot));
	mGeometry.SetS(pNode->GetGeometricScaling(FbxNode::eSourcePivot));

	FbxAMatrix mGlobal = pNode->EvaluateGlobalTransform(pTime);


	return mGlobal * mGeometry;
}

::exchange::MaterialAnimation* FbxLoad::NewMaterialAnimation(Cmdl& cmdl, FbxAnimStack* animStack)
{
	::exchange::MaterialAnimation* pNewAnimation = vnew(ALLOC_OBJECT) ::exchange::MaterialAnimation();

	pNewAnimation->SetName(animStack->GetName());
	fbxsdk::FbxTime duration = animStack->GetLocalTimeSpan().GetDuration();
	duration.SetGlobalTimeMode(FRAME_MODE);
	pNewAnimation->InitTiming((uint32)duration.GetFrameCount() + 1, 30.0f);

	return pNewAnimation;
}

::exchange::Animation* FbxLoad::NewAnimation(Cmdl& cmdl, FbxAnimStack* animStack)
{
	::exchange::Animation* pNewAnimation = vnew(ALLOC_OBJECT) ::exchange::Animation();

	pNewAnimation->SetName(animStack->GetName());
	fbxsdk::FbxTime duration = animStack->GetLocalTimeSpan().GetDuration();
	duration.SetGlobalTimeMode(FRAME_MODE);
	pNewAnimation->InitTiming((uint32)duration.GetFrameCount()+1, 30.0f);

	return pNewAnimation;
	
}

void FbxLoad::ReadAnimations(Cmdl& cmdl, FbxScene* pScene)
{
	for (int stack = 0; stack < pScene->GetSrcObjectCount<FbxAnimStack>(); stack++)
	{
		FbxAnimStack* pAnimStack = pScene->GetSrcObject<FbxAnimStack>(stack);

		std::string stackName = pAnimStack->GetName();

		::exchange::Animation* pNewAnim = NewAnimation(cmdl, pAnimStack);
		::exchange::MaterialAnimation* pMatAnim = NewMaterialAnimation(cmdl, pAnimStack);

		pScene->SetCurrentAnimationStack(pAnimStack);

 		int nbAnimLayers = pAnimStack->GetMemberCount<FbxAnimLayer>();

		ReadAnimationsRecursive(pAnimStack, pNewAnim, pScene->GetRootNode());
		pNewAnim->AllocateAnimBones();
		ReadAnimationKeyFramesRecursively(pAnimStack, pNewAnim, pMatAnim, pScene->GetRootNode());

		if(pNewAnim->ValidAnim())
		{
			cmdl.AddAnimation(pNewAnim);
		}
		else
		{
			vdelete pNewAnim;
		}

		if (pMatAnim->ValidAnim())
		{
			cmdl.AddMaterialAnimation(pMatAnim);
		}
		else
		{
			vdelete pMatAnim;
		}

	}
	pScene->SetCurrentAnimationStack(nullptr);
}

bool FbxLoad::GetAnimBoneInfluences(FbxAnimLayer* pAnimLayer, FbxNode* pNode, bool& bTrans, bool& bRot, bool& bScale)
{
	bTrans = pNode->LclTranslation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_X)
		|| pNode->LclTranslation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y)
		|| pNode->LclTranslation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z);

	bRot = pNode->LclRotation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_X)
		|| pNode->LclRotation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y)
		|| pNode->LclRotation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z);

	bScale = pNode->LclScaling.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_X)
		|| pNode->LclScaling.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y)
		|| pNode->LclScaling.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z);

	return bTrans || bRot || bScale;
}

void FbxLoad::FillOutAnimFrame(FbxNode* pNode, FbxTime currTime, usg::exchange::BoneAnimationFrame* pFrame)
{
	FbxAMatrix currentTransformOffset = pNode->EvaluateLocalTransform(currTime);// *geomTransform;
	FbxVector4 trans = currentTransformOffset.GetT();
	FbxVector4 rot = currentTransformOffset.GetR();
	FbxVector4 scale = currentTransformOffset.GetS();

	// FIXME: All of this functionality should be moved over to the reverse co-ordinate function
	usg::Matrix4x4 mRot;
	mRot.MakeRotate(Math::DegreesToRadians((float)rot[0]), Math::DegreesToRadians(-(float)rot[1]), -Math::DegreesToRadians((float)rot[2]));
	pFrame->qRot = mRot;
	// FIXME: Having to apply the scale here almost certainly means we are failing to apply a necessary transform
	pFrame->vPos.Assign(-(float)(trans[0]* m_appliedScale), (float)(trans[1] * m_appliedScale), (float)(trans[2] * m_appliedScale));
	pFrame->vScale.Assign((float)(scale[0]), (float)(scale[1]), (float)(scale[2]));
}

usg::exchange::CurveKeyFrameType FbxLoad::GetKeyFrameType(FbxAnimCurveDef::EInterpolationType eTypeIn)
{
	switch (eTypeIn)
	{
		case FbxAnimCurveDef::eInterpolationLinear:
			return usg::exchange::CurveKeyFrameType_LINEAR;
		case FbxAnimCurveDef::eInterpolationCubic:
			return usg::exchange::CurveKeyFrameType_HERMITE;
		case FbxAnimCurveDef::eInterpolationConstant:
			return usg::exchange::CurveKeyFrameType_STEP;
		default:
			ASSERT(false);
			return usg::exchange::CurveKeyFrameType_LINEAR;
	}
}

bool FbxLoad::AddAnimCurve(FbxAnimStack* pAnimStack, ::exchange::MaterialAnimation* pMatAnim, FbxPropertyT<FbxDouble3>& prop, usg::exchange::MaterialAnimationMemberType eType, const char* szName)
{
	FbxAnimLayer* pAnimLayer = pAnimStack->GetMember<FbxAnimLayer>(0);

	FbxLongLong start = pAnimStack->GetLocalTimeSpan().GetStart().GetFrameCount(FRAME_MODE);
	FbxLongLong end = pAnimStack->GetLocalTimeSpan().GetStop().GetFrameCount(FRAME_MODE);

	pMatAnim->AddMember();


	usg::exchange::AnimationMember& member = pMatAnim->GetCurrentMemberData();
	member.type = eType;
	member.targetID = 0;
	member.curveNum = 0;
	strcpy(member.targetName, szName);

	const char* szCurve[3] = { FBXSDK_CURVENODE_COMPONENT_X, FBXSDK_CURVENODE_COMPONENT_Y, FBXSDK_CURVENODE_COMPONENT_Z };

	for(int i=0; i<3; i++)
	{
		FbxAnimCurve* pCurve = prop.GetCurve(pAnimLayer, szCurve[i]);

		if(!pCurve)
			continue;


		::exchange::MaterialAnimation::Curve curve;
		curve.curve.axis = 0;
		curve.curve.keyFrameNum = pCurve->KeyGetCount();
		curve.curve.start = (sint32)start;
		curve.curve.end = (sint32)end;

		for (int i = 0; i <= curve.curve.keyFrameNum; ++i)
		{
			FbxAnimCurveKey fbxKey = pCurve->KeyGet(i);
			if (pCurve)
			{
				usg::exchange::CurveKeyFrame frame;
				frame.frame = (float)fbxKey.GetTime().GetSecondDouble();
				frame.type = GetKeyFrameType(fbxKey.GetInterpolation());
				// Confirm slopes match the original intention
				frame.inSlope = fbxKey.GetDataFloat(FbxAnimCurveDef::eRightSlope);
				frame.outSlope = fbxKey.GetDataFloat(FbxAnimCurveDef::eNextLeftSlope);
				curve.keyFrames.push_back(frame);
			}
		}

		member.curveNum++;

		pMatAnim->AddCurve(curve);
	}

	if (member.curveNum == 0)
	{
		// Remove this member set
		pMatAnim->PopMember();
		return false;
	}
	return true;
}

void FbxLoad::ReadAnimationKeyFramesRecursively(FbxAnimStack* pAnimStack, ::exchange::Animation* pAnim, ::exchange::MaterialAnimation* pMatAnim, FbxNode* pNode)
{
	if (pAnim->GetBone(pNode->GetName()))
	{
		FbxLongLong start = pAnimStack->GetLocalTimeSpan().GetStart().GetFrameCount(FRAME_MODE);
		FbxLongLong end = pAnimStack->GetLocalTimeSpan().GetStop().GetFrameCount(FRAME_MODE);
		
		for (FbxLongLong i = start; i <= end; ++i)
		{	
			usg::exchange::BoneAnimationFrame* pFrame = pAnim->GetBoneAnimFrame(pNode->GetName(), (uint32)(i-start));
			FbxTime currTime;
			currTime.SetFrame(i, FRAME_MODE);
			FillOutAnimFrame(pNode, currTime, pFrame);
		}
	}

	for (int mat = 0; mat < pNode->GetMaterialCount(); mat++)
	{
		bool bValidAnim = false;
		pMatAnim->AddMemberSet();

		FbxSurfaceMaterial* pMat = pNode->GetMaterial(mat);
		if (pMat->GetClassId().Is(FbxSurfaceLambert::ClassId))
		{
			FbxSurfaceLambert* pLamb = reinterpret_cast<FbxSurfaceLambert*>(pMat);
			bValidAnim |= AddAnimCurve(pAnimStack, pMatAnim, pLamb->Ambient, usg::exchange::MaterialAnimationMemberType_COLOR_AMBIENT, pMat->GetName());
			bValidAnim |= AddAnimCurve(pAnimStack, pMatAnim, pLamb->Diffuse, usg::exchange::MaterialAnimationMemberType_COLOR_DIFFUSE, pMat->GetName());
			bValidAnim |= AddAnimCurve(pAnimStack, pMatAnim, pLamb->Emissive, usg::exchange::MaterialAnimationMemberType_COLOR_EMISSION, pMat->GetName());

			// One or the other so we try both in the same destination
			bValidAnim |= AddAnimCurve(pAnimStack, pMatAnim, pLamb->VectorDisplacementColor, usg::exchange::MaterialAnimationMemberType_DISPLACEMENT, pMat->GetName());
			bValidAnim |= AddAnimCurve(pAnimStack, pMatAnim, pLamb->DisplacementColor, usg::exchange::MaterialAnimationMemberType_DISPLACEMENT, pMat->GetName());
		}

		if (pMat->GetClassId().Is(FbxSurfacePhong::ClassId))
		{
			FbxSurfacePhong* pPhong = reinterpret_cast<FbxSurfacePhong*>(pMat);
			bValidAnim |= AddAnimCurve(pAnimStack, pMatAnim, pPhong->Specular, usg::exchange::MaterialAnimationMemberType_COLOR_SPECULAR_0, pMat->GetName());
		}

		FbxProperty property = pMat->GetFirstProperty();

		uint32 uTextureCount = property.GetSrcObjectCount<FbxTexture>();
		for (uint32 tex = 0; tex < uTextureCount; tex++)
		{
			FbxTexture* pTexture = property.GetSrcObject<FbxTexture>(tex);
			if (pTexture)
			{
				bValidAnim |= AddAnimCurve(pAnimStack, pMatAnim, pTexture->Translation, usg::exchange::MaterialAnimationMemberType_TRANSLATE, pMat->GetName());
				bValidAnim |= AddAnimCurve(pAnimStack, pMatAnim, pTexture->Rotation, usg::exchange::MaterialAnimationMemberType_ROTATE, pMat->GetName());
				bValidAnim |= AddAnimCurve(pAnimStack, pMatAnim, pTexture->Scaling, usg::exchange::MaterialAnimationMemberType_SCALE, pMat->GetName());
			}
		}
		if (!bValidAnim)
		{
			pMatAnim->PopMemberSet();
		}
	}


	for (int modelCount = 0; modelCount < pNode->GetChildCount(); modelCount++)
	{
		ReadAnimationKeyFramesRecursively(pAnimStack, pAnim, pMatAnim, pNode->GetChild(modelCount));
	}
}

void FbxLoad::ReadAnimationsRecursive(FbxAnimStack* pAnimStack, ::exchange::Animation* pAnim, FbxNode* pNode)
{
	FbxAnimLayer* pAnimLayer = pAnimStack->GetMember<FbxAnimLayer>(0);

	FbxTimeSpan spanOut;
	bool bTrans, bRot, bScale;
	if (pNode->GetAnimationInterval(spanOut, pAnimStack, 0))
	{
		if (GetAnimBoneInfluences(pAnimLayer, pNode, bTrans, bRot, bScale))
		{
			pAnim->AddBone(pNode->GetName(), bRot, bTrans, bScale);
		}
	}

	for (int modelCount = 0; modelCount < pNode->GetChildCount(); modelCount++)
	{
		ReadAnimationsRecursive(pAnimStack, pAnim, pNode->GetChild(modelCount));
	}
}

void FbxLoad::AddMesh(Cmdl& cmdl, ::exchange::Shape* pShape, FbxNode* pNode, FbxMesh* currMesh, bool bStatic)
{
	m_activeVerts.clear();
	m_indicesTmp.clear();
	m_uTrianglesTmp = currMesh->GetPolygonCount();
	m_indicesTmp.reserve(m_uTrianglesTmp*3);
	m_activeVerts.reserve(m_uTrianglesTmp * 3);

	// TODO: Do a better job of selecting the vertex type
	FBXVertexElement position("position", usg::exchange::VertexAttribute_POSITION, usg::VE_FLOAT, 0, 3);
	FBXVertexElement normal("normal", usg::exchange::VertexAttribute_NORMAL, usg::VE_FLOAT, 0, 3);
	FBXVertexElement tangent("tangent", usg::exchange::VertexAttribute_TANGENT, usg::VE_FLOAT, 0, 3);
	FBXVertexElement color("color", usg::exchange::VertexAttribute_COLOR, usg::VE_FLOAT, 0, 3);
	FBXVertexElement binormal("binormal", usg::exchange::VertexAttribute_BINORMAL, usg::VE_FLOAT, 0, 3);
	FBXVertexElement blendweight("blendweight", usg::exchange::VertexAttribute_BONE_WEIGHT, usg::VE_FLOAT, 0, 4);
	FBXVertexElement blendindices("blendindex", usg::exchange::VertexAttribute_BONE_INDEX, usg::VE_UBYTE, 0, 4);
	FBXVertexElement UV("UV0", usg::exchange::VertexAttribute_UV, usg::VE_FLOAT, 0, 2);

	// Set up the defaults for all of the vertex types
	position.uCount = 3;
	position.type = usg::exchange::VertexAttribute_POSITION;
	position.uIndex = 0;

	// Pre-transform
//	currMesh->ApplyPivot();
	fbxsdk::FbxStringList uvNames;
	currMesh->GetUVSetNames(uvNames); 
	
	FbxAMatrix transform = GetCombinedMatrixForNode(pNode, m_pParentBoneNode);
	FbxAMatrix normalTransform = transform.Inverse();
	normalTransform = normalTransform.Transpose();
	
	//fbxsdk::FbxAMatrix trans = pNode->EvaluateLocalTransform();
	
	FbxLayerElementArrayTemplate<int>* materialIndices;
	int materialIndex = 0;
	if (currMesh->GetElementMaterialCount() != 0)
	{
		ASSERT(currMesh->GetElementMaterial()->GetMappingMode() == FbxGeometryElement::eAllSame);
		currMesh->GetMaterialIndices(&materialIndices);
		materialIndex = materialIndices->GetAt(0);
	}

	m_uMeshMaterialTmp = (uint32)materialIndex + m_uMeshMaterialOffset;

	if ( m_bHasNormalMap && currMesh->GetElementTangentCount() < 1)
	{
		// TODO: Use the normal map UV set index
		currMesh->GenerateTangentsData(0);
	}

	int iVertex = 0;

	for (uint32 uTriangle = 0; uTriangle < m_uTrianglesTmp; ++uTriangle)
	{
		for (uint32 uTriangleVert = 0; uTriangleVert < 3; ++uTriangleVert)
		{
			int iVertexIndex = currMesh->GetPolygonVertex(uTriangle, uTriangleVert);
			fbxsdk::FbxVector4 ctrlPoint = currMesh->GetControlPointAt(iVertexIndex);
			ctrlPoint = transform.MultT(ctrlPoint);
			position.elements[0] = (float)(ctrlPoint[0]* m_appliedScale);
			position.elements[1] = (float)(ctrlPoint[1]* m_appliedScale);
			position.elements[2] = (float)(ctrlPoint[2]* m_appliedScale);

			TempVertex vertexOut;
			vertexOut.controlPointIndex = iVertexIndex;
			vertexOut.elements.push_back(position);

			if (GetNormal(currMesh, iVertexIndex, iVertex, normal))
			{
				normal.Transform(normalTransform, 0.0f);
				vertexOut.elements.push_back(normal);
			}

			if (GetBinormal(currMesh, iVertexIndex, iVertex, binormal))
			{
				binormal.Transform(normalTransform, 0.0f);
				vertexOut.elements.push_back(binormal);
			}

			if (GetTangent(currMesh, iVertexIndex, iVertex, tangent))
			{
				tangent.Transform(normalTransform, 0.0f);
				vertexOut.elements.push_back(tangent);
			}

			// TODO: Multiple color streams
			if (GetColor(currMesh, iVertexIndex, 0, color))
			{
				vertexOut.elements.push_back(color);
			}
			
			int uUVCount = currMesh->GetUVLayerCount();
			for (int k = 0; k < uUVCount; ++k)
			{
				GetUV(currMesh, iVertexIndex, currMesh->GetTextureUVIndex(uTriangle, uTriangleVert), k, UV);
				UV.hint = uvNames[k];
				vertexOut.elements.push_back(UV);
			}

			m_activeVerts.push_back(vertexOut);
			m_indicesTmp.push_back(iVertex);
			++iVertex;
		}
	}

	// TODO: We need a check to see if all verts use the same bone 
	uint32 uMaxWeights = GetBlendWeightsAndIndices(cmdl, pNode, currMesh);
	if (uMaxWeights > 0)
	{
		// Alignment and input issues if we don't have 4
		uMaxWeights = uMaxWeights < 4 ? 4 : uMaxWeights;
	}

	for (uint32 uVert=0; uVert<m_activeVerts.size(); uVert++)
	{
		if (uMaxWeights > 0)
		{
			WeightingInfo& info = m_activeWeights[m_activeVerts[uVert].controlPointIndex];
			for (uint32 i = 0; i < info.weights.size(); i++)
			{
				blendweight.elements[i] = info.weights[i].fValue;
				blendindices.uElements[i] = info.weights[i].index;
			}
			for (uint32 i = (uint32_t)info.weights.size(); i < uMaxWeights; i++)
			{
				blendindices.uElements[i] = 0;
				blendweight.elements[i] = 0.0f;
			}
			blendindices.uCount = uMaxWeights;
			blendweight.uCount = uMaxWeights;
			m_activeVerts[uVert].elements.push_back(blendindices);
			// Don't need weights for rigid skinning
			if (uMaxWeights > 1)
			{
				m_activeVerts[uVert].elements.push_back(blendweight);
			}
		}

		m_activeVerts[uVert].CalculateHash();
	}
}

void FbxLoad::GetUV(FbxMesh* pMesh, int iCtrlPoint, int iTexUVIndex, int inUVLayer, FBXVertexElement& outUV)
{
	if (pMesh->GetElementUVCount() <= inUVLayer)
	{
		ASSERT(false);
	}
	FbxGeometryElementUV* vertexUV = pMesh->GetElementUV(inUVLayer);

	switch (vertexUV->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
		switch (vertexUV->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			outUV.elements[0] = (float)(vertexUV->GetDirectArray().GetAt(iCtrlPoint).mData[0]);
			outUV.elements[1] = 1.0f-(float)(vertexUV->GetDirectArray().GetAt(iCtrlPoint).mData[1]);
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexUV->GetIndexArray().GetAt(iCtrlPoint);
			outUV.elements[0] = (float)(vertexUV->GetDirectArray().GetAt(index).mData[0]);
			outUV.elements[1] = 1.0f - (float)(vertexUV->GetDirectArray().GetAt(index).mData[1]);
		}
		break;

		default:
			ASSERT(false);
			return;
		}
		break;

	case FbxGeometryElement::eByPolygonVertex:
		switch (vertexUV->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		case FbxGeometryElement::eIndexToDirect:
		{
			outUV.elements[0] = (float)(vertexUV->GetDirectArray().GetAt(iTexUVIndex).mData[0]);
			outUV.elements[1] = 1.0f-(float)(vertexUV->GetDirectArray().GetAt(iTexUVIndex).mData[1]);
		}
		break;

		default:
			ASSERT(false);
		}
		break;
	}
}

bool FbxLoad::GetColor(FbxMesh* pMesh, int iCtrlPoint, int inColorId, FBXVertexElement& outColor)
{
	if (pMesh->GetElementVertexColorCount() <= inColorId)
	{
		return false;
	}

	FbxGeometryElementVertexColor* vertexColor = pMesh->GetElementVertexColor();

	switch (vertexColor->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
		switch (vertexColor->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			outColor.elements[0] = (float)(vertexColor->GetDirectArray().GetAt(iCtrlPoint).mRed);
			outColor.elements[1] = (float)(vertexColor->GetDirectArray().GetAt(iCtrlPoint).mGreen);
			outColor.elements[2] = (float)(vertexColor->GetDirectArray().GetAt(iCtrlPoint).mBlue);
			outColor.elements[3] = (float)(vertexColor->GetDirectArray().GetAt(iCtrlPoint).mAlpha);
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexColor->GetIndexArray().GetAt(iCtrlPoint);
			outColor.elements[0] = (float)(vertexColor->GetDirectArray().GetAt(index).mRed);
			outColor.elements[1] = (float)(vertexColor->GetDirectArray().GetAt(index).mGreen);
			outColor.elements[2] = (float)(vertexColor->GetDirectArray().GetAt(index).mBlue);
			outColor.elements[3] = (float)(vertexColor->GetDirectArray().GetAt(index).mAlpha);

		}
		break;

		default:
			return false;
		}
		break;

	case FbxGeometryElement::eByPolygonVertex:
		switch (vertexColor->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		case FbxGeometryElement::eIndexToDirect:
		{
			outColor.elements[0] = (float)(vertexColor->GetDirectArray().GetAt(inColorId).mRed);
			outColor.elements[1] = (float)(vertexColor->GetDirectArray().GetAt(inColorId).mGreen);
			outColor.elements[2] = (float)(vertexColor->GetDirectArray().GetAt(inColorId).mBlue);
			outColor.elements[3] = (float)(vertexColor->GetDirectArray().GetAt(inColorId).mAlpha);
		}
		break;

		default:
			ASSERT(false);
			return false;
		}
		break;
	}

	return true;
}

bool FbxLoad::GetNormal(FbxMesh* pMesh, int iCtrlPoint, int iVertex, FBXVertexElement& outNormal)
{
	if (pMesh->GetElementNormalCount() < 1)
	{
		return false;
	}

	FbxGeometryElementNormal* vertexNormal = pMesh->GetElementNormal(0);
	switch (vertexNormal->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
		switch (vertexNormal->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			outNormal.elements[0] = (float)(vertexNormal->GetDirectArray().GetAt(iCtrlPoint).mData[0]);
			outNormal.elements[1] = (float)(vertexNormal->GetDirectArray().GetAt(iCtrlPoint).mData[1]);
			outNormal.elements[2] = (float)(vertexNormal->GetDirectArray().GetAt(iCtrlPoint).mData[2]);
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexNormal->GetIndexArray().GetAt(iCtrlPoint);
			outNormal.elements[0] = (float)(vertexNormal->GetDirectArray().GetAt(index).mData[0]);
			outNormal.elements[1] = (float)(vertexNormal->GetDirectArray().GetAt(index).mData[1]);
			outNormal.elements[2] = (float)(vertexNormal->GetDirectArray().GetAt(index).mData[2]);
		} 
		break;

		default:
			return false;
		}
		break;

	case FbxGeometryElement::eByPolygonVertex:
		switch (vertexNormal->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			outNormal.elements[0] = (float)(vertexNormal->GetDirectArray().GetAt(iVertex).mData[0]);
			outNormal.elements[1] = (float)(vertexNormal->GetDirectArray().GetAt(iVertex).mData[1]);
			outNormal.elements[2] = (float)(vertexNormal->GetDirectArray().GetAt(iVertex).mData[2]);
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexNormal->GetIndexArray().GetAt(iVertex);
			outNormal.elements[0] = (float)(vertexNormal->GetDirectArray().GetAt(index).mData[0]);
			outNormal.elements[1] = (float)(vertexNormal->GetDirectArray().GetAt(index).mData[1]);
			outNormal.elements[2] = (float)(vertexNormal->GetDirectArray().GetAt(index).mData[2]);
		}
		break;

		default:
			return false;	// Invalid normal
		}
		break;
	}

	return true;
}

bool FbxLoad::GetBinormal(FbxMesh* pMesh, int iCtrlPoint, int iVertex, FBXVertexElement& outBinormal)
{
	if (pMesh->GetElementBinormalCount() < 1)
	{
		return false;
	}

	FbxGeometryElementBinormal* vertexBinormal = pMesh->GetElementBinormal(0);
	switch (vertexBinormal->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
		switch (vertexBinormal->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			outBinormal.elements[0] = (float)(vertexBinormal->GetDirectArray().GetAt(iCtrlPoint).mData[0]);
			outBinormal.elements[1] = (float)(vertexBinormal->GetDirectArray().GetAt(iCtrlPoint).mData[1]);
			outBinormal.elements[2] = (float)(vertexBinormal->GetDirectArray().GetAt(iCtrlPoint).mData[2]);
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexBinormal->GetIndexArray().GetAt(iCtrlPoint);
			outBinormal.elements[0] = (float)(vertexBinormal->GetDirectArray().GetAt(index).mData[0]);
			outBinormal.elements[1] = (float)(vertexBinormal->GetDirectArray().GetAt(index).mData[1]);
			outBinormal.elements[2] = (float)(vertexBinormal->GetDirectArray().GetAt(index).mData[2]);
		}
		break;

		default:
			return false;
		}
		break;

	case FbxGeometryElement::eByPolygonVertex:
		switch (vertexBinormal->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			outBinormal.elements[0] = (float)(vertexBinormal->GetDirectArray().GetAt(iVertex).mData[0]);
			outBinormal.elements[1] = (float)(vertexBinormal->GetDirectArray().GetAt(iVertex).mData[1]);
			outBinormal.elements[2] = (float)(vertexBinormal->GetDirectArray().GetAt(iVertex).mData[2]);
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexBinormal->GetIndexArray().GetAt(iVertex);
			outBinormal.elements[0] = (float)(vertexBinormal->GetDirectArray().GetAt(index).mData[0]);
			outBinormal.elements[1] = (float)(vertexBinormal->GetDirectArray().GetAt(index).mData[1]);
			outBinormal.elements[2] = (float)(vertexBinormal->GetDirectArray().GetAt(index).mData[2]);
		}
		break;

		default:
			return false;
		}
		break;
	}

	return true;
}

bool FbxLoad::GetTangent(FbxMesh* pMesh, int iCtrlPoint, int iVertex, FBXVertexElement& outTangent)
{
	if (pMesh->GetElementTangentCount() < 1)
	{
		return false;
	}

	FbxGeometryElementTangent* pVertexTangent = pMesh->GetElementTangent(0);
	switch (pVertexTangent->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
		switch (pVertexTangent->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			outTangent.elements[0] = (float)(pVertexTangent->GetDirectArray().GetAt(iCtrlPoint).mData[0]);
			outTangent.elements[1] = (float)(pVertexTangent->GetDirectArray().GetAt(iCtrlPoint).mData[1]);
			outTangent.elements[2] = (float)(pVertexTangent->GetDirectArray().GetAt(iCtrlPoint).mData[2]);
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int index = pVertexTangent->GetIndexArray().GetAt(iCtrlPoint);
			outTangent.elements[0] = (float)(pVertexTangent->GetDirectArray().GetAt(index).mData[0]);
			outTangent.elements[1] = (float)(pVertexTangent->GetDirectArray().GetAt(index).mData[1]);
			outTangent.elements[2] = (float)(pVertexTangent->GetDirectArray().GetAt(index).mData[2]);
		}
		break;

		default:
			return false;
		}
		break;

	case FbxGeometryElement::eByPolygonVertex:
		switch (pVertexTangent->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			outTangent.elements[0] = (float)(pVertexTangent->GetDirectArray().GetAt(iVertex).mData[0]);
			outTangent.elements[1] = (float)(pVertexTangent->GetDirectArray().GetAt(iVertex).mData[1]);
			outTangent.elements[2] = (float)(pVertexTangent->GetDirectArray().GetAt(iVertex).mData[2]);
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int index = pVertexTangent->GetIndexArray().GetAt(iVertex);
			outTangent.elements[0] = (float)(pVertexTangent->GetDirectArray().GetAt(index).mData[0]);
			outTangent.elements[1] = (float)(pVertexTangent->GetDirectArray().GetAt(index).mData[1]);
			outTangent.elements[2] = (float)(pVertexTangent->GetDirectArray().GetAt(index).mData[2]);
		}
		break;

		default:
			return false;
		}
		break;
	}

	return true;
}


uint32 FbxLoad::FindBone(uint32 uOriginalIndex, const vector<BoneInfo>& boneInfo)
{
	uint32 i = 0;
	for (auto itr = boneInfo.begin(); itr != boneInfo.end(); ++itr)
	{
		if ((*itr).uOriginalMapping == uOriginalIndex)
		{
			return i;
		}
		i++;
	}
	return 0;
}


void FbxLoad::PostProcessing(Cmdl& cmdl)
{
	// Add the bone information to the model
	{
		std::vector<uint8>& vecRigid = cmdl.GetRigidIndices();
		vecRigid.clear();
		for (auto itr = m_rigidBoneIndices.begin(); itr != m_rigidBoneIndices.end(); ++itr)
		{
			vecRigid.push_back((*itr).uNewMapping);
		}
	}

	{
		std::vector<uint8>& vecSmooth = cmdl.GetSmoothIndices();
		vecSmooth.clear();
		for (auto itr = m_skinnedBoneIndices.begin(); itr != m_skinnedBoneIndices.end(); ++itr)
		{
			vecSmooth.push_back((*itr).uNewMapping);
		}
	}

	// TODO: Re-engage when using model matrix set
	// Fixup the bone index streams to point to the correct data
	const uint32 uShapeNum = cmdl.GetShapeNum();
	for (uint32 i = 0; i < uShapeNum; ++i)
	{
		::exchange::Shape* pShape = cmdl.GetShapePtr(i);

		const ::exchange::PrimitiveInfo& info = pShape->GetPrimitiveInfo();
		usg::exchange::VertexStreamInfo* pStreamInfo = pShape->GetVertexStreamInfoOfType(usg::exchange::VertexAttribute_BONE_INDEX);
		if (pStreamInfo)
		{
			vector<BoneInfo> &boneSet = info.eSkinType == usg::exchange::SkinningType_SMOOTH_SKINNING ? m_skinnedBoneIndices : m_rigidBoneIndices;
			::exchange::Stream* pStream = cmdl.GetStreamPtr(pStreamInfo->refIndex);
			ASSERT(pStream->GetElementSize() == 1);	// Only allowing uint8 for the bone indices
			uint8* pData = (uint8*)pStream->GetStreamArrayPtr();
			uint32 uIndices = pStream->GetLength();
			for (uint32 index = 0; index < uIndices; index++)
			{
				pData[index] = (uint8)FindBone(pData[index], boneSet);
			}
		}
	}

	uint32_t uMeshCount = cmdl.GetMeshNum();
	for (uint32_t i = 0; i < uMeshCount; ++i)
	{
		::exchange::Mesh* pMesh = cmdl.GetMeshPtr(i);
		if (pMesh->GetMaterialRefIndex() == UINT32_MAX || pMesh->GetShapeRefIndex() == UINT32_MAX)
		{
			continue;
		}

		::exchange::Material* pMaterial = cmdl.GetMaterialPtr(pMesh->GetMaterialRefIndex());
		::exchange::Shape* pShape = cmdl.GetShapePtr(pMesh->GetShapeRefIndex());


		// Vertex alpha
		if (pShape->HasVertexAlpha())
		{
			pMaterial->SetVariable("bVertexAlpha", true);
		}

	}

	uint32 uMaterialCount = cmdl.GetMaterialNum();
	for (uint32 i = 0; i < uMaterialCount; i++)
	{
		::exchange::Material* pMaterial = cmdl.GetMaterialPtr(i);

		m_pOverrides->ApplyOverrides(pMaterial->pb().materialName, pMaterial);
	}

	// Calculate adjacency
	SetupAdjacencyStream(cmdl);

	// Shadow receiver
	//_setupShadowReceiver(cmdl);
}


void FbxLoad::SetupAdjacencyStream(Cmdl& cmdl)
{
	uint32_t uShapeCount = cmdl.GetShapeNum();
	for (uint32_t i = 0; i < uShapeCount; ++i)
	{
		::exchange::Shape* pShape = cmdl.GetShapePtr(i);

		// Number of vertices
		::exchange::Stream* pPositionStream = cmdl.GetStreamPtr(pShape->GetPositionStreamRefIndex());
		uint32_t vertexNum = pPositionStream->GetLength() / pPositionStream->GetColumnNum();

		::exchange::PrimitiveInfo& primInfo = pShape->GetPrimitiveInfo();

		::exchange::Stream* pAdjacencyStream = vnew(ALLOC_OBJECT) ::exchange::Stream();
		::exchange::Stream* pIndexStream = cmdl.GetStreamPtr(primInfo.indexStreamRefIndex);
		pAdjacencyStream->allocate<uint32_t>(pIndexStream->GetLength(), pIndexStream->GetColumnNum());

		// set adjacency stream reference index
		primInfo.adjacencyStreamRefIndex = cmdl.GetStreamNum();
		cmdl.AddStream(pAdjacencyStream);
	}
}

bool FbxLoad::PostProcessSkeleton(Cmdl& cmdl)
{
	bool bWrong = false;
	::exchange::Skeleton* pSkeleton = cmdl.GetSkeleton();

	// Hook up all of the bones with their d pose information
	::exchange::Skeleton::VectorBone::iterator itr = pSkeleton->Bones().begin();
	for (; itr != pSkeleton->Bones().end(); ++itr)
	{
		pSkeleton->FillOutBindPoseMatrix((*itr));
	}


	// PASS 1 : Setup bone name of Primitive
	// Traverse all shapes
	// FIXME: Remove all this as it will be redundant
	const uint32_t uShapeCount = cmdl.GetShapeNum();
	for (uint32_t i = 0; i < uShapeCount; ++i)
	{
		::exchange::Shape* pShape = cmdl.GetShapePtr(i);

		// Setup all primitives' bounding sphere
		::exchange::PrimitiveInfo& primInfo = pShape->GetPrimitiveInfo();

		primInfo.rootBoneIndex = FindBone(*cmdl.GetSkeleton(), primInfo.rootBoneName);

		uint32_t boneIndex = primInfo.rootBoneIndex;

		usg::exchange::Bone& bone = pSkeleton->Bones().at(boneIndex);
		strncpy(primInfo.rootBoneName, bone.name, sizeof(primInfo.rootBoneName) - 1);

		// If its bone is LOD level 1 or above, replace as the bone of LOD level 0.
		char* p = NULL;
		if ((p = strstr(bone.name, "_LOD")) != NULL)
		{
			if (p[4] < '0' || '9' < p[4])
			{
				printf("This node named \"%s\" doesn't have correct LOD level!\n", bone.name);
				bWrong = true;
				break;
			}

			char valueChar[] = { p[4], '\0' };
			int lodLevel = atoi(valueChar);

			if (lodLevel >= 1)
			{
				// Overwrite LOD level to remove the _LOD
				// bone index will be changed after. That's why It's not changed here.
				primInfo.rootBoneName[p - bone.name] = '\0';
				primInfo.lodLevel = lodLevel;
			}
		}
	}

	if (bWrong) return false;

	// PASS 2 : remove bones that is LOD level 1 or above
	itr = pSkeleton->Bones().begin();
	while (itr != pSkeleton->Bones().end())
	{
		int lodLevel = 0;

		const char* p = NULL;
		if ((p = strstr((*itr).name, "LOD")) != NULL)
		{
			char valueChar[] = { p[3], '\0' };
			lodLevel = atoi(valueChar);
		}

		if (lodLevel >= 1)
		{
			// Erase and Next
			itr = pSkeleton->Bones().erase(itr);
		}
		else
		{
			++itr;
		}
	}


	// PASS 3 : 
	// Traverse all shapes again
	for (uint32_t i = 0; i < uShapeCount; ++i)
	{
		::exchange::Shape* pShape = cmdl.GetShapePtr(i);

		::exchange::PrimitiveInfo& primInfo = pShape->GetPrimitiveInfo();

		// At this moment, Primitive's bone index is INVALID. Must re-set with correct index by name search.
		primInfo.rootBoneIndex = pSkeleton->SearchBone(primInfo.rootBoneName);
		uint32_t boneIndex = primInfo.rootBoneIndex;
		// TODO
		usg::exchange::Bone& bone = pSkeleton->Bones().at(boneIndex);

		::usg::exchange::Sphere tempSphere = pShape->pb().boundingSphere;

		// If skinned the vertices will have been in bind pose location
		if (pShape->GetSkinningType() != usg::exchange::SkinningType_NO_SKINNING)
		{
			tempSphere.center = bone.invBindPoseTrans.TransformVec3(tempSphere.center);
		}

		if (bone.boundingSphere.center.Magnitude() == 0.0f && bone.boundingSphere.radius == 0.0f)
		{
			bone.boundingSphere.center = tempSphere.center;
			bone.boundingSphere.radius = tempSphere.radius;
		}
		else
		{
			LoaderUtil::uniteSphere(bone.boundingSphere.center, bone.boundingSphere.radius,
				bone.boundingSphere.center, bone.boundingSphere.radius,
				tempSphere.center, tempSphere.radius);
		}
	}


	// Remap all of the bones to the correct ids
	for (auto itr = m_rigidBoneIndices.begin(); itr != m_rigidBoneIndices.end(); ++itr)
	{
		(*itr).uNewMapping = pSkeleton->SearchBone((*itr).szName);
	}

	for (auto itr = m_skinnedBoneIndices.begin(); itr != m_skinnedBoneIndices.end(); ++itr)
	{
		(*itr).uNewMapping = pSkeleton->SearchBone((*itr).szName);
	}

	pSkeleton->pb().bonesNum = (uint32)pSkeleton->Bones().size();

	return true;
}



// This function is that uses up all the time - maybe need to consider a map... 
void FbxLoad::RemoveDuplicateVertices()
{
	// First get a list of unique vertices
	usg::vector<TempVertex> uniqueVertices;
	usg::map<uint64, uint32> uniqueMap;
	usg::vector<TempVertex>::iterator it;
	usg::vector<TempVertex>::iterator end = m_activeVerts.end();
	for (it =  m_activeVerts.begin(); it!= end; ++it)
	{
		uint64 cmp = (*it).cmpValue;
		if( uniqueMap.find(cmp) == uniqueMap.end())
		{
			uniqueMap[cmp] = (uint32)uniqueVertices.size();
			uniqueVertices.push_back(*it);
		}
	}

	// Now we update the index buffer
	usg::vector<uint32>::iterator indicesIt = m_indicesTmp.begin();
	for (it = m_activeVerts.begin(); it != end; ++it)
	{
		uint64 cmpValue = (*it).cmpValue;
		*indicesIt++ = uniqueMap[cmpValue];
	}

	m_activeVerts.clear();
	m_activeVerts = uniqueVertices;
	uniqueVertices.clear();

	// Should already be seperated into seperate meshes per material via the utilities
}

void FbxLoad::AddMaterials(Cmdl& cmdl, FbxNode* pNode, bool bSkinned)
{
	uint32 uMaterialCount = pNode->GetMaterialCount();
	m_bHasNormalMap = false;
	for (uint32 i = 0; i < uMaterialCount; ++i)
	{
		FbxSurfaceMaterial* surfaceMaterial = pNode->GetMaterial(i);
		::exchange::Material* pNewMaterial = NewMaterial(surfaceMaterial, bSkinned);
		cmdl.AddMaterial(pNewMaterial);
	}

	if (uMaterialCount == 0)
	{
		// Create a dummy material
		::exchange::Material* pNewMaterial = DummyMaterial(bSkinned);
		cmdl.AddMaterial(pNewMaterial);
	}
}





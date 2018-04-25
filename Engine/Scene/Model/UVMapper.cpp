/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Resource/ModelResource.h"
#include "Engine/Maths/MathUtil.h"
#include "Engine/Graphics/Effects/ConstantSet.h"
#include "Engine/Scene/Common/CustomEffectRuntime.h"
#include "UVMapper.h"


namespace usg
{



UVMapper::UVMapper()
{
	m_bValid = false;
}

UVMapper::~UVMapper()
{

}

void UVMapper::Init(uint32 uUVIndex,  MappingMethod eMethod, CustomEffectRuntime* pCustomFX, const Vector2f& vTranslation, const Vector2f& scale, float fRotation)
{
	m_pCustomFX = pCustomFX;
	m_eMappingMethod = eMethod;
	m_uUVIndex = uUVIndex;
	m_vScale = scale;
	m_vInvScale2.Assign(1.0f/scale.x*2.f, 1.0f/scale.y*2.f);
	m_vTranslation = vTranslation;
	m_fRotation = fRotation;
	m_bDirty = true;
	m_bValid = true;
}

void UVMapper::UploadTextureMatrix(const Matrix4x3& mMat)
{
	m_pCustomFX->SetVariable("mTexMatrix", mMat, m_uUVIndex);
}

void UVMapper::UpdateBaseMat()
{
	Matrix4x3& out = m_textureMat;
	float rotateSin;
	float rotateCos;
	Math::SinCos(m_fRotation, rotateSin, rotateCos);

	out.M[0][0] =  m_vScale.x * rotateCos;
	out.M[0][1] = -m_vScale.x * rotateSin;
	out.M[0][3] =  m_vScale.x * ( 0.5f * rotateSin - 0.5f * rotateCos + 0.5f - m_vTranslation.x);

	out.M[1][0] = m_vScale.y * rotateSin;
	out.M[1][1] = m_vScale.y * rotateCos;
	out.M[1][3] = m_vScale.y * (-0.5f * rotateSin - 0.5f * rotateCos + 0.5f - m_vTranslation.y);

	out.M[0][2] = 0.0f;
	out.M[1][2] = 0.0f;

	out.M[2][0] = 0.0f;
	out.M[2][1] = 0.0f;
	out.M[2][3] = 0.0f;
	out.M[2][2] = 1.0f;
}

void UVMapper::UpdateInt()
{
	UpdateBaseMat();

	switch(m_eMappingMethod)
	{
	case usg::exchange::TextureCoordinator_MappingMethod_PROJECTION:
		{

		}
		break;
		case usg::exchange::TextureCoordinator_MappingMethod_UV_COORDINATE:
		case usg::exchange::TextureCoordinator_MappingMethod_SPHERE_ENV:
			{
				if(m_bDirty)
				{
					UploadTextureMatrix(m_textureMat);
				}
				break;
			}
		case usg::exchange::TextureCoordinator_MappingMethod_CUBE_ENV:
			{
				 // Do nothing, we've adjusted the shader to use the inverse view matrix
				
				break;
			}
	
		default:
			ASSERT(false);		
	}

	m_bDirty = false;
}

void UVMapper::SetUVTranslation(const Vector2f& vTranslation)
{
	if( !m_vTranslation.IsEqual(vTranslation) )	
	{
		m_vTranslation = vTranslation;

		if(fabsf(m_vTranslation.x) > m_vInvScale2.x)
		{
			m_vTranslation.x -= Math::Sign(m_vTranslation.x)*m_vInvScale2.x;
		}

		if(fabsf(m_vTranslation.y) > m_vInvScale2.y)
		{
			m_vTranslation.y -= Math::Sign(m_vTranslation.x)*m_vInvScale2.y;
		}

		m_bDirty = true;
	}
}


void UVMapper::SetUVRotation(float fRotation)
{
	if( !Math::IsEqual(m_fRotation, fRotation) )
	{
		m_fRotation = fRotation;
		if(fabsf(m_fRotation) > Math::two_pi)
		{
			m_fRotation -= Math::Sign(m_fRotation)*(Math::two_pi*2.f);
		}
		m_bDirty = true;
	}
}


void UVMapper::SetScale( const Vector2f& scale )
{
	if( !m_vScale.IsEqual( scale ) ) {
		m_vScale = scale;
		m_vInvScale2.Assign( 1.0f / scale.x*2.f, 1.0f / scale.y*2.f );

		m_bDirty = true;
	}
}

void UVMapper::SetAnimFrame( float fFrame )
{
	// TODO: Implement me
}


}
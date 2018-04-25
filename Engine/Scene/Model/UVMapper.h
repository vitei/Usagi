/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Class for updating a models UV set
*****************************************************************************/
#ifndef _USG_GRAPHICS_SCENE_UVMAPPER_H_
#define _USG_GRAPHICS_SCENE_UVMAPPER_H_

#include "Engine/Common/Common.h"
#include "Engine/Scene/Camera/Camera.h"
#include "Engine/Graphics/Materials/Material.pb.h"
#include "Engine/Scene/Common/CustomEffectRuntime.h"

namespace usg {

	class ConstantSet;

	class UVMapper
	{
	public:
		typedef usg::exchange::_TextureCoordinator_MappingMethod MappingMethod;

		UVMapper();
		virtual ~UVMapper();
	
		void Init(uint32 uUVIndex, MappingMethod eMethod, CustomEffectRuntime* pCustomFX, const Vector2f& vTranslation, const Vector2f& scale, float fRotation);
		void SetUVTranslation(const Vector2f& vTranslation);
		void AddUVTranslation(const Vector2f& vTranslation) { SetUVTranslation(vTranslation + GetUVTranslation()); }
		const Vector2f& GetUVTranslation() { return m_vTranslation; }
		void SetUVRotation(float fRotation);
		void AddUVRotation(float fRotation) { SetUVRotation(fRotation+GetUVRotation()); }
		float GetUVRotation() { return m_fRotation; }
		void SetScale( const Vector2f& scale );
		bool Update() { if(m_bDirty) { UpdateInt(); return true; } return false; }

		void SetAnimFrame(float f);
		inline bool IsValid() { return m_bValid; }
		CustomEffectRuntime* GetCustomEffect() { return m_pCustomFX; }
	private:
		bool TextureMatDirty() { return m_bDirty; }
		void UploadTextureMatrix(const Matrix4x3& mMat);
		void UpdateBaseMat();
		void UpdateInt();

		bool			m_bValid;
		MappingMethod	m_eMappingMethod;
		CustomEffectRuntime*	m_pCustomFX;
		uint32			m_uUVIndex;
		Matrix4x3		m_textureMat;
		MappingMethod	m_eMethod;
		Vector2f		m_vInvScale2;
		Vector2f		m_vTranslation;
		Vector2f		m_vScale;
		float			m_fRotation;
		bool			m_bDirty;
	};




}

#endif

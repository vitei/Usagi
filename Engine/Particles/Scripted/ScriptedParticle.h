/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_PARTICLES_SCRIPT_SCRIPTED_PARTICLE_H
#define _USG_PARTICLES_SCRIPT_SCRIPTED_PARTICLE_H
#include "Engine/Common/Common.h"
#include "Engine/Maths/Vector3f.h"
#include "Engine/Graphics/Color.h"
#include "Engine/Graphics/Effects/ConstantSet.h"
#include "Engine/Graphics/Primitives/VertexDeclaration.h"
#include "Engine/Particles/Scripted/ScriptEmitter.pb.h"

namespace usg
{

	namespace Particle 
	{
		struct ScriptedParticle
		{
			Vector3f	vPos;
			Vector2f	vUVOffset;
			Vector2f	vSizeBase;
			Vector3f	vVelocity;
			float32		fColorIndex;
			float		fLifeStart, fInvLife;
			float		fRotStart, fRotSpeed, fRotAttenuation;
		};

		struct ScriptedTrailParticle
		{
			enum
			{
				POS_COUNT = 6
			};
			Vector3f	vPos[POS_COUNT];
			Vector2f	vWidth;
			float32		fColorIndex;
			float		fLifeStart, fInvLife;
		};

		struct ScriptedMetaData
		{
			uint32 uRandom[particles::EmitterEmission::textureData_max_count];
			float32 fElapsed;
		};

		// The simulation for trails is mostly done on the CPU for now since we don't have any better options
		// on the 3DS
		struct ScriptedTrailMetaData
		{
			Vector3f	vVelocity;
		};
		
		static const VertexElement g_scriptedParticleVertexElements[] =
		{
			VERTEX_DATA_ELEMENT_NAME(0, ScriptedParticle, vPos, VE_FLOAT, 3, false),
			VERTEX_DATA_ELEMENT_NAME(2, ScriptedParticle, vUVOffset, VE_FLOAT, 2, false),
			VERTEX_DATA_ELEMENT_NAME(1, ScriptedParticle, vSizeBase, VE_FLOAT, 2, false),
			VERTEX_DATA_ELEMENT_NAME(3, ScriptedParticle, vVelocity, VE_FLOAT, 3, false),
			VERTEX_DATA_ELEMENT_NAME(4, ScriptedParticle, fColorIndex, VE_FLOAT, 1, false),
			VERTEX_DATA_ELEMENT_NAME(5, ScriptedParticle, fLifeStart, VE_FLOAT, 2, false),
			VERTEX_DATA_ELEMENT_NAME(6, ScriptedParticle, fRotStart, VE_FLOAT, 3, false),
			VERTEX_DATA_END()
		};


		struct ScriptedParticleConstants
		{
			Vector4f	vGravityDir;
			Color		vColorSet[3];
			Vector4f	vColorTiming;	// In, peak, out, repetition
			Vector4f	vAlphaValues;	// Initial, intermediate, ending alpha
			Vector2f 	vAlphaTiming;	// In, out
			Vector4f	vScaling;		// Start, intermediate, end
			Vector2f	vScaleTiming;	// In, out
			Vector2f	vUVScale[2];
			float		fColorBehaviour;
			float		fAirResistance;
			float		fUpdatePosition;	// 0 to use the CPU, 1 to calculate internally
			float		fColorAnimRepeat;
			Color		vEnvColor;
			bool		bLocalEffect;
		};

		struct ScriptedParticlePerFrame
		{
			float		fEffectTime;
		};

		struct ScriptedParticleGSTrans
		{
			Matrix4x3	mOrientation;
			bool		bCustomMatrix;
			bool		bYAxisAlign;
		};

		struct ScriptedParticleFragment
		{
			float		fAlphaRef;
			float		fDepthFade;
		};


		static const ShaderConstantDecl g_scriptedParticlePerEffectDecl[] =
		{
			SHADER_CONSTANT_ELEMENT(ScriptedParticleConstants, vGravityDir,			CT_VECTOR_4, 1),
			SHADER_CONSTANT_ELEMENT(ScriptedParticleConstants, vColorSet,			CT_VECTOR_4, 3),
			SHADER_CONSTANT_ELEMENT(ScriptedParticleConstants, vColorTiming,		CT_VECTOR_4, 1),
			SHADER_CONSTANT_ELEMENT(ScriptedParticleConstants, vAlphaValues,		CT_VECTOR_4, 1),
			SHADER_CONSTANT_ELEMENT(ScriptedParticleConstants, vAlphaTiming,		CT_VECTOR_2, 1),
			SHADER_CONSTANT_ELEMENT(ScriptedParticleConstants, vScaling,			CT_VECTOR_4, 1),
			SHADER_CONSTANT_ELEMENT(ScriptedParticleConstants, vScaleTiming,		CT_VECTOR_2, 1),
			SHADER_CONSTANT_ELEMENT(ScriptedParticleConstants, vUVScale[0],			CT_VECTOR_2, 2),
			SHADER_CONSTANT_ELEMENT(ScriptedParticleConstants, fColorBehaviour,		CT_FLOAT, 1),
			SHADER_CONSTANT_ELEMENT(ScriptedParticleConstants, fAirResistance,		CT_FLOAT, 1),
			SHADER_CONSTANT_ELEMENT(ScriptedParticleConstants, fUpdatePosition,		CT_FLOAT, 1),
			SHADER_CONSTANT_ELEMENT(ScriptedParticleConstants, fColorAnimRepeat,	CT_FLOAT, 1),
			SHADER_CONSTANT_ELEMENT(ScriptedParticleConstants, vEnvColor,			CT_VECTOR_4, 1),
			SHADER_CONSTANT_ELEMENT(ScriptedParticleConstants, bLocalEffect,		CT_BOOL, 1),
			SHADER_CONSTANT_END()
		};

		static const ShaderConstantDecl g_scriptedParticleGSTrans[] =
		{
			SHADER_CONSTANT_ELEMENT(ScriptedParticleGSTrans, mOrientation,	CT_MATRIX_43, 1),
			SHADER_CONSTANT_ELEMENT(ScriptedParticleGSTrans, bCustomMatrix,	CT_BOOL, 1),
			SHADER_CONSTANT_ELEMENT(ScriptedParticleGSTrans, bYAxisAlign,	CT_BOOL, 1),
			SHADER_CONSTANT_END()
		};

		static const ShaderConstantDecl g_scriptedParticlePerFrame[] =
		{
			SHADER_CONSTANT_ELEMENT(ScriptedParticlePerFrame, fEffectTime,		CT_FLOAT, 1),
			SHADER_CONSTANT_END()
		};

		static const ShaderConstantDecl g_scriptedFragmentDecl[] =
		{
			SHADER_CONSTANT_ELEMENT(ScriptedParticleFragment, fAlphaRef, CT_FLOAT, 1),
			SHADER_CONSTANT_ELEMENT(ScriptedParticleFragment, fDepthFade, CT_FLOAT, 1),
			SHADER_CONSTANT_END()
		};

		static const DescriptorDeclaration g_scriptedDescriptorDecl[] =
		{
			DESCRIPTOR_ELEMENT(0,							DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),
			DESCRIPTOR_ELEMENT(SHADER_CONSTANT_MATERIAL,	DESCRIPTOR_TYPE_CONSTANT_BUFFER,		1, SHADER_FLAG_VERTEX),
			DESCRIPTOR_ELEMENT(SHADER_CONSTANT_CUSTOM_3,		DESCRIPTOR_TYPE_CONSTANT_BUFFER,		1, SHADER_FLAG_VERTEX),
			DESCRIPTOR_ELEMENT(SHADER_CONSTANT_CUSTOM_0,	DESCRIPTOR_TYPE_CONSTANT_BUFFER,			1, SHADER_FLAG_VERTEX),
			DESCRIPTOR_ELEMENT(SHADER_CONSTANT_CUSTOM_1,	DESCRIPTOR_TYPE_CONSTANT_BUFFER,		1, SHADER_FLAG_GEOMETRY),
			DESCRIPTOR_ELEMENT(SHADER_CONSTANT_MATERIAL_1,	DESCRIPTOR_TYPE_CONSTANT_BUFFER,		1, SHADER_FLAG_PIXEL),
			DESCRIPTOR_END()
		};
	}
}

#endif

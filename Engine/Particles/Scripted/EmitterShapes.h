/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_PARTICLES_SCRIPT_EMITTER_SHAPES_H_
#define _USG_PARTICLES_SCRIPT_EMITTER_SHAPES_H_
#include "Engine/Common/Common.h"
#include "Engine/Particles/ParticleEmitter.h"
#include "Engine/Core/ProtocolBuffers/ProtocolBufferFile.h"
#include "Engine/Maths/MathUtil.h"
#include "Engine/Particles/Scripted/ScriptedParticle.h"
#include "Engine/Particles/Scripted/EmitterShapes.pb.h"
#include "Engine/Particles/Scripted/ScriptEmitter.pb.h"

namespace usg
{
	class ScriptEmitter;
	
	class EmitterShape
	{
	public:
		EmitterShape();
		virtual ~EmitterShape();

		static EmitterShape* CreateShape(particles::EmitterShape eShape, const particles::EmitterShapeDetails& shapeDetails);

		void Init(const particles::EmitterShapeDetails& details);
		virtual void FillEmissionParams(Vector3f& vPosOut, Vector3f& vVelocityOut) const;

		void ApplyXZVelocityDispersion(const Vector3f& vPos, Vector3f &vVelocityOut) const;
		const particles::EmitterShapeDetails& GetDetails() const { return m_shapeDetails; }
	protected:
		virtual void FillEmissionParamsInt(Vector3f& vPosOut, Vector3f& vVelocityOut) const {};

		virtual void LoadAdditional() {}

		particles::EmitterShapeDetails		m_shapeDetails;
		particles::EmitterShapeBase			m_shapeDef;
	};

}

#endif // _USG_PARTICLES_SCRIPT_EMITTER_H_

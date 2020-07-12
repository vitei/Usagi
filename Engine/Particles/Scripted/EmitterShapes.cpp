/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Particles/Scripted/EmitterShapes.h"
#include <float.h>

namespace usg
{

	// None of these specific classes need to be visible outside of here
	class EmitterShapeArc
	{
	public:
		EmitterShapeArc();
		~EmitterShapeArc();

		void Load(usg::particles::EmitterArc& arc);

		float GetRandomArcRotation(float32 &sinOut, float32 &cosOut) const
		{
			float32 fRot = Math::RangedRandom(m_fStartAngleRad, m_fEndAngleRad);
			Math::SinCos(fRot, sinOut, cosOut);
			return fRot;
		}

	private:
		float	m_fStartAngleRad;
		float	m_fEndAngleRad;
	};

	class PointEmitter : public EmitterShape
	{
	public:
		PointEmitter() {}
		virtual ~PointEmitter() {}

	private:
		virtual void FillEmissionParamsInt(Vector3f& vPosOut, Vector3f& vVelocityOut) const;

	};


	class SphereEmitter : public EmitterShape
	{
	public:
		SphereEmitter() {}
		virtual ~SphereEmitter() {}

	protected:
		virtual void LoadAdditional();
		virtual void FillEmissionParamsInt(Vector3f& vPosOut, Vector3f& vVelocityOut) const;

		EmitterShapeArc				m_arc;
	};

	class CylinderEmitter : public EmitterShape
	{
	public:
		CylinderEmitter() {}
		virtual ~CylinderEmitter() {}

	protected:
		virtual void FillEmissionParamsInt(Vector3f& vPosOut, Vector3f& vVelocityOut) const;
		virtual void LoadAdditional();

		EmitterShapeArc				m_arc;
	};

	class CubeEmitter : public EmitterShape
	{
	public:
		CubeEmitter() {}
		virtual ~CubeEmitter() {}

	protected:
		virtual void FillEmissionParamsInt(Vector3f& vPosOut, Vector3f& vVelocityOut) const;


	};


	EmitterShape* EmitterShape::CreateShape(particles::EmitterShape eShape, const particles::EmitterShapeDetails& shapeDetails)
	{
		EmitterShape* pShape = nullptr;
		switch (eShape)
		{
		case particles::EMITTER_SHAPE_POINT:
			pShape = vnew(ALLOC_PARTICLES) PointEmitter;
			break;
		case particles::EMITTER_SHAPE_SPHERE:
			pShape = vnew(ALLOC_PARTICLES) SphereEmitter;
			break;
		case particles::EMITTER_SHAPE_CYLINDER:
			pShape = vnew(ALLOC_PARTICLES) CylinderEmitter;
			break;
		case particles::EMITTER_SHAPE_CUBE:
			pShape = vnew(ALLOC_PARTICLES) CubeEmitter;
			break;
		default:
			// Not yet implemented
			ASSERT(false);
		}

		if (pShape)
		{
			pShape->Init(shapeDetails);
		}

		return pShape;
	}


	EmitterShape::EmitterShape()
	{

	}

	EmitterShape::~EmitterShape()
	{

	}

	void EmitterShape::Init(const particles::EmitterShapeDetails& details)
	{
		m_shapeDetails = details;
		m_shapeDef = details.baseShape;

		LoadAdditional();
	}

	void EmitterShape::FillEmissionParams(Vector3f& vPosOut, Vector3f& vVelocityOut) const
	{
		FillEmissionParamsInt(vPosOut, vVelocityOut);
	}


	EmitterShapeArc::EmitterShapeArc()
	{

	}

	EmitterShapeArc::~EmitterShapeArc()
	{

	}

	void EmitterShapeArc::Load(usg::particles::EmitterArc& arcDef)
	{
		m_fStartAngleRad = Math::DegreesToRadians(arcDef.fArcStartDeg);
		m_fEndAngleRad = Math::DegreesToRadians(arcDef.fArcStartDeg + arcDef.fArcWidthDeg);
	}


	void PointEmitter::FillEmissionParamsInt(Vector3f& vPosOut, Vector3f& vVelocityOut) const
	{
		vPosOut.Assign(0.0f, 0.0f, 0.0f);
		vVelocityOut.SphericalRandomVector(m_shapeDef.fShapeExpandVel);
	}


	void SphereEmitter::LoadAdditional()
	{
		m_arc.Load(m_shapeDetails.arc);
	}


	void SphereEmitter::FillEmissionParamsInt(Vector3f& vPosOut, Vector3f& vVelocityOut) const
	{
		float fSin, fCos;
		float fHollowness = m_shapeDef.fHollowness;
		m_arc.GetRandomArcRotation(fSin, fCos);

		float fHeight = Math::RangedRandom(-1.0f, 1.0f);
		float fR = Math::SqrtSafe( 1.0f - fHeight * fHeight );

		float fRadius = Math::RangedRandom(0.0f, 1.0f);
		fRadius = Math::SqrtSafe( fRadius );
		fRadius = fRadius * (1.0f-fHollowness) + fHollowness;
		
		Vector3f vDir(fR * fSin, fHeight, fR * fCos);

		vPosOut = (vDir * m_shapeDetails.vShapeExtents) * fRadius;
		
		float fBaseVelocity = m_shapeDef.fShapeExpandVel;
		vVelocityOut = vDir * fBaseVelocity;

}

	void CylinderEmitter::LoadAdditional()
	{
		m_arc.Load(m_shapeDetails.arc);
	}


	void CylinderEmitter::FillEmissionParamsInt(Vector3f& vPosOut, Vector3f& vVelocityOut) const
	{
		float32 fRand = Math::RangedRandom(0.0f, 1.0f);
		float32 inner = m_shapeDef.fHollowness;
		fRand = Math::SqrtSafe( fRand + inner * inner * ( 1.0f - fRand ) );

		float fHeight = Math::RangedRandom(-1.0f, 1.0f);
		float fSin, fCos;
		m_arc.GetRandomArcRotation(fSin, fCos);

		vPosOut.Assign(fSin, fHeight, fCos);
		vPosOut = vPosOut * fRand * m_shapeDetails.vShapeExtents;

		float fBaseVelocity = m_shapeDef.fShapeExpandVel;
		vVelocityOut.Assign(fSin * fBaseVelocity, 0.0f, fCos * fBaseVelocity);

	}


	void CubeEmitter::FillEmissionParamsInt(Vector3f& vPosOut, Vector3f& vVelocityOut) const
	{
		uint32 uSide = Math::RangedRandomUInt(0, 3);
		float fSign = Math::Sign(Math::RangedRandom(-1.f, 1.f));

		Vector3f vRandRang = Vector3f::RandomRange(Vector3f(-1.f), Vector3f(1.f));

		if (uSide < 1)
		{
			vPosOut.x = m_shapeDetails.vShapeExtents.x * vRandRang.x;
			vPosOut.y = m_shapeDetails.vShapeExtents.y * vRandRang.y;
			vPosOut.z = fSign * m_shapeDetails.vShapeExtents.z;

		}
		else if (uSide < 2)
		{
			vPosOut.x = m_shapeDetails.vShapeExtents.x * vRandRang.x;
			vPosOut.y = fSign * m_shapeDetails.vShapeExtents.y;
			vPosOut.z = m_shapeDetails.vShapeExtents.z * vRandRang.z;
		}
		else
		{
			vPosOut.x = fSign * m_shapeDetails.vShapeExtents.x;
			vPosOut.y = m_shapeDetails.vShapeExtents.y * vRandRang.y;
			vPosOut.z = m_shapeDetails.vShapeExtents.z * vRandRang.z;
		}

		// Velocity
		Vector3f vDir;
		if (vPosOut.MagnitudeSquared() != 0.0f)
		{
			vPosOut.GetNormalised(vDir);
		}
		else
		{
			vDir = Vector3f::ZERO;
		}

		float fBaseVelocity = m_shapeDef.fShapeExpandVel;
		vVelocityOut.x = vDir.x * fBaseVelocity;
		vVelocityOut.y = vDir.y * fBaseVelocity;
		vVelocityOut.z = vDir.z * fBaseVelocity;
	}

}
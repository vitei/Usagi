/****************************************************************************
 //	Usagi Engine, Copyright © Vitei, Inc. 2013
 //	Description: A 2D shape. Used for pathfidning
 *****************************************************************************/

#ifndef __USG_AI_SHAPE__
#define __USG_AI_SHAPE__


#include "Engine/Memory/MemUtil.h"
#include "Engine/Maths/Vector2f.h"
#include "AABB.h"
#include "Projection.h"

namespace usg
{

namespace ai
{

	// Helper function defined in Shape.cpp
	namespace shape_details {
		void UpdateAABB(AABB& aabb, uint64& uGridMask, const Vector2f& vCenter, const Vector2f* pVerts, uint32 uVertNum);
		uint64 ComputeGridMask(const AABB& aabb);
		uint64 ComputeGridMask(const Vector2f& vMin, const Vector2f& vMax);
		uint64 ComputeGridMask(const Vector2f& vPoint);
	}

struct Line
{
	Line():
		m_vFrom(0.0f, 0.0f),
		m_vTo(0.0f, 0.0f)
	{}

	Line(const Vector2f& vFrom, const Vector2f& vTo) :
		m_vFrom(vFrom),
		m_vTo(vTo)
	{}

	Vector2f m_vFrom;
	Vector2f m_vTo;
};

class IShape
{
protected:
	static const float Degrees;
	
	// Cohen-Sutherland
	static const int Inside		= 0;	//	0000
	static const int Left		= 1;	//	0001
	static const int Right		= 2;	//	0010
	static const int Bottom		= 4;	//	0100
	static const int Top		= 8;	//	1000
public:

	IShape() : 
		m_uGridMask((uint64)(-1)),
		m_vCenter(0.0f, 0.0f),
		m_fAngle(0.0f)
	{}
	virtual ~IShape(){}

	virtual uint32 GetNumVerts() const = 0;
	virtual void SetVerts(Vector2f* pVerts) = 0;

	virtual void Rotate(float fValue) = 0;
	virtual void SetRotation(float fValue) = 0;

	void Translate(const Vector2f& vV) { m_vCenter += vV; m_aabb.m_vPos += vV; };
	void SetCenter(const Vector2f& vCenter) { m_vCenter = vCenter; m_aabb.m_vPos = vCenter; };

	virtual void Scale(const Vector2f& vScale) = 0;
	virtual void SetScale(const Vector2f& vScale) = 0;

	virtual void GetAxes(usg::Vector2f* axes, const uint32 uSize) const = 0;
	virtual Projection Project(const Vector2f& vAxis) const = 0;

	virtual bool Intersects(const usg::Vector2f& vPoint) const = 0;
	virtual bool Intersects(const usg::ai::AABB& aabb) const = 0;
	virtual bool Intersects(const IShape* pShape) const = 0;
	virtual bool Intersects(const IShape* pShape, Vector2f& vHit) const = 0;
	virtual bool Intersects(const usg::ai::Line& line) const = 0;
	virtual bool Intersects(usg::ai::Line line, Vector2f& vHit) const = 0;

	// Returns the number of intersections found (due to convexity, at most 2).
	virtual int Intersects(const usg::ai::Line& line, Vector2f& vHit1, Vector2f& vHit2) const;

	virtual float GetSquaredDistanceToPoint(const Vector2f& vPosition) const = 0;

	virtual const Vector2f* GetVerts() const = 0;

	const usg::ai::AABB& GetAABB() const { return m_aabb; }
	const Vector2f& GetCenter() const { return m_vCenter; }
	const Vector2f& GetScale() const { return m_vScale; }
	float GetAngle() const { return m_fAngle; }

	uint64 GetGridMask() const {
		return m_uGridMask;
	}

protected:
	virtual void UpdateAABB() = 0;

	usg::ai::AABB m_aabb;
	uint64 m_uGridMask;
	Vector2f m_vCenter;
	Vector2f m_vScale;
	float m_fAngle;
};

template <uint32 t_uSize>
class Shape : public IShape
{
public:
	Shape(){}
	virtual ~Shape(){}

	virtual uint32 GetNumVerts() const { return t_uSize; }
	virtual void SetVerts(Vector2f* pVerts)
	{
		MemCpy(m_avVerts, pVerts, sizeof(Vector2f) * (size_t)t_uSize);
	}

	virtual void Rotate(float fValue)
	{
		float fTargetAngle = m_fAngle + fValue;
		const float fLimit = IShape::Degrees;

		if(fTargetAngle > fLimit)
		{
			fTargetAngle = (m_fAngle - fLimit);
		}
		else if(fTargetAngle < -fLimit)
		{
			float fV = m_fAngle * -1.0f;
			fTargetAngle = fLimit - fV;
		}

		_Rotate(m_fAngle - fTargetAngle);
		m_fAngle = fTargetAngle;
	}

	virtual void SetRotation(float fValue)
	{
		const float fAngleToZero = -m_fAngle;
		_Rotate(fAngleToZero);

		while (fValue < -180)
		{
			fValue += 360.0f;
		}
		while (fValue > 180)
		{
			fValue -= 360.0f;
		}

		_Rotate(fValue);
		m_fAngle = fValue;
	}

	virtual void GetAxes(usg::Vector2f* axes, const uint32 uSize) const
	{
		uint32 i, uC = t_uSize;
		for(i = 0; i < uC && i < uSize; i++)
		{
			const Vector2f vP1 = m_vCenter + m_avVerts[i];
			const Vector2f vP2 = m_vCenter + m_avVerts[((i + 1) == uC ? 0 : i + 1)];
			const Vector2f vEdge = vP1 - vP2;
			const Vector2f vNorm = Vector2f(vEdge.y, -vEdge.x);
			axes[i] = vNorm;
		}
	}

	virtual usg::ai::Projection Project(const Vector2f& vAxis) const
	{
		float fMin = vAxis.DotProduct(m_avVerts[0] + m_vCenter);
		float fMax = fMin;

		uint32 i, uC = t_uSize;
		for(i = 1; i < uC; i++)
		{
			float fV = vAxis.DotProduct(m_avVerts[i] + m_vCenter);

			if(fV < fMin)
			{
				fMin = fV;
			}
			else if(fV > fMax)
			{
				fMax = fV;
			}
		}

		return usg::ai::Projection(fMin, fMax);
	}

	virtual const Vector2f* GetVerts() const { return m_avVerts; }

	virtual bool Intersects(const Vector2f& vPoint) const;
	virtual bool Intersects(const usg::ai::AABB& aabb) const;
	virtual bool Intersects(const IShape* pShape) const;
	virtual bool Intersects(const IShape* pShape, Vector2f& vHit) const;	//	not yet implemented
	virtual bool Intersects(const usg::ai::Line& line) const;
	virtual bool Intersects(usg::ai::Line line, Vector2f& vHit) const;

protected:
	bool _Intersects(const IShape* pShape) const
	{
		const uint32 uCount = 32;
		const uint32 uTargetNumAxes = pShape->GetNumVerts();

		Vector2f myAxes[t_uSize];
		Vector2f targetAxes[uCount];

		this->GetAxes(myAxes, t_uSize);
		pShape->GetAxes(targetAxes, uTargetNumAxes);

		uint32 i, uC = t_uSize;
		for(i = 0; i < uC; i++)
		{
			usg::ai::Projection p1 = this->Project(myAxes[i]);
			usg::ai::Projection p2 = pShape->Project(myAxes[i]);

			if(!p1.Overlap(p2))
			{
				return false;
			}
		}

		uC = uTargetNumAxes;
		for(i = 0; i < uC; i++)
		{
			usg::ai::Projection p1 = this->Project(targetAxes[i]);
			usg::ai::Projection p2 = pShape->Project(targetAxes[i]);

			if(!p1.Overlap(p2))
			{
				return false;
			}
		}

		return true;
	}

	bool _Intersects(const IShape* pShape, Vector2f& vHit) const
	{
		ASSERT(false);
		//	not yet implemented
		return false;
	}

	void _Rotate(float fValue)
	{
		float fC = 0.0f;
		float fS = 0.0f;
		usg::Math::SinCos(usg::Math::DegToRad(-fValue), fS, fC);

		uint32 i, uC = t_uSize;
		for(i = 0; i < uC; i++)
		{
			const Vector2f vOrigin = m_avVerts[i];
			float fX = (vOrigin.x * fC) - (vOrigin.y * fS);
			float fY = (vOrigin.x * fS) + (vOrigin.y * fC);
			m_avVerts[i] = Vector2f(fX, fY);
		}

		UpdateAABB();
	}
	void ClearVerts()
	{
		MemSet(m_avVerts, 0, sizeof(Vector2f) * (size_t)t_uSize);
	}

	virtual void UpdateAABB()
	{
		shape_details::UpdateAABB(m_aabb,m_uGridMask,m_vCenter,m_avVerts,t_uSize);
	}

	int ComputeOutCode(float fX, float fY, const Vector2f& vMin, const Vector2f& vMax) const
	{
		int iMask = IShape::Inside;

		if(fX < vMin.x) { iMask |= IShape::Left; }
		else if(fX > vMax.x) { iMask |= IShape::Right; }

		if(fY < vMin.y) { iMask |= IShape::Bottom; }
		else if(fY > vMax.y) { iMask |= IShape::Top; }

		return iMask;
	}

	usg::Vector2f m_avVerts[t_uSize];
};

class Point : public Shape<1>
{
public:
	Point(){}
	Point(const Vector2f& vPoint) 
	{
		m_avVerts[0] = vPoint;
	}
	virtual ~Point(){}

	virtual void Scale(const Vector2f& vScale){}
	virtual void SetScale(const Vector2f& vScale){}

	virtual float GetSquaredDistanceToPoint(const Vector2f& vPosition) const
	{
		return vPosition.GetSquaredDistanceFrom(m_avVerts[0]);
	}
};

class Segment : public Shape<2>
{
public:
	Segment(){UpdateAABB();}
	Segment(const usg::Vector2f& vFrom, const usg::Vector2f& vTo)
	{
		m_avVerts[0] = vFrom;
		m_avVerts[1] = vTo;
		UpdateAABB();
	}
	virtual ~Segment(){}

	virtual void Scale(const Vector2f& vScale)
	{
		float fAngleToZero = m_fAngle - IShape::Degrees;
		Rotate(-fAngleToZero);
		m_avVerts[1].x += vScale.x;
		m_avVerts[0].x -= vScale.x;
		Rotate(fAngleToZero);
		m_vScale += vScale;
	}

	virtual float GetSquaredDistanceToPoint(const Vector2f& vPosition) const
	{
		ASSERT(false);
		return 0;
	}

	virtual void Scale(float fWidth)
	{
		float fAngleToZero = m_fAngle - IShape::Degrees;
		Rotate(-fAngleToZero);
		m_avVerts[1].x += fWidth;
		m_avVerts[0].x -= fWidth;
		Rotate(fAngleToZero);
		m_vScale.x += fWidth;
		m_vScale.y = 0.0f;
	}
	
	virtual void SetScale(const Vector2f& vScale)
	{
		float fAngleToZero = IShape::Degrees - m_fAngle;
		Rotate(fAngleToZero);
		m_avVerts[1].x = vScale.x;
		m_avVerts[0].x = vScale.x;
		Rotate(-fAngleToZero);
		m_vScale = vScale;
	}

	virtual void SetScale(float fWidth)
	{
		float fAngleToZero = IShape::Degrees - m_fAngle;
		Rotate(fAngleToZero);
		m_avVerts[1].x = fWidth;
		m_avVerts[0].x = fWidth;
		Rotate(-fAngleToZero);
		m_vScale.x = fWidth;
		m_vScale.y = 0.0f;
	}

	virtual void UpdateAABB()
	{
		m_aabb.m_vSize = Vector2f
			(
				fabsf(m_avVerts[0].x) + fabsf(m_avVerts[1].x),
				fabsf(m_avVerts[0].y) + fabsf(m_avVerts[1].y)
			);
		m_aabb.m_vPos = m_aabb.m_vSize * 0.5f;
	}


};

class OBB : public Shape<4>
{
public:
	OBB(){}
	OBB(const usg::Vector2f& vTopRight,
		const usg::Vector2f& vBottomRight,
		const usg::Vector2f& vBottomLeft,
		const usg::Vector2f& vTopLeft)
	{
		m_avVerts[0] = vTopRight;
		m_avVerts[1] = vBottomRight;
		m_avVerts[2] = vBottomLeft;
		m_avVerts[3] = vTopLeft;
	}
	virtual ~OBB(){}

	virtual void Scale(const Vector2f& vScale)
	{
		float fAngleToZero = m_fAngle - IShape::Degrees;
		Rotate(-fAngleToZero);

		float fX = vScale.x * 0.5f;
		float fY = vScale.y * 0.5f;

		m_avVerts[0] += Vector2f( fX,  fY);
		m_avVerts[1] += Vector2f( fX, -fY);
		m_avVerts[2] += Vector2f(-fX, -fY);
		m_avVerts[3] += Vector2f(-fX,  fY);

		Rotate(fAngleToZero);
		m_vScale += vScale;
	}
	
	virtual void SetScale(const Vector2f& vScale)
	{
		float fX = vScale.x * 0.5f;
		float fY = vScale.y * 0.5f;

		m_avVerts[0] = Vector2f( fX,  fY);
		m_avVerts[1] = Vector2f( fX, -fY);
		m_avVerts[2] = Vector2f(-fX, -fY);
		m_avVerts[3] = Vector2f(-fX,  fY);

		Rotate(m_fAngle);
		m_vScale = vScale;
	}

	virtual float GetSquaredDistanceToPoint(const Vector2f& vPosition) const
	{
		const OBB& obb = *this;
		const float fAngle = Math::DegToRad(obb.GetAngle());
		const Vector2f vDiffFromCenterToPoint = vPosition - obb.GetCenter();
		const float fSin = sinf(fAngle);
		const float fCos = cosf(fAngle);
		const Vector2f vRotatedPoint(vDiffFromCenterToPoint.x*fCos - vDiffFromCenterToPoint.y*fSin, vDiffFromCenterToPoint.y*fCos + vDiffFromCenterToPoint.x*fSin);
		const float fDx = Math::Max(fabsf(vRotatedPoint.x) - obb.GetScale().x / 2, 0.0f);
		const float fDy = Math::Max(fabsf(vRotatedPoint.y) - obb.GetScale().y / 2, 0.0f);
		const float fSquaredDistance = fDx*fDx + fDy*fDy;
		return fSquaredDistance;
	}
};

template <uint32 t_uSize>
bool Shape<t_uSize>::Intersects(const usg::Vector2f& vPoint) const
{
	if(!m_aabb.Intersects(vPoint))
	{
		return false;
	}

	Vector2f myAxes[t_uSize];
	Point point(vPoint);
	const Vector2f vAxis(vPoint.y, -vPoint.x);

	this->GetAxes(myAxes, t_uSize);
	for(uint32 i = 0; i < t_uSize; i++)
	{
		usg::ai::Projection p1 = this->Project(myAxes[i]);
		usg::ai::Projection p2 = point.Project(myAxes[i]);

		if(!p1.Overlap(p2))
		{
			return false;
		}
	}

	usg::ai::Projection p1 = this->Project(vAxis);
	usg::ai::Projection p2 = point.Project(vAxis);

	if(!p1.Overlap(p2))
	{
		return false;
	}

	return true;
}

template <uint32 t_uSize>
bool Shape<t_uSize>::Intersects(const usg::ai::AABB& aabb) const
{
	return false;
}

template <uint32 t_uSize>
bool Shape<t_uSize>::Intersects(const IShape* pShape) const
{
	if(!m_aabb.Intersects(pShape->GetAABB()))
	{
		return false;
	}

	return _Intersects(pShape);
}

template <uint32 t_uSize>
bool Shape<t_uSize>::Intersects(const IShape* pShape, Vector2f& vHit) const
{
	if(!m_aabb.Intersects(pShape->GetAABB()))
	{
		return false;
	}

	return _Intersects(pShape, vHit);
}

template <uint32 t_uSize>
bool Shape<t_uSize>::Intersects(const usg::ai::Line& line) const
{
	Segment segment(line.m_vFrom, line.m_vTo);
	return _Intersects(&segment);
}

template <uint32 t_uSize>
bool Shape<t_uSize>::Intersects(usg::ai::Line line, Vector2f& vHit) const
{
	bool bAccept = false;
	usg::Vector2f vMin, vMax;

	{
		const usg::Vector2f vExtents = m_aabb.GetExtents();
		vMin = Vector2f(m_aabb.m_vPos.x - vExtents.x, m_aabb.m_vPos.y - vExtents.y);
		vMax = Vector2f(m_aabb.m_vPos.x + vExtents.x, m_aabb.m_vPos.y + vExtents.y);
	}

	int iOutCode0 = ComputeOutCode(line.m_vFrom.x, line.m_vFrom.y, vMin, vMax);
	int iOutCode1 = ComputeOutCode(line.m_vTo.x, line.m_vTo.y, vMin, vMax);

	while(1)
	{
		if((iOutCode0 | iOutCode1) == 0)
		{
			bAccept = true;
			break;
		}
		else if((iOutCode0 & iOutCode1) != 0)
		{
			break;
		}
		else
		{
			float fX = 0.0f;
			float fY = 0.0f;

			int iOutCode = (iOutCode0 == 0) ? iOutCode1 : iOutCode0;
			Vector2f& vL1 = line.m_vFrom;
			Vector2f& vL2 = line.m_vTo;

			if((iOutCode & IShape::Top) == IShape::Top)
			{
				fX = vL1.x + (vL2.x - vL1.x) * (vMax.y - vL1.y) / (vL2.y - vL1.y);
				fY = vMax.y;
			}
			else if((iOutCode & IShape::Bottom) == IShape::Bottom)
			{
			    fX = vL1.x + (vL2.x - vL1.x) * (vMin.y - vL1.y) / (vL2.y - vL1.y);
			    fY = vMin.y;
			}
			else if((iOutCode & IShape::Right) == IShape::Right)
			{
			    fY = vL1.y + (vL2.y - vL1.y) * (vMax.x - vL1.x) / (vL2.x - vL1.x);
			    fX = vMax.x;
			}
			else if((iOutCode & IShape::Left) == IShape::Left)
			{
			    fY = vL1.y + (vL2.y - vL1.y) * (vMin.x - vL1.x) / (vL2.x - vL1.x);
			    fX = vMin.x;
			}

			if(iOutCode == iOutCode0)
			{
				vL1.x = fX;
				vL1.y = fY;
				iOutCode0 = ComputeOutCode(vL1.x, vL1.y, vMin, vMax);
			}
			else
			{
				vL2.x = fX;
				vL2.y = fY;
				iOutCode1 = ComputeOutCode(vL2.x, vL2.y, vMin, vMax);
			}
		}
	}

	if(bAccept)
	{
		vHit = line.m_vFrom;
		return true;
	}

	return false;
}

}	//	namespace ai

}	//	namespace usg

#endif	//	__USG_AI_SHAPE__
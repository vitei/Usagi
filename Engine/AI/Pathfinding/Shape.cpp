/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
//	Shape.cpp
#include "Engine/Common/Common.h"
#include "Shape.h"

namespace usg
{

	namespace ai
	{

		const float IShape::Degrees = 180.0f;

		float Signed2DTriArea(Vector2f a, Vector2f b, Vector2f c)
		{
			return (a.x - c.x) * (b.y - c.y) - (a.y - c.y) * (b.x - c.x);
		}

		int Test2DSegmentSegment(Vector2f a, Vector2f b, Vector2f c, Vector2f d, float &t, Vector2f &p)
		{
			float a1 = Signed2DTriArea(a, b, d);
			float a2 = Signed2DTriArea(a, b, c);
			if (a1 * a2 < 0.0f)
			{
				float a3 = Signed2DTriArea(c, d, a);
				float a4 = a3 + a2 - a1;
				if (a3 * a4 < 0.0f)
				{
					t = a3 / (a3 - a4);
					p = a + (b - a)*t;
					return 1;
				}
			}
			return 0;
		}

		int IShape::Intersects(const usg::ai::Line& line, Vector2f& vHit1, Vector2f& vHit2) const
		{
			const uint32 uNumVerts = GetNumVerts();
			const Vector2f* pVerts = GetVerts();
			const Vector2f& a = line.m_vFrom - GetCenter();
			const Vector2f& b = line.m_vTo - GetCenter();
			int iIntersectionCount = 0;
			Vector2f vTemp;
			float fT;
			for (uint32 i = 0; i < uNumVerts; i++)
			{
				const Vector2f& c = pVerts[i];
				const Vector2f& d = pVerts[(i + 1) % uNumVerts];
				if (Test2DSegmentSegment(a, b, c, d, fT, vTemp))
				{
					if (iIntersectionCount++ == 0)
					{
						vHit1 = vTemp + GetCenter();
					}
					else
					{
						vHit2 = vTemp + GetCenter();
					}
					if (iIntersectionCount == 2)
					{
						// Because we assume the shape is convex, there can be no more than 2 intersections
						return 2;
					}
				}
			}
			return iIntersectionCount;
		}

		namespace shape_details
		{
			static const float WorldSize = 500;

			inline uint64 PositionToIndex(float fWorldSize, const Vector2f& v)
			{
				const float fGridX = (v.x + fWorldSize) / (2*fWorldSize)* 8;
				const float fGridY = (v.y + fWorldSize) / (2*fWorldSize)* 8;
				const uint64 uIndex = (uint64)(Math::Clamp((int)(fGridY), 0, 8 - 1)) * 8 + (Math::Clamp((int)(fGridX), 0, 8 - 1));
				return uIndex;
			}

#ifdef DEBUG_BUILD
			static void PrintMask(uint64 uMask)
			{
				char temp[9];
				temp[8] = 0;
				const uint64 uOne = 1;
				for (uint64 i = 0; i < 8; i++)
				{
					for (uint64 j = 0; j < 8; j++)
					{
						const uint64 uIndex = i * 8 + j;
						if (uMask & (uOne<<uIndex))
						{
							temp[j] = '#';
						}
						else
						{
							temp[j] = '.';
						}
					}
					DEBUG_PRINT("%s\n",temp);
				}
			}
#endif

			uint64 ComputeGridMask(const Vector2f& vPoint)
			{
				const uint64 uEverything = (uint64)(-1);

				if (vPoint.x < -WorldSize || vPoint.y < -WorldSize)
				{
					return uEverything;
				}
				if (vPoint.x > WorldSize || vPoint.y > WorldSize)
				{
					return uEverything;
				}
				const uint64 uOne = 1;
				const uint64 uIndex = PositionToIndex(WorldSize, vPoint);
				return uOne << uIndex;
			}

			uint64 ComputeGridMask(const Vector2f& vMin, const Vector2f& vMax)
			{
				const uint64 uEverything = (uint64)(-1);

				if (vMin.x < -WorldSize || vMin.y < -WorldSize)
				{
					return uEverything;
				}
				if (vMax.x > WorldSize || vMax.y > WorldSize)
				{
					return uEverything;
				}

				const uint64 uOne = 1;
				const uint64 uIndex1 = PositionToIndex(WorldSize, vMin);
				const uint64 uIndex2 = PositionToIndex(WorldSize, vMax);
				if (uIndex1 != uIndex2)
				{
					const uint64 uMask1 = uOne << uIndex1;
					const uint64 uMask2 = uOne << uIndex2;
					uint64 uR = 0;
					if (uIndex1 == uIndex2 + 1 || uIndex1 == uIndex2 - 1)
					{
						uR = uMask1 | uMask2;
					}
					else if (uIndex1 == uIndex2 + 8 || uIndex2 == uIndex1 + 8)
					{
						uR = uMask1 | uMask2;
					}
					else
					{
						const int iX1 = (int)(uIndex1 % 8);
						const int iX2 = (int)(uIndex2 % 8);
						const int iDX = (int)(iX2 > iX1 ? 1 : -1);
						const int iY1 = (int)(uIndex1 / 8);
						const int iY2 = (int)(uIndex2 / 8);
						const int iDY = (int)(iY2 > iY1 ? 1 : -1);
						
						for (int i = iX1; i != iX2 + iDX; i += iDX)
						{
							for (int j = iY1; j != iY2 + iDY; j += iDY)
							{
								const uint64 uIndex = j * 8 + i;
								uR |= (uOne << uIndex);
							}
						}
					}
					return uR;
				}
				return uOne << uIndex1;
			}

			uint64 ComputeGridMask(const AABB& aabb)
			{
				const Vector2f& vMin = aabb.m_vPos - aabb.m_vSize*0.5f;
				const Vector2f& vMax = aabb.m_vPos + aabb.m_vSize*0.5f;
				return ComputeGridMask(vMin, vMax);
			}

			void UpdateAABB(AABB& aabb, uint64& uGridMask, const Vector2f& vCenter, const Vector2f* pVerts, uint32 uVertNum)
			{
				float fRight = 0.0f;
				float fLeft = 0.0f;
				float fTop = 0.0f;
				float fBottom = 0.0f;

				uint32 i, uC = uVertNum;
				for (i = 0; i < uC; i++)
				{
					const Vector2f& vV = pVerts[i];

					if (vV.x > fRight) { fRight = vV.x; }
					if (vV.x < fLeft) { fLeft = vV.x; }
					if (vV.y > fTop) { fTop = vV.y; }
					if (vV.y < fBottom) { fBottom = vV.y; }
				}

				aabb.m_vPos = vCenter;
				aabb.m_vSize = Vector2f(Math::Abs(fRight) + Math::Abs(fLeft), Math::Abs(fTop) + Math::Abs(fBottom));
				uGridMask = ComputeGridMask(aabb);
			}
		}

	}	//	namespace ai

}	//	namespace usg
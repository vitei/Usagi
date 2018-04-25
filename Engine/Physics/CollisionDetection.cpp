/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
//
// Functions taken from Real-Time collision Detection by Christopher Ericson,
// published by Morgan Kaufmann Publishers, ©2005 Elsevier Inc
//
#include "Engine/Common/Common.h"
#include "Engine/Physics/CollisionDetection.h"

namespace usg
{

	Vector3f ClosestPointOnTriangleToPoint(const Vector3f& vPoint, const Vector3f& vA, const Vector3f& vB, const Vector3f& vC)
	{
		Vector3f ab = vB - vA;
		Vector3f ac = vC - vA;
		Vector3f bc = vC - vB;

		// Compute parametric position s for projection P' of P on AB,
		// P' = A + s*AB, s = snom/(snom+sdenom)
		float snom = DotProduct(vPoint - vA, ab), sdenom = DotProduct(vPoint - vB, vA - vB);

		// Compute parametric position t for projection P' of P on AC,
		// P' = A + t*AC, s = tnom/(tnom+tdenom)
		float tnom = DotProduct(vPoint - vA, ac), tdenom = DotProduct(vPoint - vC, vA - vC);

		if (snom <= 0.0f && tnom <= 0.0f) return vA; // Vertex region early out

		// Compute parametric position u for projection P' of P on BC,
		// P' = B + u*BC, u = unom/(unom+udenom)
		float unom = DotProduct(vPoint - vB, bc), udenom = DotProduct(vPoint - vC, vB - vC);

		if (sdenom <= 0.0f && unom <= 0.0f) return vB; // Vertex region early out
		if (tdenom <= 0.0f && udenom <= 0.0f) return vC; // Vertex region early out


		// P is outside (or on) AB if the triple scalar product [N PA PB] <= 0
		Vector3f n = CrossProduct(vB - vA, vC - vA);
		float vc = DotProduct(n, CrossProduct(vA - vPoint, vB - vPoint));
		// If P outside AB and within feature region of AB,
		// return projection of P onto AB
		if (vc <= 0.0f && snom >= 0.0f && sdenom >= 0.0f)
		{
			return vA + snom / (snom + sdenom) * ab;
		}

		// P is outside (or on) BC if the triple scalar product [N PB PC] <= 0
		float va = DotProduct(n, CrossProduct(vB - vPoint, vC - vPoint));
		// If P outside BC and within feature region of BC,
		// return projection of P onto BC
		if (va <= 0.0f && unom >= 0.0f && udenom >= 0.0f)
		{
			return vB + unom / (unom + udenom) * bc;
		}

		// P is outside (or on) CA if the triple scalar product [N PC PA] <= 0
		float vb = DotProduct(n, CrossProduct(vC - vPoint, vA - vPoint));
		// If P outside CA and within feature region of CA,
		// return projection of P onto CA
		if (vb <= 0.0f && tnom >= 0.0f && tdenom >= 0.0f)
		{
			return vA + tnom / (tnom + tdenom) * ac;
		}

		// P must project inside face region. Compute Q using barycentric coordinates
		float u = va / (va + vb + vc);
		float v = vb / (va + vb + vc);
		float w = 1.0f - u - v; // = vc / (va + vb + vc)
		return u * vA + v * vB + w * vC;
	}

}


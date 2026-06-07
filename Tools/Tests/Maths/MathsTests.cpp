#include "Engine/Common/Common.h"
#include "Engine/Maths/AABB.h"
#include "Engine/Maths/Matrix4x4.h"
#include "Engine/Maths/Quaternionf.h"
#include "Engine/Maths/Sphere.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>

namespace usg
{
	MemType g_uDefaultMemType = MEMTYPE_STANDARD;

	namespace mem
	{
		void* Alloc(MemType, MemAllocType, memsize uSize, uint32 uAlign, bool)
		{
			UNUSED_VAR(uAlign);
			return new char[uSize];
		}

		void Free(MemType, void* pData, bool)
		{
			delete[] (char*)pData;
		}

		void Free(void* pData)
		{
			delete[] (char*)pData;
		}
	}
}

namespace eastl
{
	void AssertionFailure(const char* pExpression)
	{
		printf("EASTL ASSERT FAILED: %s\n", pExpression);
		abort();
	}
}

namespace
{
	bool Expect(bool condition, const char* szMessage)
	{
		if(!condition)
		{
			printf("FAILED: %s\n", szMessage);
			return false;
		}
		return true;
	}

	bool NearlyEqual(float a, float b, float epsilon = 1e-4f)
	{
		return fabsf(a - b) <= epsilon;
	}

	bool ExpectVector(const usg::Vector3f& value, const usg::Vector3f& expected, const char* szMessage)
	{
		const bool ok = NearlyEqual(value.x, expected.x) && NearlyEqual(value.y, expected.y) && NearlyEqual(value.z, expected.z);
		if(!ok)
		{
			printf("FAILED: %s got=(%.6f, %.6f, %.6f) expected=(%.6f, %.6f, %.6f)\n",
				szMessage, value.x, value.y, value.z, expected.x, expected.y, expected.z);
		}
		return ok;
	}

	bool ExpectVector4(const usg::Vector4f& value, const usg::Vector4f& expected, const char* szMessage)
	{
		const bool ok = NearlyEqual(value.x, expected.x) && NearlyEqual(value.y, expected.y) &&
			NearlyEqual(value.z, expected.z) && NearlyEqual(value.w, expected.w);
		if(!ok)
		{
			printf("FAILED: %s got=(%.6f, %.6f, %.6f, %.6f) expected=(%.6f, %.6f, %.6f, %.6f)\n",
				szMessage, value.x, value.y, value.z, value.w, expected.x, expected.y, expected.z, expected.w);
		}
		return ok;
	}

	bool TestMatrixTranslationUsesVectorW()
	{
		usg::Matrix4x4 transform = usg::Matrix4x4::Identity();
		transform.SetTranslation(2.0f, -3.0f, 5.0f);

		bool ok = true;
		ok &= ExpectVector(transform.TransformVec3(usg::Vector3f(1.0f, 2.0f, 3.0f), 1.0f),
			usg::Vector3f(3.0f, -1.0f, 8.0f), "position transform applies translation");
		ok &= ExpectVector(transform.TransformVec3(usg::Vector3f(1.0f, 2.0f, 3.0f), 0.0f),
			usg::Vector3f(1.0f, 2.0f, 3.0f), "direction transform ignores translation");
		return ok;
	}

	bool TestAABBContainmentAndLerp()
	{
		usg::AABB box;
		box.SetCentreRadii(usg::Vector3f(0.0f, 0.0f, 0.0f), usg::Vector3f(1.0f, 2.0f, 3.0f));

		bool ok = true;
		ok &= Expect(box.InBox(usg::Vector3f(1.0f, -2.0f, 3.0f)), "AABB includes boundary points");
		ok &= Expect(!box.InBox(usg::Vector3f(1.1f, 0.0f, 0.0f)), "AABB rejects points outside one axis");

		usg::AABB start(usg::Vector3f(-1.0f, -1.0f, -1.0f), usg::Vector3f(1.0f, 1.0f, 1.0f));
		usg::AABB end(usg::Vector3f(1.0f, 3.0f, 5.0f), usg::Vector3f(5.0f, 7.0f, 9.0f));
		usg::AABB midpoint;
		usg::Lerp(start, end, midpoint, 0.5f);

		usg::Vector3f centre;
		usg::Vector3f radii;
		midpoint.GetCentreRadii(centre, radii);
		ok &= ExpectVector(centre, usg::Vector3f(1.5f, 2.5f, 3.5f), "AABB lerp interpolates centre");
		ok &= ExpectVector(radii, usg::Vector3f(1.5f, 1.5f, 1.5f), "AABB lerp interpolates radii");
		return ok;
	}

	bool TestVector4LerpInterpolatesW()
	{
		const usg::Vector4f start(0.0f, 2.0f, 4.0f, 0.0f);
		const usg::Vector4f end(10.0f, 12.0f, 14.0f, 1.0f);
		const usg::Vector4f expected(5.0f, 7.0f, 9.0f, 0.5f);

		bool ok = true;
		ok &= ExpectVector4(usg::Lerp(start, end, 0.5f), expected, "Vector4f Lerp return overload interpolates w");

		usg::Vector4f out;
		usg::Lerp(start, end, out, 0.5f);
		ok &= ExpectVector4(out, expected, "Vector4f Lerp out-param overload interpolates w");
		return ok;
	}

	bool TestScalarWrapHelpers()
	{
		bool ok = true;
		ok &= Expect(NearlyEqual(usg::Math::WrapValue(370.0f, 0.0f, 360.0f), 10.0f),
			"WrapValue wraps values above range");
		ok &= Expect(NearlyEqual(usg::Math::WrapValue(-10.0f, 0.0f, 360.0f), 350.0f),
			"WrapValue wraps values below range");
		ok &= Expect(NearlyEqual(usg::Math::MatchLoopingValue(5.0f, 355.0f, 0.0f, 360.0f), 365.0f),
			"MatchLoopingValue moves low value near high match");
		ok &= Expect(NearlyEqual(usg::Math::MatchLoopingValue(355.0f, 5.0f, 0.0f, 360.0f), -5.0f),
			"MatchLoopingValue moves high value near low match");
		return ok;
	}

	bool TestHermiteEndpoints()
	{
		const usg::Vector3f p0(1.0f, 2.0f, 3.0f);
		const usg::Vector3f v0(4.0f, 5.0f, 6.0f);
		const usg::Vector3f p1(7.0f, 8.0f, 9.0f);
		const usg::Vector3f v1(-1.0f, -2.0f, -3.0f);

		bool ok = true;
		ok &= ExpectVector(usg::Math::Hermite(p0, v0, p1, v1, 0.0f), p0, "Hermite returns p0 at t=0");
		ok &= ExpectVector(usg::Math::Hermite(p0, v0, p1, v1, 1.0f), p1, "Hermite returns p1 at t=1");
		ok &= ExpectVector(usg::Math::HermiteDerivative(p0, v0, p1, v1, 0.0f), v0, "HermiteDerivative returns v0 at t=0");
		ok &= ExpectVector(usg::Math::HermiteDerivative(p0, v0, p1, v1, 1.0f), v1, "HermiteDerivative returns v1 at t=1");
		return ok;
	}

	bool TestAABBPointAccumulation()
	{
		usg::AABB box;
		box.Invalidate();
		box.Apply(usg::Vector3f(-1.0f, 2.0f, 0.5f));
		box.Apply(usg::Vector3f(3.0f, -2.0f, 4.0f));

		usg::Vector3f centre;
		usg::Vector3f radii;
		box.GetCentreRadii(centre, radii);

		bool ok = true;
		ok &= ExpectVector(box.GetMin(), usg::Vector3f(-1.0f, -2.0f, 0.5f), "AABB Apply tracks minimum point");
		ok &= ExpectVector(box.GetMax(), usg::Vector3f(3.0f, 2.0f, 4.0f), "AABB Apply tracks maximum point");
		ok &= ExpectVector(centre, usg::Vector3f(1.0f, 0.0f, 2.25f), "AABB Apply updates centre");
		ok &= ExpectVector(radii, usg::Vector3f(2.0f, 2.0f, 1.75f), "AABB Apply updates radii");
		return ok;
	}

	bool TestSphereIntersectionBoundary()
	{
		const usg::Sphere origin(usg::Vector3f(0.0f, 0.0f, 0.0f), 2.0f);

		bool ok = true;
		ok &= Expect(origin.Intersect(usg::Sphere(usg::Vector3f(3.0f, 0.0f, 0.0f), 1.0f)),
			"Sphere intersection includes tangent boundary");
		ok &= Expect(!origin.Intersect(usg::Sphere(usg::Vector3f(3.01f, 0.0f, 0.0f), 1.0f)),
			"Sphere intersection rejects separated spheres");
		return ok;
	}

	bool TestAABBSphereYRadiusBoundaries()
	{
		usg::AABB box;
		box.SetCentreRadii(usg::Vector3f(0.0f, 0.0f, 0.0f), usg::Vector3f(1.0f, 1.0f, 1.0f));

		bool ok = true;
		ok &= Expect(box.IntersectBox(usg::Sphere(usg::Vector3f(0.0f, 1.5f, 0.0f), 0.5f)),
			"AABB sphere intersection expands Y bounds by sphere radius");
		ok &= Expect(!box.IntersectBox(usg::Sphere(usg::Vector3f(0.0f, 1.6f, 0.0f), 0.5f)),
			"AABB sphere intersection rejects Y separation beyond radius");
		ok &= Expect(box.ContainedInBox(usg::Sphere(usg::Vector3f(0.0f, 0.5f, 0.0f), 0.5f)),
			"AABB sphere containment shrinks Y bounds by sphere radius");
		ok &= Expect(!box.ContainedInBox(usg::Sphere(usg::Vector3f(0.0f, 0.6f, 0.0f), 0.5f)),
			"AABB sphere containment rejects Y overlap outside shrunken bounds");
		return ok;
	}
}

int main()
{
	bool ok = true;
	ok &= TestMatrixTranslationUsesVectorW();
	ok &= TestAABBContainmentAndLerp();
	ok &= TestVector4LerpInterpolatesW();
	ok &= TestScalarWrapHelpers();
	ok &= TestHermiteEndpoints();
	ok &= TestAABBPointAccumulation();
	ok &= TestSphereIntersectionBoundary();
	ok &= TestAABBSphereYRadiusBoundaries();

	if(!ok)
	{
		return 1;
	}

	printf("Maths tests passed\n");
	return 0;
}

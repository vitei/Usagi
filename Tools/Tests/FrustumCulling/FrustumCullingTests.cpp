#include "Engine/Common/Common.h"
#include "Engine/Scene/Frustum.h"

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

	usg::Frustum CreateCanonicalFrustum()
	{
		usg::Frustum frustum;
		frustum.SetUp(usg::Matrix4x4::Identity());
		return frustum;
	}

	bool TestAcceptsMixedPointSets()
	{
		usg::Frustum frustum = CreateCanonicalFrustum();
		const usg::Vector4f points[] =
		{
			usg::Vector4f(0.0f, 0.0f, 0.0f, 1.0f),
			usg::Vector4f(2.0f, 0.0f, 0.0f, 1.0f),
			usg::Vector4f(0.0f, 0.5f, 0.0f, 1.0f)
		};

		return Expect(frustum.ArePointsInFrustum(points, ARRAY_SIZE(points)),
			"point sets stay visible when at least one point survives each frustum plane");
	}

	bool TestRejectsPointSetsBehindOnePlane()
	{
		usg::Frustum frustum = CreateCanonicalFrustum();
		const usg::Vector4f points[] =
		{
			usg::Vector4f(2.0f, -0.5f, 0.0f, 1.0f),
			usg::Vector4f(2.0f, 0.0f, 0.0f, 1.0f),
			usg::Vector4f(2.0f, 0.5f, 0.0f, 1.0f)
		};

		return Expect(!frustum.ArePointsInFrustum(points, ARRAY_SIZE(points)),
			"point sets cull only when every point is behind the same frustum plane");
	}

	bool TestAcceptsCanonicalBoxCorners()
	{
		usg::Frustum frustum = CreateCanonicalFrustum();
		const usg::Vector4f points[] =
		{
			usg::Vector4f(-0.5f, -0.5f, 0.0f, 1.0f),
			usg::Vector4f(-0.5f, 0.5f, 0.0f, 1.0f),
			usg::Vector4f(0.5f, -0.5f, 0.0f, 1.0f),
			usg::Vector4f(0.5f, 0.5f, 0.0f, 1.0f)
		};

		return Expect(frustum.ArePointsInFrustum(points, ARRAY_SIZE(points)),
			"canonical in-frustum corners are visible");
	}
}

int main()
{
	bool ok = true;
	ok &= TestAcceptsMixedPointSets();
	ok &= TestRejectsPointSetsBehindOnePlane();
	ok &= TestAcceptsCanonicalBoxCorners();

	if(!ok)
	{
		return 1;
	}

	printf("FrustumCulling tests passed\n");
	return 0;
}

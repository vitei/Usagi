/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Maths/Matrix4x4.h"
#include "Engine/Maths/Matrix3x3.h"
#include "Engine/Maths/MathUtil.h"
#include "Engine/Physics/CollisionDetection.h"
#include "Engine/Debug/Rendering/DebugRender.h"
#include "Engine/Physics/CollisionQuadTree.h"
#include "Engine/Resource/CollisionModelResource.h"

#define SQR(v) ((v)*(v))

namespace usg{

//void CalcEndpoints(const Capsule* capsule, Vector3f& p1, Vector3f& p2)  ;
float ClosestPtSegmentSegment(const Vector3f& p1, const Vector3f& q1, const Vector3f& p2, const Vector3f& q2,
                              float& s, float& t, Vector3f& c1, Vector3f& c2);
	
#define CLIP_RIGHT	(1<<0)	// cohen-sutherland clipping outcodes
#define CLIP_LEFT		(1<<1)
#define CLIP_TOP		(1<<2)
#define CLIP_BOTTOM	(1<<3)
#define CLIP_FRONT	(1<<4)
#define CLIP_BACK		(1<<5)
	
static unsigned long calc_outcode( const Vector3f &bbox_min, const Vector3f &bbox_max, const Vector3f &pnt )
{
	unsigned long outcode = 0;
	
	if( pnt.x > bbox_max.x ) {
		outcode |= CLIP_RIGHT;
	} else if( pnt.x < bbox_min.x ) {
		outcode |= CLIP_LEFT;
	}
	if( pnt.y > bbox_max.y ) {
		outcode |= CLIP_TOP;
	} else if( pnt.y < bbox_min.y ) {
		outcode |= CLIP_BOTTOM;
	}
	if( pnt.z > bbox_max.z ) {
		outcode |= CLIP_BACK;
	} else if( pnt.z < bbox_min.z ) {
		outcode |= CLIP_FRONT;
	}
	
	return outcode;
}
	
	
	
bool collide_linesegment_boundingbox( const Vector3f &bbox_min, const Vector3f &bbox_max, const Vector3f &p1, const Vector3f &p2, Vector3f &intercept )
{
	unsigned long outcode1, outcode2;
	
	outcode1 = calc_outcode( bbox_min, bbox_max, p1 );
	if( outcode1 == 0 ) {
		// point inside bounding box
		intercept = p1;
		return true;
	}
	
	outcode2 = calc_outcode( bbox_min, bbox_max, p2 );
	if( outcode2 == 0 ) {
		// point inside bounding box
		intercept = p2;
		return true;
	}
	
	if( (outcode1 & outcode2) > 0 ) {
		// both points on same side of box
		return false;
	}
	
	// check intersections
	if( outcode1 & (CLIP_RIGHT | CLIP_LEFT) ) {
		if( outcode1 & CLIP_RIGHT ) {
			intercept.x = bbox_max.x;
		} else {
			intercept.x = bbox_min.x;
		}
		float x1 = p2.x - p1.x;
		float x2 = intercept.x - p1.x;
		intercept.y = p1.y + x2 * (p2.y - p1.y) / x1;
		intercept.z = p1.z + x2 * (p2.z - p1.z) / x1;
		
		if( intercept.y <= bbox_max.y && intercept.y >= bbox_min.y && intercept.z <= bbox_max.z && intercept.z >= bbox_min.z ) {
			return true;
		}
	}
	
	if( outcode1 & (CLIP_TOP | CLIP_BOTTOM) ) {
		if( outcode1 & CLIP_TOP ) {
			intercept.y = bbox_max.y;
		} else {
			intercept.y = bbox_min.y;
		}
		float y1 = p2.y - p1.y;
		float y2 = intercept.y - p1.y;
		intercept.x = p1.x + y2 * (p2.x - p1.x) / y1;
		intercept.z = p1.z + y2 * (p2.z - p1.z) / y1;
		
		if( intercept.x <= bbox_max.x && intercept.x >= bbox_min.x && intercept.z <= bbox_max.z && intercept.z >= bbox_min.z ) {
			return true;
		}
	}
	
	if( outcode1 & (CLIP_FRONT | CLIP_BACK) ) {
		if( outcode1 & CLIP_BACK ) {
			intercept.z = bbox_max.z;
		} else {
			intercept.z = bbox_min.z;
		}
		float z1 = p2.z - p1.z;
		float z2 = intercept.z - p1.z;
		intercept.x = p1.x + z2 * (p2.x - p1.x) / z1;
		intercept.y = p1.y + z2 * (p2.y - p1.y) / z1;
		
		if( intercept.x <= bbox_max.x && intercept.x >= bbox_min.x && intercept.y <= bbox_max.y && intercept.y >= bbox_min.y ) {
			return true;
		}
	}
	// nothing found
	return false;
}

void Swap(float &a, float &b)
{
	float t = a;
	a = b;
	b = t;
}
	

/////////////////// UTILITIES //////////////////

float ClosestPtSegmentSegment(const Vector3f& p1, const Vector3f& q1, const Vector3f& p2, const Vector3f& q2,
                              float& s, float& t, Vector3f& c1, Vector3f& c2)
{
	Vector3f d1 = q1 - p1; // dir vec of segment s1
	Vector3f d2 = q2 - p2; // dir vec of segment s2
	Vector3f r = p1 - p2;
	float a = DotProduct(d1, d1); // sq len of segment s1
	float e = DotProduct(d2, d2); // sq len of segment s2
	float f = DotProduct(d2, r);

	// check if either or both segments degenerate into points
	if (a <= Math::EPSILON && e <= Math::EPSILON)
	{
		// both segments degenerate into points
		s = t = 0.0f;
		c1 = p1;
		c2 = p2;
		return DotProduct(c1-c2, c1-c2);
	}

	if (a <= Math::EPSILON)
	{
		// first segment degenerates into a point
		s = 0.0f;
		t = f / e; // s = 0 => t = (b*s+f) / e = f / e
		t = Math::Clamp(t, 0.0f, 1.0f);
	}
	else
	{
		float c = DotProduct(d1, r);
		if (e <= Math::EPSILON)
		{
			// second segment degenerates into a point
			t = 0.0f;
			s = Math::Clamp(-c/a, 0.0f, 1.0f); // t = 0 => s = (b*t-c) / a = -c/a
		}
		else
		{
			// general nondegenerate case starts here
			float b = DotProduct(d1, d2);
			float denom = a*e-b*b; // always nonnegative

			// if segments not parallel, compute closest point on l1 to l2 and
			// clamp to segment s1. else pick arbitrary s (here 0)
			if (denom != 0.0f)
			{
				s = Math::Clamp((b*f - c*e)/denom, 0.0f, 1.0f);
			}
			else
			{
				s = 0.0f;
			}

			float tnom = b*s + f;
			if (tnom < 0.0f)
			{
				t = 0.0f;
				s = Math::Clamp(-c/a, 0.0f, 1.0f);
			}
			else if (tnom > e)
			{
				t = 1.0f;
				s = Math::Clamp((b-c)/a, 0.0f, 1.0f);
			}
			else
			{
				t = tnom/e;
			}
		}
	}

	c1 = p1 + d1 * s;
	c2 = p2 + d2 * t;
	return DotProduct(c1-c2, c1-c2);
}
	
}

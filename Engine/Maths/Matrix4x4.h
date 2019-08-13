/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Storage and arithmetic operations a row major 4x4 matrix
//	N.B. use of row vectors means don't need to transpose to send to OGL
*****************************************************************************/
#ifndef _USG_MATRIX4X4_H_
#define	_USG_MATRIX4X4_H_

#include "Vector3f.h"
#include "Vector4f.h"


namespace usg{

class Plane;
class Quaternionf;
class Matrix4x3;

class Matrix4x4
{
public:
	Matrix4x4() { /*LoadIdentity();*/ }
	Matrix4x4(	float r11, float r12, float r13, float r14,
				float r21, float r22, float r23, float r24,
				float r31, float r32, float r33, float r34,
				float r41, float r42, float r43, float r44 );

	Matrix4x4(const Matrix4x3& in) { Assign(in); }
	Matrix4x4(const Matrix3x3& in) { *this = in; }
	Matrix4x4(const Quaternionf &quat)
	{
		*this = quat;
	}
				
	~ Matrix4x4() {}

	void LoadIdentity();
	void Clear();

	void Transpose();
	void GetTranspose(Matrix4x4 &out) const;

	inline Vector3f TransformVec3(const Vector3f& vec3, float w = 1.0f) const;

	void Inverse();
	void GetInverse(Matrix4x4 &out) const;
	void GetQuickInverse(Matrix4x4 &out) const;

	float Determinant() const;

	Matrix4x4 operator*( const Matrix4x4 &rhs ) const;
	void operator = (const Quaternionf &quat);
	Matrix4x4& operator=(const Matrix4x3& in) { Assign(in); return *this; }
	Matrix4x4& operator=(const Matrix3x3& in);

	Matrix4x4 operator *= (const Matrix4x4 matr);
	bool operator == (const Matrix4x4 &mat) const; 

	Matrix4x4 operator + (const Matrix4x4 rhs) const;
	Matrix4x4& operator += (const Matrix4x4 rhs);

	Matrix4x4 operator * (float fRhs) const;
	Matrix4x4& operator *= (float fRhs);
	
	void MakeRotateX(float x);
	void MakeRotateY(float y);
	void MakeRotateZ(float z);

	void MakeRotateYPR(float yaw, float pitch, float roll);

	void MakeRotate( float x, float y, float z );

	void MakeRotAboutAxis(const Vector4f &axis, float fRadians);

	void Copy( const float* in );

	void CameraMatrix( 
		const Vector4f &right, const Vector4f &up, const Vector4f &view, const Vector4f &pos );

	void BuildCameraFromModel( const Matrix4x4 &matIn );
	void BuildCameraFromModel( const Matrix4x4 &matIn, const Vector4f &localEyePos );

	void BuildModelFromCamera( const Matrix4x4 &matIn );

	void ModelMatrix( 
		const Vector4f &right, const Vector4f &up, const Vector4f &view, const Vector4f &pos );

	void Perspective(float32 fFovY, float32 fAspect, float32 fZNear, float32 fZFar, bool bScreenOrient = true);
	void PerspectiveRH(float32 fFovY, float32 fAspect, float32 fZNear, float32 fZFar, bool bScreenOrient = true);

	void Orthographic(float32 fLeft, float32 fRight, float32 fBottom, float32 fTop, float32 fZNear, float32 fZFar, bool bScreenOrient = true);
	void OrthographicRH(float32 fLeft, float32 fRight, float32 fBottom, float32 fTop, float32 fZNear, float32 fZFar, bool bScreenOrient = true);

	void Billboard(const Vector4f &vcPos, const Vector4f &vcDir, const Vector4f &vcWorldUp);

	void LookAt(const Vector3f& vEyePos, const Vector3f& vAt, const Vector3f& vUp);
	void LookAtRH(const Vector3f& vEyePos, const Vector3f& vAt, const Vector3f& vUp);

	void Translate( const Vector4f &translation );
	void Translate( const Vector3f &translation );

	void Translate( float x, float y, float z );

	inline void SetTranslation( const Vector4f &pos );
	inline void SetTranslation( const Vector3f &pos );
	inline void SetTranslation( float x, float y, float z );

	// Sets the scale (doesn't not multiply existing values)
	void MakeScale( float x, float y, float z );
	void MakeScale( const Vector3f& vec );
    
	void Scale( float x, float y, float z, float w );
    
    
	// For our mirrors
	void MakeReflectedMatrix( const Plane &reflectPlane, Matrix4x4& out ) const;

	// So we can use the convention [][]
	const float* operator[](int id) const { return &m[id*4]; };
	float* operator[](int id) { return &m[id*4]; };


	// We could cast our internal data, but doing so confused the one of our compilers are maximum optimization
#if 1
	const Vector4f vRight() const	{ return Vector4f(_11, _12, _13, _14); }
	const Vector4f vUp() const		{ return Vector4f(_21, _22, _23, _24); }
	const Vector4f vFace() const	{ return Vector4f(_31, _32, _33, _34); }
	const Vector4f vPos() const		{ return Vector4f(_41, _42, _43, _44); }
#else
	const Vector4f& vRight() const { return *(Vector4f*)(&_11); }
	const Vector4f& vUp() const { return *(Vector4f*)(&_21);  }
	const Vector4f& vFace() const { return *(Vector4f*)(&_31); }
	const Vector4f& vPos() const { return *(Vector4f*)(&_41); }
#endif

	void SetRight(const Vector4f & vRight) { _11 = vRight.x; _12 = vRight.y; _13 = vRight.z; _14 = vRight.w; }
	void SetUp(const Vector4f & vUp) { _21 = vUp.x; _22 = vUp.y; _23 = vUp.z; _24 = vUp.w; }
	void SetFace(const Vector4f & vFace) { _31 = vFace.x; _32 = vFace.y; _33 = vFace.z; _34 = vFace.w; }
	void SetPos(const Vector4f & vPos) { _41 = vPos.x; _42 = vPos.y; _43 = vPos.z; _44 = vPos.w; }
	void SetPos(const Vector3f & vPos) { _41 = vPos.x; _42 = vPos.y; _43 = vPos.z; _44 = 1.0f; }

	void TransformPerspective(float x, float y, float z, Vector4f& vecOut);


	static Matrix4x4 TextureBiasMatrix();
	static Matrix4x4 TranslationMatrix(const Vector3f& t)
	{
		Matrix4x4 mtx;
		mtx.LoadIdentity();
		mtx.SetTranslation(t);
		return mtx;
	}
	
	union
	{
		float M[4][4];
		float m[16];
		struct
		{
			float32 _11, _12, _13, _14;
			float32 _21, _22, _23, _24;
			float32 _31, _32, _33, _34;
			float32 _41, _42, _43, _44;
		};
	};

	static Matrix4x4 IdentityOnce();

	static const Matrix4x4& Identity()
	{
		static Matrix4x4 identityMat = Matrix4x4::IdentityOnce();
		return identityMat;
	}

private:
	void Assign(const Matrix4x3& in);
	void LookAtInt(const Vector3f& vEyePos, const Vector3f& vZAxis, const Vector3f& vUp);
};

inline Matrix4x4 Matrix4x4::IdentityOnce()
{
	Matrix4x4 tmp;
	tmp._11 = 1.0f; tmp._12 = 0.0f;  tmp._13 = 0.0f;  tmp._14 = 0.0f;
	tmp._21 = 0.0f; tmp._22 = 1.0f;  tmp._23 = 0.0f;  tmp._24 = 0.0f;
	tmp._31 = 0.0f; tmp._32 = 0.0f;  tmp._33 = 1.0f;  tmp._34 = 0.0f;
	tmp._41 = 0.0f; tmp._42 = 0.0f;  tmp._43 = 0.0f;  tmp._44 = 1.0f;
	return tmp;
}


inline void Matrix4x4::Translate( const Vector4f &translation )
{
	ASSERT(translation.w == 0.0f);
	_41 += translation.x;
	_42 += translation.y;
	_43 += translation.z;
}

inline void Matrix4x4::Translate( const Vector3f &translation )
{
	_41 += translation.x;
	_42 += translation.y;
	_43 += translation.z;
}


inline void Matrix4x4::Translate( float x, float y, float z )
{
	_41 += x;
	_42 += y;
	_43 += z;
}

inline void Matrix4x4::SetTranslation( const Vector4f &pos )
{
	ASSERT(pos.w == 1.0f);
	_41 = pos.x;
	_42 = pos.y;
	_43 = pos.z;
}

inline void Matrix4x4::SetTranslation( const Vector3f &pos )
{
	_41 = pos.x;
	_42 = pos.y;
	_43 = pos.z;
}

inline void Matrix4x4::SetTranslation( float x, float y, float z )
{
	_41 = x;
	_42 = y;
	_43 = z;
}

inline Vector3f Matrix4x4::TransformVec3(const Vector3f& vec, float fW) const
{
	// Perform the dot product with each row in the matrix
	return Vector3f(
	vec.x*_11 + vec.y*_21 + vec.z*_31 + fW*_41,
	vec.x*_12 + vec.y*_22 + vec.z*_32 + fW*_42,
	vec.x*_13 + vec.y*_23 + vec.z*_33 + fW*_43 );
}

inline void Matrix4x4::TransformPerspective(float x, float y, float z, Vector4f& vecOut)
{
	float fW = x*_14 + y*_24 + z*_34 + _44;

	float fX = x*_11 + y*_21 + z*_31 + _41;
	float fY = x*_12 + y*_22 + z*_32 + _42;
	float fZ = x*_13 + y*_23 + z*_33 + _43;

	vecOut.Assign(fX/fW, fY/fW, fZ/fW, 1.0f);
}

#include API_HEADER(Engine/Maths, Matrix4x4_ps.h)

}

#endif

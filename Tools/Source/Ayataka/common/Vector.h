#ifndef VECTOR_H
#define VECTOR_H

#include <math.h>

class Vector2 {
public:
	Vector2( void ) {
		assign( 0.0f, 0.0f );
	}
	Vector2( float _x, float _y ) {
		assign( _x, _y );
	}

	void assign( float _x, float _y ) {
		x = _x; y = _y;
	}

	float calcLength( void ) {
		return sqrtf( ( x * x ) + ( y * y ) );
	}

	void normalize( void ) {
		float length = calcLength();
		x /= length;
		y /= length;
	}

	void copy( float out[] ) {
		out[0] = x;
		out[1] = y;
	}

	void operator += ( const Vector2 &add ) {
		x += add.x;
		y += add.y;
	}

	Vector2 operator + ( const Vector2 &add ) const {
		return Vector2( x + add.x, y + add.y );
	}

	void operator -= ( const Vector2 &sub ) {
		x -= sub.x;
		y -= sub.y;
	}

	Vector2 operator - ( const Vector2 &sub ) const {
		return Vector2( x - sub.x, y - sub.y );
	}

	void operator *= ( float scalar ) {
		x *= scalar;
		y *= scalar;
	}
	Vector2 operator * ( float scalar ) const {
		return Vector2( x * scalar, y * scalar );
	}

	void operator /= ( float scalar ) {
		x /= scalar;
		y /= scalar;
	}
	Vector2 operator / ( float scalar ) const {
		return Vector2( x / scalar, y / scalar );
	}

	void operator = ( const Vector2 subst ) {
		x = subst.x;
		y = subst.y;
	}

	float x, y;
};

class Vector3 {
public:
	Vector3( void ) {
		assign( 0.0f, 0.0f, 0.0f );
	}
	Vector3( float _x, float _y, float _z ) {
		assign( _x, _y, _z );
	}

	void assign( float _x, float _y, float _z ) {
		x = _x; y = _y; z = _z;
	}

	float calcLength( void ) {
		return sqrtf( ( x * x ) + ( y * y ) + ( z * z ) );
	}

	void normalize( void ) {
		float length = calcLength();
		x /= length;
		y /= length;
		z /= length;
	}

	void copy( float out[] ) {
		out[0] = x;
		out[1] = y;
		out[2] = z;
	}

	void operator += (const Vector3 &add) {
		x += add.x;
		y += add.y;
		z += add.z;
	}

	Vector3 operator + (const Vector3 &add) const {
		return Vector3( x + add.x, y + add.y, z + add.z );
	}

	void operator -= (const Vector3 &sub) {
		x -= sub.x;
		y -= sub.y;
		z -= sub.z;
	}

	Vector3 operator - (const Vector3 &sub) const {
		return Vector3( x - sub.x, y - sub.y, z - sub.z );
	}

	void operator *= (float scalar) {
		x *= scalar;
		y *= scalar;
		z *= scalar;
	}
	Vector3 operator * (float scalar) const {
		return Vector3( x * scalar, y * scalar, z * scalar );
	}

	void operator /= (float scalar) {
		x /= scalar;
		y /= scalar;
		z /= scalar;
	}
	Vector3 operator / (float scalar) const {
		return Vector3( x / scalar, y / scalar, z / scalar );
	}

	void operator = ( const Vector3 subst ) {
		x = subst.x;
		y = subst.y;
		z = subst.z;

	}

	float x, y, z;
};

class Vector4 {
public:
	Vector4( void ) {
		assign( 0.0f, 0.0f, 0.0f, 0.0f );
	}
	Vector4( float _x, float _y, float _z, float _w ) {
		assign( _x, _y, _z, _w );
	}

	void assign( float _x, float _y, float _z, float _w ) {
		x = _x; y = _y; z = _z; w = _w;
	}

	float x, y, z, w;
};

#endif // VECTOR_H

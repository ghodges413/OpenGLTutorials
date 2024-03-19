//
//	Quat.h
//
#pragma once
#include "Vector.h"
#include "Matrix.h"

/*
 ================================
 Quat
 ================================
 */
class Quat {
public:
	Quat();	
	Quat( const Quat & rhs );
	Quat( float X, float Y, float Z, float W );
	Quat( float X, float Y, float Z );
	Quat( Vec3 n, const float angleRadians );
	Quat( float theta, float phi );
	const Quat & operator = ( const Quat & rhs );
	
	Quat &	operator *= ( const float & rhs );
	Quat &	operator *= ( const Quat & rhs );
	Quat	operator * ( const Quat & rhs ) const;
	Quat	operator - ( const Quat & rhs ) const;

	void	Normalize();
	void	Invert();
	Quat	Inverse() const;
	float	MagnitudeSquared() const;
	float	GetMagnitude() const;
	Vec3	RotatePoint( const Vec3 & rhs ) const;
	Mat3	RotateMatrix( const Mat3 & rhs ) const;
	Vec3	xyz() const { return Vec3( x, y, z ); }
	bool	IsValid() const;
	void	ComputeW();

	Mat3	ToMat3() const;
	Mat4	ToMat4() const;
	Vec4	ToVec4() const { return Vec4( w, x, y, z ); }

	const float *   ToPtr() const   { return &x; }
	float *         ToPtr()         { return &x; }

public:
	float w;
	float x;
	float y;
	float z;
};

inline Quat::Quat() :
x( 0 ),
y( 0 ),
z( 0 ),
w( 1 ) {
}

inline Quat::Quat( const Quat &rhs ) :
x( rhs.x ),
y( rhs.y ),
z( rhs.z ),
w( rhs.w ) {
}

inline Quat::Quat( float X, float Y, float Z, float W ) :
x( X ),
y( Y ),
z( Z ),
w( W ) {
}

inline Quat::Quat( float X, float Y, float Z ) :
x( X ),
y( Y ),
z( Z ) {
	ComputeW();
}

inline Quat::Quat( Vec3 n, const float angleRadians ) {
	const float halfAngleRadians = 0.5f * angleRadians;

	w = cosf( halfAngleRadians );

	const float halfSine = sinf( halfAngleRadians );
	n.Normalize();
	x = n.x * halfSine;
	y = n.y * halfSine;
	z = n.z * halfSine;
}

inline Quat::Quat( float theta, float phi ) {
	const float pi = acosf( -1 );
	theta -= pi * 0.5f;

	Quat tmpA = Quat( Vec3( 0, 1, 0 ), theta );
	Quat tmpB = Quat( Vec3( 0, 0, 1 ), phi );

	*this = tmpB * tmpA;
}

inline const Quat & Quat::operator = ( const Quat & rhs ) {
	x = rhs.x;
	y = rhs.y;
	z = rhs.z;
	w = rhs.w;
	return *this;
}

inline Quat & Quat::operator *= ( const float & rhs ) {
    x *= rhs;
    y *= rhs;
    z *= rhs;
    w *= rhs;
    return *this;
}

inline Quat & Quat::operator *= ( const Quat & rhs ) {
	Quat temp = *this * rhs;
	w = temp.w;
	x = temp.x;
	y = temp.y;
	z = temp.z;
	return *this;
}

inline Quat Quat::operator * ( const Quat & rhs ) const {
	Quat temp;	
	temp.w = ( w * rhs.w ) - ( x * rhs.x ) - ( y * rhs.y ) - ( z * rhs.z );
	temp.x = ( x * rhs.w ) + ( w * rhs.x ) + ( y * rhs.z ) - ( z * rhs.y );
	temp.y = ( y * rhs.w ) + ( w * rhs.y ) + ( z * rhs.x ) - ( x * rhs.z );
	temp.z = ( z * rhs.w ) + ( w * rhs.z ) + ( x * rhs.y ) - ( y * rhs.x );
	return temp;
}

inline Quat Quat::operator - ( const Quat & rhs ) const {
	Quat temp;	
	temp.w = w - rhs.w;
	temp.x = x - rhs.x;
	temp.y = y - rhs.y;
	temp.z = z - rhs.z;
	return temp;
}

inline void Quat::Normalize() {
	float invMag = 1.0f / GetMagnitude();
	
	if ( 0.0f * invMag == 0.0f * invMag ) {
		x = x * invMag;
		y = y * invMag;
		z = z * invMag;
		w = w * invMag;
	}
}

inline void Quat::Invert() {
    *this *= 1.0f / MagnitudeSquared();
    x = -x;
    y = -y;
    z = -z;
}

inline Quat Quat::Inverse() const {
    Quat val( *this );
    val.Invert();
    return val;
}

inline float Quat::MagnitudeSquared() const {
    return ( ( x * x ) + ( y * y ) + ( z * z ) + ( w * w ) );
}

inline float Quat::GetMagnitude() const {
	return sqrtf( MagnitudeSquared() );
}

inline Vec3 Quat::RotatePoint( const Vec3 & rhs ) const {
	Quat vector( rhs.x, rhs.y, rhs.z, 0.0f );
	Quat final = *this * vector * Inverse();
	return Vec3( final.x, final.y, final.z );
}

inline bool Quat::IsValid() const {
	if ( x * 0 != x * 0 ) {
		return false;
	}
	if ( y * 0 != y * 0 ) {
		return false;
	}
	if ( z * 0 != z * 0 ) {
		return false;
	}
	if ( w * 0 != w * 0 ) {
		return false;
	}
	return true;
}

inline Mat3 Quat::RotateMatrix( const Mat3 & rhs ) const {
	Mat3 mat;
	mat.rows[ 0 ] = RotatePoint( rhs.rows[ 0 ] );
	mat.rows[ 1 ] = RotatePoint( rhs.rows[ 1 ] );
	mat.rows[ 2 ] = RotatePoint( rhs.rows[ 2 ] );
	return mat;
}

inline Mat3 Quat::ToMat3() const {
	Mat3 mat;
	mat.Identity();

	mat.rows[ 0 ] = RotatePoint( mat.rows[ 0 ] );
	mat.rows[ 1 ] = RotatePoint( mat.rows[ 1 ] );
	mat.rows[ 2 ] = RotatePoint( mat.rows[ 2 ] );
	return mat;
}

inline Mat4 Quat::ToMat4() const {
	Mat4 mat;
	mat.Identity();

	Vec3 rows0 = RotatePoint( Vec3( 1, 0, 0 ) );
	Vec3 rows1 = RotatePoint( Vec3( 0, 1, 0 ) );
	Vec3 rows2 = RotatePoint( Vec3( 0, 0, 1 ) );
	mat.rows[ 0 ] = Vec4( rows0.x, rows0.y, rows0.z, 0.0f );
	mat.rows[ 1 ] = Vec4( rows1.x, rows1.y, rows1.z, 0.0f );
	mat.rows[ 2 ] = Vec4( rows2.x, rows2.y, rows2.z, 0.0f );
	return mat;
}

inline void Quat::ComputeW() {
	float w2 = 1.0f - x * x - y * y - z * z;
	if ( w2 > 0.0f ) {
		w = -sqrtf( w2 );
	} else {
		w = 0.0f;
	}
}
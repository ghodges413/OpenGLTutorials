/*
 *  Quat.h
 *
 */
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
                        Quat( const Vec3d & rhs );
                        Quat( const float & X, const float & Y, const float & Z );
                        Quat( const float & X, const float & Y, const float & Z, const float & W );
						Quat( const Vec3d & n, const float angleDegrees );
    const   Quat &    operator = ( const Quat & rhs );
                        ~Quat();
	
            bool        operator == ( const Quat & rhs ) const;
            Quat      operator + ( const Quat & rhs ) const;
            Quat &    operator += ( const Quat & rhs );
            Quat &    operator -= ( const Quat & rhs );
            Quat &    operator *= ( const float & rhs );
			Quat &    operator *= ( const Quat & rhs );
            Quat &    operator /= ( const float & rhs );
            Quat      operator - ( const Quat & rhs ) const;
            Quat      operator * ( const float & rhs ) const;
            Quat      operator * ( const Vec3d & rhs ) const;
            Quat      operator * ( const Quat & rhs ) const;
    
    const   float &     operator[] ( const int & idx ) const;
            float       operator[] ( const int & idx );

    const   float *     ToPtr() const   { return &x; }
            float *     ToPtr()         { return &x; }
    
            void        Normalize();
            void        NormalizeAndInvert();
            void        Invert();
            Quat      Inverse() const;
            void        ComputeW();
            float       MagnitudeSquared() const;
            float       GetMagnitude() const;
            Vec3d     RotatePoint( const Vec3d & rhs ) const;
            
            Matrix    ToMatrix() const;
    
    friend  Quat      operator * ( const int & lhs, const Quat & rhs );
    friend  Quat      operator * ( const float & lhs, const Quat & rhs );
	
public:
	// Remember that this xyz is a quaternion... it's not a direct xyz vector
	float x;
	float y;
	float z;
	float w;
};
inline Quat operator * ( const int & lhs, const Quat & rhs ) {
    return rhs * lhs;
}
inline Quat operator * ( const float & lhs, const Quat & rhs ) {
    return rhs * lhs;
}

/*
 ================================
 Quat::Quat
 Since we only use quaternions for rotation,
 initialize to an identity quat that performs no rotation.
 ================================
 */
inline Quat::Quat() :
x( 0 ),
y( 0 ),
z( 1 ),
w( 0 ) {
}

/*
 ================================
 Quat::Quat
 ================================
 */
inline Quat::Quat( const Quat &rhs ) :
x( rhs.x ),
y( rhs.y ),
z( rhs.z ),
w( rhs.w ) {
}

/*
 ================================
 Quat::Quat
 ================================
 */
inline Quat::Quat( const Vec3d & rhs ) :
x( rhs.x ),
y( rhs.y ),
z( rhs.z ),
w( 0 ) {
    ComputeW();
}

/*
 ================================
 Quat::Quat
 ================================
 */
inline Quat::Quat( const float & X, const float & Y, const float & Z ) :
x( X ),
y( Y ),
z( Z ) {
	ComputeW();
}

/*
 ================================
 Quat::Quat
 ================================
 */
inline Quat::Quat( const float & X, const float & Y, const float & Z, const float & W ) :
x( X ),
y( Y ),
z( Z ),
w( W ) {
}

/*
 ================================
 Quat::operator =
 ================================
 */
inline const Quat & Quat::operator = ( const Quat & rhs ) {
	x = rhs.x;
	y = rhs.y;
	z = rhs.z;
	w = rhs.w;
	return *this;
}

/*
 ================================
 Quat::~Quat
 ================================
 */
inline Quat::~Quat() {
}

/*
 ================================
 Quat::operator == 
 ================================
 */
inline bool Quat::operator == ( const Quat & rhs ) const {
	if ( x != rhs.x ) {
		return false;
	}
	if ( y != rhs.y ) {
		return false;
	}
	if ( z != rhs.z ) {
		return false;
	}
	if ( w != rhs.w ) {
		return false;
	}
	
	return true;
}

/*
 ================================
 Quat::operator +
 ================================
 */
inline Quat Quat::operator + ( const Quat & rhs ) const {
	Quat temp;
	temp.x = x + rhs.x;
	temp.y = y + rhs.y;
	temp.z = z + rhs.z;
	temp.w = w + rhs.w;
	return temp;
}

/*
 ================================
 Quat::operator +=
 ================================
 */
inline Quat & Quat::operator += ( const Quat & rhs ) {
	x += rhs.x;
	y += rhs.y;
	z += rhs.z;
	w += rhs.w;
	return *this;
}

/*
 ================================
 Quat::operator -=
 ================================
 */
inline Quat & Quat::operator -= ( const Quat & rhs ) {
	x -= rhs.x;
	y -= rhs.y;
	z -= rhs.z;
	w -= rhs.w;
	return *this;
}

/*
 ================================
 Quat::operator *=
 ================================
 */
inline Quat & Quat::operator *= ( const float & rhs ) {
    x *= rhs;
    y *= rhs;
    z *= rhs;
    w *= rhs;
    return *this;
}

/*
 ================================
 Quat::operator *=
 ================================
 */
inline Quat & Quat::operator *= ( const Quat & rhs ) {
	Quat temp = *this * rhs;
	w = temp.w;
	x = temp.x;
	y = temp.y;
	z = temp.z;
	return *this;
}

/*
 ================================
 Quat::operator /=
 ================================
 */
inline Quat & Quat::operator /= ( const float & rhs ) {
    x /= rhs;
    y /= rhs;
    z /= rhs;
    w /= rhs;
    return *this;
}

/*
 ================================
 Quat::operator -
 ================================
 */
inline Quat Quat::operator - ( const Quat & rhs ) const {
	Quat temp;
	temp.x = x - rhs.x;
	temp.y = y - rhs.y;
	temp.z = z - rhs.z;
	temp.w = w - rhs.w;
	return temp;
}

/*
 ================================
 Quat::operator *
 ================================
 */
inline Quat Quat::operator * ( const float & rhs ) const {
	Quat temp;
	temp.x = x * rhs;
	temp.y = y * rhs;
	temp.z = z * rhs;
	temp.w = w * rhs;
	return temp;
}

/*
 ================================
 Quat::operator * 
 ================================
 */
inline Quat Quat::operator * ( const Vec3d & rhs ) const {
	Quat temp;
	temp.w = - ( x * rhs.x ) - ( y * rhs.y ) - ( z * rhs.z );
	temp.x =   ( w * rhs.x ) + ( y * rhs.z ) - ( z * rhs.y );
	temp.y =   ( w * rhs.y ) + ( z * rhs.x ) - ( x * rhs.z );
	temp.z =   ( w * rhs.z ) + ( x * rhs.y ) - ( y * rhs.x );
	return temp;
}

/*
 ================================
 Quat::operator *
 ================================
 */
inline Quat Quat::operator * ( const Quat & rhs ) const {
	Quat temp;	
	temp.w = ( w * rhs.w ) - ( x * rhs.x ) - ( y * rhs.y ) - ( z * rhs.z );
	temp.x = ( x * rhs.w ) + ( w * rhs.x ) + ( y * rhs.z ) - ( z * rhs.y );
	temp.y = ( y * rhs.w ) + ( w * rhs.y ) + ( z * rhs.x ) - ( x * rhs.z );
	temp.z = ( z * rhs.w ) + ( w * rhs.z ) + ( x * rhs.y ) - ( y * rhs.x );
	return temp;
}

/*
 ================================
 Quat::operator []
 ================================
 */
inline const float & Quat::operator[] ( const int & idx ) const {
    assert( idx >= 0 );
    assert( idx < 4 );
    return *( ToPtr() + idx );
}

/*
 ================================
 Quat::operator []
 ================================
 */
inline float Quat::operator[] ( const int & idx ) {
    assert( idx >= 0 );
    assert( idx < 4 );
    return *( ToPtr() + idx );
}

/*
 ================================
 Quat::Normalize
 ================================
 */
inline void Quat::Normalize() {
	float mag = GetMagnitude();
	
	if ( mag > 0.0001f || mag < -0.0001f ) { //don't divide by zero
		x = x / mag;
		y = y / mag;
		z = z / mag;
		w = w / mag;
	}
}

/*
 ================================
 Quat::NormalizeAndInvert
 ================================
 */
inline void Quat::NormalizeAndInvert() {
	Normalize();
	x = -x;
	y = -y;
	z = -z;
}

/*
 ================================
 Quat::Invert
 ================================
 */
inline void Quat::Invert() {
    *this /= MagnitudeSquared();
    x = -x;
    y = -y;
    z = -z;
}

/*
 ================================
 Quat::Inverse
 ================================
 */
inline Quat Quat::Inverse() const {
    Quat val( *this );
    val /= val.MagnitudeSquared();
    val.x = -x;
    val.y = -y;
    val.z = -z;
    return val;
}

/*
 ================================
 Quat::ComputeW
 ================================
 */
inline void Quat::ComputeW() {
	float t = 1.0f - ( x * x ) - ( y * y ) - ( z * z );
	
	if ( t < 0.0f ) {
        // This shouldn't happen.  This happens when the quaternion is not normalized.
        // But if the quaternion is not normalized, then this is not being used
        // in animation.  And therefore ComputeW shouldn't get called,
        // since it's impossible to compute W if we do not know the
        // length of the quaternion.
		w = 0.0f;
	} else {
		w = -sqrt( t );
	}
}

/*
 ================================
 Quat::MagnitudeSquared
 ================================
 */
inline float Quat::MagnitudeSquared() const {
    return ( ( x * x ) + ( y * y ) + ( z * z ) + ( w * w ) );
}

/*
 ================================
 Quat::GetMagnitude
 ================================
 */
inline float Quat::GetMagnitude() const {
    return sqrtf( MagnitudeSquared() );
}

/*
 ================================
 Quat::RotatePoint
 ================================
 */
inline Vec3d Quat::RotatePoint( const Vec3d & rhs ) const {
	// This could use some optimization.  Do the calculation on paper
    // and program the result directly.

	// multiply quat by vector
	Quat tmp( x, y, z, w );
	tmp = tmp * rhs;
	
//	// get inverse rotation
//	Quat inv( x, y, z, w );
//	inv.NormalizeAndInvert();
	
	// return final
	Quat final = tmp * Inverse();
	return Vec3d( final.x, final.y, final.z );
}

/*
 ================================
 Quat::ToMatrix
 ================================
 */
inline Matrix Quat::ToMatrix() const {
    //    2x^2 + 2w^2 -1,   2xy - 2zw, 2xz + 2yw,
    //    2xy + 2zw, 2y^2 + 2w^2 - 1, 2yz - 2xw,
    //    2xz - 2yw, 2yz + 2xw, 2z^2 + 2w^2 -1
    //      Results in a rotation about x,y,z by 2*theta
    //      where cos( theta ) = w
    Matrix mat;
    float xx = x * x;
    float yy = y * y;
    float zz = z * z;
    float ww = w * w;
    
    float xy = x * y;
    float xz = x * z;
    float xw = x * w;
    
    float yw = y * w;
    float yz = y * z;
    
    float zw = z * w;

    mat[0][0] = ww + xx - yy - zz;
    mat[0][1] = ( xy + zw ) * 2.0f;
    mat[0][2] = ( xz - yw ) * 2.0f;
    mat[0][3] = 0;
    
    mat[1][0] = ( xy - zw ) * 2.0f;
    mat[1][1] = ww - xx + yy - zz;
    mat[1][2] = ( yz + xw ) * 2.0f;
    mat[1][3] = 0;
    
    mat[2][0] = ( xz + yw ) * 2.0f;
    mat[2][1] = ( yz - xw ) * 2.0f;
    mat[2][2] = ww - xx - yy + zz;
    mat[2][3] = 0;
    
    mat[3][0] = 0;
    mat[3][1] = 0;
    mat[3][2] = 0;
    mat[3][3] = 1;
    return mat;
}






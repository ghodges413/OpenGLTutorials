//
//  Complex.h
//
#pragma once
#include "Math/Vector.h"
#include "Math/Matrix.h"

/*
 ================================
 Complex
 
 This implementation is of the straight forward imaginary numbers.
 It is not treated as a spinor.  I don't really know if there's an
 advantage to treating as a spinor.
 ================================
 */
class Complex {
public:
                            Complex();
                            Complex( const Complex & rhs );
                            Complex( const Vec2d & rhs );
                            Complex( const float & radius, const float & theta );
    const   Complex &     operator = ( const Complex & rhs );
                            ~Complex();
	
            bool            operator == ( const Complex & rhs ) const;
            Complex       operator + ( const Complex & rhs ) const;
            Complex &     operator += ( const Complex & rhs );
            Complex &     operator -= ( const Complex & rhs );
            Complex &     operator *= ( const float & rhs );
            Complex &     operator /= ( const float & rhs );
            Complex       operator - ( const Complex & rhs ) const;
            Complex       operator * ( const float & rhs ) const;
            Complex       operator * ( const Complex & rhs ) const;
    
    const   float &     operator[] ( const int & idx ) const;
            float       operator[] ( const int & idx );

    const   float *     ToPtr() const   { return &x; }
            float *     ToPtr()         { return &x; }
    
			void		Zero() { r = 0.0f; i = 0.0f; }
            void        Normalize();
            void        NormalizeAndInvert();
            void        Invert();
            Complex   Inverse() const;
            float       MagnitudeSquared() const;
            float       GetMagnitude() const;
            Vec2d     RotatePoint( const Vec2d & rhs ) const;
            
            Matrix    ToMatrix() const;
    
    friend  Complex   operator * ( const int & lhs, const Complex & rhs );
    friend  Complex   operator * ( const float & lhs, const Complex & rhs );
	
public:
    union {
        float x;
        float real;
		float r;
    };

    union {
        float y;
        float imaginary;
		float i;
    };
};
inline Complex operator * ( const int & lhs, const Complex & rhs ) {
    return rhs * lhs;
}
inline Complex operator * ( const float & lhs, const Complex & rhs ) {
    return rhs * lhs;
}

/*
 ================================
 Complex::Complex
 ================================
 */
inline Complex::Complex() :
x( 0 ),
y( 0 ) {
}

/*
 ================================
 Complex::Complex
 ================================
 */
inline Complex::Complex( const Complex &rhs ) :
x( rhs.x ),
y( rhs.y ) {
}

/*
 ================================
 Complex::Complex
 ================================
 */
inline Complex::Complex( const Vec2d & rhs ) :
x( rhs.x ),
y( rhs.y ) {
}

/*
 ================================
 Complex::Complex
 
 Assumes theta is in radians.
 ================================
 */
inline Complex::Complex( const float & radius, const float & theta ) {
    assert( radius > 0 );
    x = radius * cosf( theta );
    y = radius * sinf( theta );
}

/*
 ================================
 Complex::operator =
 ================================
 */
inline const Complex & Complex::operator = ( const Complex & rhs ) {
	x = rhs.x;
	y = rhs.y;
	return *this;
}

/*
 ================================
 Complex::~Complex
 ================================
 */
inline Complex::~Complex() {
}

/*
 ================================
 Complex::operator == 
 ================================
 */
inline bool Complex::operator == ( const Complex & rhs ) const {
	if ( x != rhs.x ) {
		return false;
	}
	if ( y != rhs.y ) {
		return false;
	}
	
	return true;
}

/*
 ================================
 Complex::operator +
 ================================
 */
inline Complex Complex::operator + ( const Complex & rhs ) const {
	Complex temp;
	temp.x = x + rhs.x;
	temp.y = y + rhs.y;
	return temp;
}

/*
 ================================
 Complex::operator +=
 ================================
 */
inline Complex & Complex::operator += ( const Complex & rhs ) {
	x += rhs.x;
	y += rhs.y;
	return *this;
}

/*
 ================================
 Complex::operator -=
 ================================
 */
inline Complex & Complex::operator -= ( const Complex & rhs ) {
	x -= rhs.x;
	y -= rhs.y;
	return *this;
}

/*
 ================================
 Complex::operator *=
 ================================
 */
inline Complex & Complex::operator *= ( const float & rhs ) {
    x *= rhs;
    y *= rhs;
    return *this;
}

/*
 ================================
 Complex::operator /=
 ================================
 */
inline Complex & Complex::operator /= ( const float & rhs ) {
    x /= rhs;
    y /= rhs;
    return *this;
}

/*
 ================================
 Complex::operator -
 ================================
 */
inline Complex Complex::operator - ( const Complex & rhs ) const {
	Complex temp;
	temp.x = x - rhs.x;
	temp.y = y - rhs.y;
	return temp;
}

/*
 ================================
 Complex::operator *
 ================================
 */
inline Complex Complex::operator * ( const float & rhs ) const {
	Complex temp;
	temp.x = x * rhs;
	temp.y = y * rhs;
	return temp;
}

/*
 ================================
 Complex::operator *
 ================================
 */
inline Complex Complex::operator * ( const Complex & rhs ) const {
	Complex temp;	
    temp.x = x * rhs.x - y * rhs.y;
	temp.y = y * rhs.x + x * rhs.y;
	return temp;
}

/*
 ================================
 Complex::operator []
 ================================
 */
inline const float & Complex::operator[] ( const int & idx ) const {
    assert( idx >= 0 );
    assert( idx < 2 );
    return *( ToPtr() + idx );
}

/*
 ================================
 Complex::operator []
 ================================
 */
inline float Complex::operator[] ( const int & idx ) {
    assert( idx >= 0 );
    assert( idx < 2 );
    return *( ToPtr() + idx );
}

/*
 ================================
 Complex::Normalize
 ================================
 */
inline void Complex::Normalize() {
	float mag = GetMagnitude();
	
	if ( mag > 0.0001f || mag < -0.0001f ) { //don't divide by zero
		x = x / mag;
		y = y / mag;
	}
}

/*
 ================================
 Complex::NormalizeAndInvert
 ================================
 */
inline void Complex::NormalizeAndInvert() {
	Normalize();
	y = -y;
}

/*
 ================================
 Complex::Invert
 ================================
 */
inline void Complex::Invert() {
    *this /= MagnitudeSquared();
    y = -y;
}

/*
 ================================
 Complex::Inverse
 ================================
 */
inline Complex Complex::Inverse() const {
    Complex val( *this );
    val /= val.MagnitudeSquared();
    val.y = -y;
    return val;
}

/*
 ================================
 Complex::MagnitudeSquared
 ================================
 */
inline float Complex::MagnitudeSquared() const {
    return ( ( x * x ) + ( y * y ) );
}

/*
 ================================
 Complex::GetMagnitude
 ================================
 */
inline float Complex::GetMagnitude() const {
    return sqrtf( MagnitudeSquared() );
}

/*
 ================================
 Complex::RotatePoint
 ================================
 */
inline Vec2d Complex::RotatePoint( const Vec2d & rhs ) const {
    Complex tmp( rhs );
    tmp = ( *this ) * tmp;
    
    return Vec2d( tmp.x, tmp.y );
}

/*
 ================================
 Complex::ToMatrix
 
 This assumes that Complex is normalized.
 ================================
 */
inline Matrix Complex::ToMatrix() const {
    Matrix mat;

    mat[0][0] = x;
    mat[0][1] = y;
    mat[0][2] = 0;
    mat[0][3] = 0;
    
    mat[1][0] = -y;
    mat[1][1] = x;
    mat[1][2] = 0;
    mat[1][3] = 0;
    
    mat[2][0] = 0;
    mat[2][1] = 0;
    mat[2][2] = 0;
    mat[2][3] = 0;
    
    mat[3][0] = 0;
    mat[3][1] = 0;
    mat[3][2] = 0;
    mat[3][3] = 1;
    return mat;
}






//
//  Complex.h
//
#pragma once
#include "Vector.h"
#include "Matrix.h"

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
    Complex( const Vec2 & rhs );
    Complex( const float & radius, const float & theta );
    const   Complex &   operator = ( const Complex & rhs );
    ~Complex();
	
    bool          operator == ( const Complex & rhs ) const;
    Complex       operator + ( const Complex & rhs ) const;
    Complex &     operator += ( const Complex & rhs );
    Complex &     operator -= ( const Complex & rhs );
    Complex &     operator *= ( const float & rhs );
    Complex &     operator *= ( const Complex & rhs );
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
    Complex     Inverse() const;
    float       MagnitudeSquared() const;
    float       GetMagnitude() const;
    Vec2        RotatePoint( const Vec2 & rhs ) const;
    bool        IsValid() const;
    
    friend  Complex   operator * ( const int lhs, const Complex & rhs );
    friend  Complex   operator * ( const float lhs, const Complex & rhs );
	
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
inline Complex operator * ( const int lhs, const Complex & rhs ) {
    return rhs * (float)lhs;
}
inline Complex operator * ( const float lhs, const Complex & rhs ) {
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
inline Complex::Complex( const Vec2 & rhs ) :
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
 Complex::operator *=
 ================================
 */
inline Complex & Complex::operator *= ( const Complex & rhs ) {
    float tmpX = x;
    x = x * rhs.x - y * rhs.y;
	y = y * rhs.x + tmpX * rhs.y;
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
inline Vec2 Complex::RotatePoint( const Vec2 & rhs ) const {
    Complex tmp( rhs );
    tmp = ( *this ) * tmp;
    
    return Vec2( tmp.x, tmp.y );
}

/*
 ================================
 Complex::IsValid
 ================================
 */
inline bool Complex::IsValid() const {
	if ( x * 0.0f != x * 0.0f ) {
		// x is NaN or Inf
		return false;
	}
	
	if ( y * 0.0f != y * 0.0f ) {
		// y is NaN or Inf
		return false;
	}
	
	return true;
}




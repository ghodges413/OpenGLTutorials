//
//	Vector.h
//
#pragma once
#include <math.h>
#include <assert.h>

/*
 ================================
 Vec2d
 ================================
 */
class Vec2d {
public:
	Vec2d();
    Vec2d( const int& value );
	Vec2d( const float& value );
	Vec2d( const Vec2d& rhs );     //copy ctor
	Vec2d( float X, float Y );
	Vec2d( const float * xy );
	Vec2d& operator=( const Vec2d& rhs );  // assignment operator
	
	~Vec2d();
	
	bool			operator==( const Vec2d& rhs ) const;
	bool			operator != ( const Vec2d& rhs ) const;
	Vec2d			operator+( const Vec2d& rhs ) const;
	const Vec2d&	operator+=( const Vec2d& rhs );
	const Vec2d&	operator-=( const Vec2d& rhs );
	Vec2d			operator-( const Vec2d& rhs ) const;
	Vec2d			operator*( const Vec2d& rhs ) const;
	Vec2d			operator*( const float& rhs ) const;
    const Vec2d&	operator*=( const float& rhs );
	Vec2d			operator/( const float& rhs ) const;
    const Vec2d&	operator/=( const float& rhs );
	float			operator[]( const int& idx ) const;
	float&			operator[]( const int& idx );
	
	const Vec2d & Normalize();
	float GetMagnitude() const;
	bool IsValid() const;
	float Dot( const Vec2d & rhs ) const;
	
	const float * ToPtr() const { return &x; }
    
    friend Vec2d operator*( const int& lhs, const Vec2d& rhs );
    friend Vec2d operator*( const float& lhs, const Vec2d& rhs );
	
public:
	float x;
	float y;
};

/*
 ================================
 Vec2d::Vec2d
 ================================
 */
inline Vec2d::Vec2d() : 
x( 0 ), 
y( 0 ) {
}

/*
 ================================
 Vec2d::Vec2d
 ================================
 */
inline Vec2d::Vec2d( const int& value ) :
x( value ),
y( value ) {
}

/*
 ================================
 Vec2d::Vec2d
 ================================
 */
inline Vec2d::Vec2d( const float& value ) :
x( value ),
y( value ) {
}

/*
 ================================
 Vec2d::Vec2d
 ================================
 */
inline Vec2d::Vec2d( const Vec2d &rhs ) :
x( rhs.x ),
y( rhs.y ) {
}

/*
 ================================
 Vec2d::Vec2d
 ================================
 */
inline Vec2d::Vec2d( float X, float Y ) :
x( X ),
y( Y ) {
}

/*
 ================================
 Vec2d::Vec2d
 ================================
 */
inline Vec2d::Vec2d( const float * xy ) :
x( xy[ 0 ] ),
y( xy[ 1 ] ) {
}

/*
 ================================
 Vec2d::operator=
 ================================
 */
inline Vec2d& Vec2d::operator=( const Vec2d &rhs ) {
	x = rhs.x;
	y = rhs.y;
	return *this;
}

inline Vec2d::~Vec2d() {
}

/*
 ================================
 Vec2d::operator==
 ================================
 */
inline bool Vec2d::operator==( const Vec2d &rhs ) const {
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
 Vec2d::operator !=
 ================================
 */
inline bool Vec2d::operator != ( const Vec2d& rhs ) const {
	if ( *this == rhs ) {
		return false;
	}
	
	return true;
}

/*
 ================================
 Vec2d::operator+
 ================================
 */
inline Vec2d Vec2d::operator+( const Vec2d &rhs ) const {
	Vec2d temp;
	temp.x = x + rhs.x;
	temp.y = y + rhs.y;
	return temp;
}

/*
 ================================
 Vec2d::operator+=
 ================================
 */
inline const Vec2d& Vec2d::operator+=( const Vec2d &rhs ) {
	x += rhs.x;
	y += rhs.y;
	return *this;
}

/*
 ================================
 Vec2d::operator-=
 ================================
 */
inline const Vec2d& Vec2d::operator-=( const Vec2d &rhs ) {
	x -= rhs.x;
	y -= rhs.y;
	return *this;
}

/*
 ================================
 Vec2d::operator-
 ================================
 */
inline Vec2d Vec2d::operator-( const Vec2d &rhs ) const {
	Vec2d temp;
	temp.x = x - rhs.x;
	temp.y = y - rhs.y;
	return temp;
}

/*
 ================================
 Vec2d::operator*
 ================================
 */
inline Vec2d Vec2d::operator*( const Vec2d & rhs ) const {
	Vec2d temp;
	temp.x = x * rhs.x;
	temp.y = y * rhs.y;
	return temp;
}

/*
 ================================
 Vec2d::operator*
 ================================
 */
inline Vec2d Vec2d::operator*( const float &rhs ) const {
	Vec2d temp;
	temp.x = x * rhs;
	temp.y = y * rhs;
	return temp;
}

/*
 ================================
 Vec2d::operator*=
 ================================
 */
inline const Vec2d & Vec2d::operator*=( const float &rhs ) {
	x *= rhs;
	y *= rhs;
    return *this;
}

/*
 ================================
 Vec2d::operator/
 ================================
 */
inline Vec2d Vec2d::operator/( const float &rhs ) const {
	Vec2d temp;
	temp.x = x / rhs;
	temp.y = y / rhs;
	return temp;
}

/*
 ================================
 Vec2d::operator/=
 ================================
 */
inline const Vec2d & Vec2d::operator/=( const float &rhs ) {
	x /= rhs;
	y /= rhs;
    return *this;
}

/*
 ================================
 Vec2d::operator[]
 ================================
 */
inline float Vec2d::operator[]( const int& idx ) const {
	assert( idx >= 0 && idx < 2 );
	return ( &x )[ idx ];
}

/*
 ================================
 Vec2d::operator[]
 ================================
 */
inline float& Vec2d::operator[]( const int& idx ) {
	assert( idx >= 0 && idx < 2 );
	return ( &x )[ idx ];
}

/*
 ================================
 Vec2d::Normalize
 ================================
 */
inline const Vec2d & Vec2d::Normalize() {
	float mag = GetMagnitude();
	
	if ( mag > 0.0001f || mag < -0.0001f ) {
		//don't divide by zero
		x = x / mag;
		y = y / mag;
	}
    
    return *this;
}

/*
 ================================
 Vec2d::GetMagnitude
 ================================
 */
inline float Vec2d::GetMagnitude() const {
	float mag;
	
	mag = x*x + y*y;
	mag = sqrt(mag);
	
	return mag;
}

/*
 ================================
 Vec2d::IsValid
 ================================
 */
inline bool Vec2d::IsValid() const {
	if ( x != x ) {
		// x is NaN
		return false;
	}
	
	if ( y != y ) {
		// y is NaN
		return false;
	}
	
	return true;
}

/*
 ================================
 Vec2d::Dot
 ================================
 */
inline float Vec2d::Dot( const Vec2d & rhs ) const {
	return ( x * rhs.x ) + ( y * rhs.y );
}









/*
 ================================
 Vec3d
 ================================
 */
class Vec3d {
public:
	Vec3d();
	Vec3d( int value );
	Vec3d( float value );
	Vec3d( const Vec3d& rhs );     //copy ctor
	Vec3d( float X, float Y, float Z );
	Vec3d( const float * xyz );
	Vec3d& operator=(const Vec3d& rhs);  //assignment ctor
	
	~Vec3d();
    
	bool operator == ( const Vec3d& rhs ) const;
	bool operator != ( const Vec3d& rhs ) const;
	Vec3d operator+(const Vec3d& rhs) const;
	const Vec3d& operator+=(const Vec3d& rhs);
	const Vec3d& operator-=(const Vec3d& rhs);
	Vec3d operator-(const Vec3d& rhs) const;
	Vec3d operator*(const Vec3d& rhs) const;
	const Vec3d & operator*=(const Vec3d& rhs);
	Vec3d operator*(const float& rhs) const;
    Vec3d operator/(const float& rhs) const;
	const Vec3d& operator*=(const float& rhs);
    const Vec3d& operator/=(const float& rhs);
	float			operator[]( const int& idx ) const;
	float&			operator[]( const int& idx );
	bool operator > ( const Vec3d & rhs ) const;
	bool operator < ( const Vec3d & rhs ) const;
	
	Vec3d Cross( const Vec3d& rhs ) const;
	float Dot( const Vec3d& rhs ) const;
	float AngleBetween( Vec3d& rhs ) const;
	
	const Vec3d & Normalize();
	void RotateAboutZ( float angleRadians );
	float GetMagnitude() const;
	bool IsValid() const;
	void Zero() { x = 0; y = 0; z = 0; }
	
	const float * ToPtr() const { return &x; }
    
    friend Vec3d operator*( const int& lhs, const Vec3d& rhs );
    friend Vec3d operator*( const float& lhs, const Vec3d& rhs );

public:
	float x;
	float y;
	float z;
};

/*
 ================================
 Vec3d::Vec3d
 ================================
 */
inline Vec3d::Vec3d() :
x( 0 ),
y( 0 ),
z( 0 ) {
}

/*
 ================================
 Vec3d::Vec3d
 ================================
 */
inline Vec3d::Vec3d( int value ) :
x( value ),
y( value ),
z( value ) {
}

/*
 ================================
 Vec3d::Vec3d
 ================================
 */
inline Vec3d::Vec3d( float value ) :
x( value ),
y( value ),
z( value ) {
}

/*
 ================================
 Vec3d::Vec3d
 ================================
 */
inline Vec3d::Vec3d( const Vec3d &rhs ) :
x( rhs.x ),
y( rhs.y ),
z( rhs.z ) {
}

/*
 ================================
 Vec3d::Vec3d
 ================================
 */
inline Vec3d::Vec3d( float X, float Y, float Z ) :
x( X ),
y( Y ),
z( Z ) {
}

/*
 ================================
 Vec3d::Vec3d
 ================================
 */
inline Vec3d::Vec3d( const float * xyz ) :
x( xyz[ 0 ] ),
y( xyz[ 1 ] ),
z( xyz[ 2 ] ) {
}

/*
 ================================
 Vec3d::operator=
 ================================
 */
inline Vec3d& Vec3d::operator=( const Vec3d& rhs ) {
	x = rhs.x;
	y = rhs.y;
	z = rhs.z;
	return *this;
}

/*
 ================================
 Vec3d::~Vec3d
 ================================
 */
inline Vec3d::~Vec3d() {
}

/*
 ================================
 Vec3d::operator ==
 ================================
 */
inline bool Vec3d::operator == ( const Vec3d& rhs ) const {
	if ( x != rhs.x ) {
		return false;
	}
	
	if ( y != rhs.y ) {
		return false;
	}
	
	if ( z != rhs.z ) {
		return false;
	}
	
	return true;
}

/*
 ================================
 Vec3d::operator !=
 ================================
 */
inline bool Vec3d::operator != ( const Vec3d& rhs ) const {
	if ( *this == rhs ) {
		return false;
	}
	
	return true;
}

/*
 ================================
 Vec3d::operator+
 ================================
 */
inline Vec3d Vec3d::operator+( const Vec3d& rhs ) const {
	Vec3d temp;
	temp.x = x + rhs.x;
	temp.y = y + rhs.y;
	temp.z = z + rhs.z;
	return temp;
}

/*
 ================================
 Vec3d::operator+=
 ================================
 */
inline const Vec3d& Vec3d::operator+=( const Vec3d& rhs ) {
	x += rhs.x;
	y += rhs.y;
	z += rhs.z;
	return *this;
}

/*
 ================================
 Vec3d::operator-=
 ================================
 */
inline const Vec3d& Vec3d::operator-=( const Vec3d& rhs ) {
	x -= rhs.x;
	y -= rhs.y;
	z -= rhs.z;
	return *this;
}

/*
 ================================
 Vec3d::operator-
 ================================
 */
inline Vec3d Vec3d::operator-( const Vec3d& rhs ) const {
	Vec3d temp;
	temp.x = x - rhs.x;
	temp.y = y - rhs.y;
	temp.z = z - rhs.z;
	return temp;
}

inline Vec3d Vec3d::operator*(const Vec3d & rhs) const {
	Vec3d temp;
	temp.x = x * rhs.x;
	temp.y = y * rhs.y;
	temp.z = z * rhs.z;
	return temp;
}

inline const Vec3d& Vec3d::operator*=(const Vec3d& rhs) {
	x *= rhs.x;
	y *= rhs.y;
	z *= rhs.z;
	return *this;
}

/*
 ================================
 Vec3d::operator*
 ================================
 */
inline Vec3d Vec3d::operator*( const float& rhs ) const {
	Vec3d temp;
	temp.x = x * rhs;
	temp.y = y * rhs;
	temp.z = z * rhs;
	return temp;
}

/*
 ================================
 Vec3d::operator/
 ================================
 */
inline Vec3d Vec3d::operator/( const float& rhs ) const {
	Vec3d temp;
	temp.x = x / rhs;
	temp.y = y / rhs;
	temp.z = z / rhs;
	return temp;
}

/*
 ================================
 Vec3d::operator*=
 ================================
 */
inline const Vec3d& Vec3d::operator*=( const float& rhs ) {
	x *= rhs;
	y *= rhs;
	z *= rhs;
	return *this;
}

/*
 ================================
 Vec3d::operator/=
 ================================
 */
inline const Vec3d& Vec3d::operator/=( const float& rhs ) {
	x /= rhs;
	y /= rhs;
	z /= rhs;
	return *this;
}

/*
 ================================
 Vec3d::operator[]
 ================================
 */
inline float Vec3d::operator[]( const int& idx ) const {
	assert( idx >= 0 && idx < 3 );
	return ( &x )[ idx ];
}

/*
 ================================
 Vec3d::operator[]
 ================================
 */
inline float& Vec3d::operator[]( const int& idx ) {
	assert( idx >= 0 && idx < 3 );
	return ( &x )[ idx ];
}

/*
 ================================
 Vec3d::operator >
 ================================
 */
inline bool Vec3d::operator > ( const Vec3d & rhs ) const {
	return ( x > rhs.x && y > rhs.y && z > rhs.z );
}

/*
 ================================
 Vec3d::operator <
 ================================
 */
inline bool Vec3d::operator < ( const Vec3d & rhs ) const {
	return ( x < rhs.x && y < rhs.y && z < rhs.z );
}

/*
 ================================
 Vec3d::crossProduct
 * This cross product is A x B, where this is A and rhs is B
 ================================
 */
inline Vec3d Vec3d::Cross( const Vec3d &rhs ) const {
	Vec3d temp;
	temp.x = ( y * rhs.z ) - ( rhs.y * z );
	temp.y = ( rhs.x * z ) - ( x * rhs.z );
	temp.z = ( x * rhs.y ) - ( rhs.x * y );
	return temp;
}

/*
 ================================
 Vec3d::dotProduct
 ================================
 */
inline float Vec3d::Dot( const Vec3d& rhs ) const {
	float temp = ( x * rhs.x ) + ( y * rhs.y ) + ( z * rhs.z );
	return temp;
}

/*
 ================================
 Vec3d::angleBetween
 ================================
 */
inline float Vec3d::AngleBetween( Vec3d& rhs ) const {
	float dot = Dot( rhs );
	float angle = acos( dot / ( GetMagnitude() * rhs.GetMagnitude() ) );
	return angle;
}

/*
 ================================
 Vec3d::Normalize
 ================================
 */
inline const Vec3d & Vec3d::Normalize() {
	float mag = GetMagnitude();
	
	if ( mag > 0.0001f || mag < -0.0001f ) {
		// don't divide by zero
		x = x/mag;
		y = y/mag;
		z = z/mag;
	}
    
    return *this;
}

/*
 ================================
 Vec3d::Normalize
 ================================
 */
inline void Vec3d::RotateAboutZ( float angleRadians )  {
	float tmpX = x;
	float tmpY = y;
	x = tmpX * cos( angleRadians ) - tmpY * sin( angleRadians );
	y = tmpX * sin( angleRadians ) + tmpY * cos( angleRadians );
}

/*
 ================================
 Vec3d::Normalize
 ================================
 */
inline float Vec3d::GetMagnitude() const {
	float mag;
	
	mag = x*x + y*y + z*z;
	mag = sqrt(mag);
	
	return mag;
}

/*
 ================================
 Vec3d::IsValid
 ================================
 */
inline bool Vec3d::IsValid() const {
	if ( x != x ) {
		// x is NaN
		return false;
	}
	
	if ( y != y ) {
		// y is NaN
		return false;
	}
	
	if ( z != z ) {
		// z is NaN
		return false;
	}
	
	return true;
}
















/*
 ================================
 Vec4d
 ================================
 */
class Vec4d {
public:
	Vec4d();
	Vec4d( const float value );
	Vec4d( const Vec4d& rhs );     // copy ctor
	Vec4d( float X, float Y, float Z, float W );
	Vec4d( const float * rhs );
	Vec4d& operator=(const Vec4d& rhs );  // assignment operator
	
	~Vec4d();
	
	bool			operator == ( const Vec4d& rhs ) const;
	bool			operator != ( const Vec4d& rhs ) const;
	Vec4d			operator+( const Vec4d& rhs ) const;
	const Vec4d&	operator+=( const Vec4d& rhs );
	const Vec4d&	operator-=( const Vec4d& rhs );
    const Vec4d&	operator*=( const Vec4d& rhs );
	const Vec4d&	operator/=( const Vec4d& rhs );
	Vec4d			operator-( const Vec4d& rhs ) const;
	Vec4d			operator*( const float& rhs ) const;
	float			operator[]( const int& idx ) const;
	float&			operator[]( const int& idx );
	
	float Dot( const Vec4d & rhs ) const { return x * rhs.x + y * rhs.y + z * rhs.z + w * rhs.w; }
	const Vec4d & Normalize();
	float GetMagnitude() const;
	bool IsValid() const;
	float LengthSqr() const { return x * x + y * y + z * z + w * w; }
	void Zero() { x = 0; y = 0; z = 0; w = 0; }

    const float *   ToPtr() const   { return &x; }
	float *         ToPtr()         { return &x; }

	Vec3d xyz() const { return Vec3d( x, y, z ); }
    
    friend Vec4d operator*( const int& lhs, const Vec4d& rhs );
    friend Vec4d operator*( const float& lhs, const Vec4d& rhs );
	
public:
	float x;
	float y;
	float z;
	float w;
};

/*
 ================================
 Vec4d::Vec4d
 ================================
 */
inline Vec4d::Vec4d() :
x( 0 ),
y( 0 ),
z( 0 ),
w( 0 ) {
}

/*
 ================================
 Vec4d::Vec4d
 ================================
 */
inline Vec4d::Vec4d( const float value ) :
x( value ),
y( value ),
z( value ),
w( value ) {
}

/*
 ================================
 Vec4d::Vec4d
 ================================
 */
inline Vec4d::Vec4d( const Vec4d &rhs ) :
x( rhs.x ),
y( rhs.y ),
z( rhs.z ),
w( rhs.w ) {
}

/*
 ================================
 Vec4d::Vec4d
 ================================
 */
inline Vec4d::Vec4d( float X, float Y, float Z, float W ) :
x( X ),
y( Y ),
z( Z ),
w( W ) {
}

/*
 ================================
 Vec4d::Vec4d
 ================================
 */
inline Vec4d::Vec4d( const float * rhs ) {
	x = rhs[ 0 ];
	y = rhs[ 1 ];
	z = rhs[ 2 ];
	w = rhs[ 3 ];
}

/*
 ================================
 Vec4d::operator=
 ================================
 */
inline Vec4d& Vec4d::operator=( const Vec4d &rhs ) {
	x = rhs.x;
	y = rhs.y;
	z = rhs.z;
	w = rhs.w;
	return *this;
}

/*
 ================================
 Vec4d::~Vec4d
 ================================
 */
inline Vec4d::~Vec4d() {
}

/*
 ================================
 Vec4d::operator==
 ================================
 */
inline bool Vec4d::operator==( const Vec4d &rhs ) const {
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
 Vec4d::operator !=
 ================================
 */
inline bool Vec4d::operator != ( const Vec4d& rhs ) const {
	if ( *this == rhs ) {
		return false;
	}
	
	return true;
}

/*
 ================================
 Vec4d::operator+
 ================================
 */
inline Vec4d Vec4d::operator+( const Vec4d &rhs ) const {
	Vec4d temp;
	temp.x = x + rhs.x;
	temp.y = y + rhs.y;
	temp.z = z + rhs.z;
	temp.w = w + rhs.w;
	return temp;
}

/*
 ================================
 Vec4d::operator+=
 ================================
 */
inline const Vec4d& Vec4d::operator+=( const Vec4d &rhs ) {
	x += rhs.x;
	y += rhs.y;
	z += rhs.z;
	w += rhs.w;
	return *this;
}

/*
 ================================
 Vec4d::operator-=
 ================================
 */
inline const Vec4d& Vec4d::operator-=( const Vec4d &rhs ) {
	x -= rhs.x;
	y -= rhs.y;
	z -= rhs.z;
	w -= rhs.w;
	return *this;
}

/*
 ================================
 Vec4d::operator*=
 ================================
 */
inline const Vec4d& Vec4d::operator*=( const Vec4d &rhs ) {
	x *= rhs.x;
	y *= rhs.y;
	z *= rhs.z;
	w *= rhs.w;
	return *this;
}

/*
 ================================
 Vec4d::operator/=
 ================================
 */
inline const Vec4d& Vec4d::operator/=( const Vec4d &rhs ) {
	x /= rhs.x;
	y /= rhs.y;
	z /= rhs.z;
	w /= rhs.w;
	return *this;
}

/*
 ================================
 Vec4d::operator-
 ================================
 */
inline Vec4d Vec4d::operator-( const Vec4d &rhs ) const {
	Vec4d temp;
	temp.x = x - rhs.x;
	temp.y = y - rhs.y;
	temp.z = z - rhs.z;
	temp.w = w - rhs.w;
	return temp;
}

/*
 ================================
 Vec4d::operator*
 ================================
 */
inline Vec4d Vec4d::operator*( const float &rhs ) const {
	Vec4d temp;
	temp.x = x * rhs;
	temp.y = y * rhs;
	temp.z = z * rhs;
	temp.w = w * rhs;
	return temp;
}

/*
 ================================
 Vec4d::operator[]
 ================================
 */
inline float Vec4d::operator[]( const int& idx ) const {
	assert( idx >= 0 && idx < 4 );
	return ( &x )[ idx ];
}

/*
 ================================
 Vec4d::operator[]
 ================================
 */
inline float& Vec4d::operator[]( const int& idx ) {
	assert( idx >= 0 && idx < 4 );
	return ( &x )[ idx ];
}

/*
 ================================
 Vec4d::Normalize
 ================================
 */
inline const Vec4d & Vec4d::Normalize() {
	float mag = GetMagnitude();
	
	if ( mag > 0.0001f || mag < -0.0001f ) {
		//don't divide by zero
		x = x / mag;
		y = y / mag;
		z = z / mag;
		w = w / mag;
	}
    
    return *this;
}

/*
 ================================
 Vec4d::Normalize
 ================================
 */
inline float Vec4d::GetMagnitude() const {
	float mag;
	
	mag = x*x + y*y + z*z + w*w;
	mag = sqrt( mag );
	
	return mag;
}

/*
 ================================
 Vec4d::IsValid
 ================================
 */
inline bool Vec4d::IsValid() const {
	if ( x != x ) {
		// x is NaN
		return false;
	}
	
	if ( y != y ) {
		// y is NaN
		return false;
	}
	
	if ( z != z ) {
		// z is NaN
		return false;
	}
	
	if ( w != w ) {
		// w is NaN
		return false;
	}
	
	return true;
}



inline float dot( Vec2d a, Vec2d b ) {
    return a.Dot( b );
}

inline float dot( Vec3d a, Vec3d b ) {
    return a.Dot( b );
}
//
//	Matrix.h
//
#pragma once
#include "Math/Vector.h"
#include "Math/MatrixOps.h"
#include "Graphics/Graphics.h"


#ifndef MY_PI
#define MY_PI 3.14159265f
#endif

class Quat;

/*
 ================================
 Matrix
 * OpenGL uses column major matrices.
 * Therefore this matrix class will be column major.
 ================================
 */
class Matrix {
public:
	Matrix();
	Matrix( const Matrix & rhs );
    Matrix( const float * mat );
	Matrix( const Vec3d & col0, const Vec3d & col1, const Vec3d & col2 );
	Matrix( const Vec4d & col0, const Vec4d & col1, const Vec4d & col2, const Vec4d & col3 );
	Matrix & operator = ( const Matrix & rhs );
	~Matrix() {}
    
    static Matrix RotationMatrix( const float x, const float y, const float z, const float angleDegrees );
    static Matrix RotationMatrix( const Vec3d & axis, const float angleDegrees );
	
	void SetIdentity();
	void SetTranslationMatrix( const Vec2d & translation );
    void SetTranslationMatrix( const Vec3d & translation );
	void SetTranslationMatrix( const Vec4d & translation );
	void SetRotationMatrix( const float x, const float y, const float z, const float angle_degrees );
    void SetRotationMatrix( const Vec3d & axis, const float angle_degrees );
	void SetScalingMatrix( float scale );
	void SetScalingMatrix( const Vec4d & scale );
    
	float Trace() const;
    float Determinant() const;
	const Matrix &	Transpose();
    const Matrix &	Invert();
	//Matrix			Inverse() const;
	
	Matrix    operator * ( const Matrix & rhs ) const;
	Vec2d     operator * ( const Vec2d & rhs ) const;
    Vec3d     operator * ( const Vec3d & rhs ) const;
    Vec4d     operator * ( const Vec4d & rhs ) const;
	
	Vec4d		operator[]( const int idx ) const	{ assert( idx >= 0 && idx < 4 ); return *( ( &column0 ) + idx ); }
	Vec4d &	operator[]( const int idx )			{ assert( idx >= 0 && idx < 4 ); return *( ( &column0 ) + idx ); }
    
    float * ToPtr();
	const float * ToPtr() const;
    void ToFloatArray( float * out ) const;
	
public:
	Vec4d	column0;
	Vec4d	column1;
	Vec4d	column2;
	Vec4d	column3;
};

/*
 ================================
 Matrix::Matrix
 ================================
 */
inline Matrix::Matrix() {
	SetIdentity();
}

/*
 ================================
 Matrix::Matrix
 ================================
 */
inline Matrix::Matrix( const Matrix& rhs ) :
column0( rhs.column0 ),
column1( rhs.column1 ),
column2( rhs.column2 ),
column3( rhs.column3 ) {
}

/*
 ================================
 Matrix::Matrix
 ================================
 */
inline Matrix::Matrix( const float * mat ) {
    column0[0] = mat[0];
    column0[1] = mat[1];
    column0[2] = mat[2];
    column0[3] = mat[3];
    
    column1[0] = mat[4];
    column1[1] = mat[5];
    column1[2] = mat[6];
    column1[3] = mat[7];
    
    column2[0] = mat[8];
    column2[1] = mat[9];
    column2[2] = mat[10];
    column2[3] = mat[11];
    
    column3[0] = mat[12];
    column3[1] = mat[13];
    column3[2] = mat[14];
    column3[3] = mat[15];
}

/*
 ================================
 Matrix::Matrix

 This is meant for creating rotation matrices
 ================================
 */
inline Matrix::Matrix( const Vec3d & col0, const Vec3d & col1, const Vec3d & col2 ) {
	column0.x = col0.x;
	column0.y = col0.y;
	column0.z = col0.z;
	column0.w = 0.0f;

	column1.x = col1.x;
	column1.y = col1.y;
	column1.z = col1.z;
	column1.w = 0.0f;

	column2.x = col2.x;
	column2.y = col2.y;
	column2.z = col2.z;
	column2.w = 0.0f;

	column3 = Vec4d( 0.0f );
}

/*
 ================================
 Matrix::Matrix
 ================================
 */
inline Matrix::Matrix( const Vec4d & col0, const Vec4d & col1, const Vec4d & col2, const Vec4d & col3 ) :
column0( col0 ),
column1( col1 ),
column2( col2 ),
column3( col3 ) {
}

/*
 ================================
 Matrix::operator=
 ================================
 */
inline Matrix& Matrix::operator=( const Matrix& rhs ) {
	column0 = rhs.column0;
	column1 = rhs.column1;
	column2 = rhs.column2;
	column3 = rhs.column3;
	return *this;
}

/*
 ================================
 Matrix::SetIdentity
 ================================
 */
inline void Matrix::SetIdentity() {
	column0 = Vec4d( 1, 0, 0, 0 );
	column1 = Vec4d( 0, 1, 0, 0 );
	column2 = Vec4d( 0, 0, 1, 0 );
	column3 = Vec4d( 0, 0, 0, 1 );
}

/*
 ================================
 Matrix::SetTranslationMatrix
 * this is clearly setup for opengl matrices
 ================================
 */
inline void Matrix::SetTranslationMatrix( const Vec2d & translation ) {
	SetIdentity();
#if 0
	column0[ 3 ] = translation.x;
	column1[ 3 ] = translation.y;
	column2[ 3 ] = 0.0f;
	column3[ 3 ] = 1.0;
#else
    column3[ 0 ] = translation.x;
	column3[ 1 ] = translation.y;
	column3[ 2 ] = 0.0f;
	column3[ 3 ] = 1.0;
#endif
}

/*
 ================================
 Matrix::SetTranslationMatrix
 * this is clearly setup for opengl matrices
 ================================
 */
inline void Matrix::SetTranslationMatrix( const Vec3d & translation ) {
	SetIdentity();
#if 0
	column0[ 3 ] = translation.x;
	column1[ 3 ] = translation.y;
	column2[ 3 ] = translation.z;
	column3[ 3 ] = 1.0;
#else
    column3[ 0 ] = translation.x;
	column3[ 1 ] = translation.y;
	column3[ 2 ] = translation.z;
	column3[ 3 ] = 1.0;
#endif
}

/*
 ================================
 Matrix::SetTranslationMatrix
 ================================
 */
inline void Matrix::SetTranslationMatrix( const Vec4d & translation ) {
	SetIdentity();
	column0[ 3 ] = translation.x;
	column1[ 3 ] = translation.y;
	column2[ 3 ] = translation.z;
	column3[ 3 ] = translation.w;
}

/*
 ================================
 Matrix::SetRotationMatrix
 ================================
 */
inline void Matrix::SetRotationMatrix( const float x, const float y, const float z, const float angle_degrees ) {
    float cosA = cosf( angle_degrees * MY_PI / 180.0f );
	float sinA = sinf( angle_degrees * MY_PI / 180.0f );
	
	column0[ 0 ] =	cosA					+ x * x * ( 1.0f - cosA );
	column0[ 1 ] =	y * x * ( 1.0f - cosA ) + z * sinA;
	column0[ 2 ] =	z * x * ( 1.0f - cosA ) - y * sinA;
	column0[ 3 ] =	0;
	
	column1[ 0 ] =	x * y * ( 1.0f - cosA ) - z * sinA;
	column1[ 1 ] =	cosA					+ y * y * ( 1.0f - cosA );
	column1[ 2 ] =	z * y * ( 1.0f - cosA ) + x * sinA;
	column1[ 3 ] =	0;
	
	column2[ 0 ] =	x * z * ( 1.0f - cosA ) + y * sinA;
	column2[ 1 ] =	y * z * ( 1.0f - cosA ) - x * sinA;
	column2[ 2 ] =	cosA					+ z * z * ( 1.0f - cosA );
	column2[ 3 ] =	0;
	
	column3[ 0 ] = 0;
	column3[ 1 ] = 0;
	column3[ 2 ] = 0;
	column3[ 3 ] = 1;
}

/*
 ================================
 Matrix::SetRotationMatrix
 ================================
 */
inline void Matrix::SetRotationMatrix( const Vec3d& axis, const float angle_degrees ) {
    SetRotationMatrix( axis.x, axis.y, axis.z, angle_degrees );
}

/*
 ================================
 Matrix::SetScalingMatrix
 ================================
 */
inline void Matrix::SetScalingMatrix( float scale ) {
	column0 = Vec4d( scale, 0, 0, 0 );
	column1 = Vec4d( 0, scale, 0, 0 );
	column2 = Vec4d( 0, 0, scale, 0 );
	column3 = Vec4d( 0, 0, 0, scale );
}

/*
 ================================
 Matrix::SetTranslationMatrix
 ================================
 */
inline void Matrix::SetScalingMatrix( const Vec4d& scale ) {
	column0 = Vec4d( scale.x, 0, 0, 0 );
	column1 = Vec4d( 0, scale.y, 0, 0 );
	column2 = Vec4d( 0, 0, scale.z, 0 );
	column3 = Vec4d( 0, 0, 0, scale.w );
}

/*
 ================================
 Matrix::Trace
 ================================
 */
inline float Matrix::Trace() const {
	return ( column0[ 0 ] + column1[ 1 ] + column2[ 2 ] + column3[ 3 ] );
}

/*
 ================================
 Matrix::Determinant
 ================================
 */
//inline float Matrix::Determinant() const {
//    float d;
//}

/*
 ================================
 Matrix::Transpose
 ================================
 */
inline const Matrix & Matrix::Transpose() {
	Swap( column0[ 1 ], column1[ 0 ] );
	Swap( column0[ 2 ], column2[ 0 ] );
	Swap( column0[ 3 ], column3[ 0 ] );
	
	Swap( column1[ 2 ], column2[ 1 ] );
	Swap( column1[ 3 ], column3[ 1 ] );
	
	Swap( column2[ 3 ], column3[ 2 ] );
	return *this;
}

/*
 ================================
 Matrix::Invert
 ================================
 */
//inline Matrix& Matrix::Invert() {
//    return *this;
//}

/*
 ================================
 Matrix::operator *
 ================================
 */
inline Matrix Matrix::operator*( const Matrix& rhs ) const {
    Matrix mat;
    
    Matrix tmpA = *this;
    Matrix tmpB = rhs;
    myMatrixMultiply( tmpA.ToPtr(), tmpB.ToPtr(), mat.ToPtr() );
//    hbglMatrixMultiply( tmpB.ToPtr(), tmpA.ToPtr(), mat.ToPtr() );
    return mat;
}

/*
================================
Matrix::operator *
================================
*/
inline Vec2d Matrix::operator*( const Vec2d & rhs ) const {
    Vec2d v;
#if 0
    v.x = rhs.x * column0.x + rhs.y * column0.y + column0.z;
    v.y = rhs.x * column1.x + rhs.y * column1.y + column1.z;
#else
    // the addition of column 3 is in the event we have a translational component
    v.x = rhs.x * column0.x + rhs.y * column1.x + column3.x;
    v.y = rhs.x * column0.y + rhs.y * column1.y + column3.y;
#endif
    return v;
}

/*
 ================================
 Matrix::operator *
 ================================
 */
inline Vec3d Matrix::operator*( const Vec3d & rhs ) const {
    Vec3d v;
#if 0
    v.x = rhs.x * column0.x + rhs.y * column0.y + rhs.z * column0.z + column0.w;
    v.y = rhs.x * column1.x + rhs.y * column1.y + rhs.z * column1.z + column0.w;
    v.z = rhs.x * column2.x + rhs.y * column2.y + rhs.z * column2.z + column0.w;
#else
    // the addition of column 3 is in the event we have a translational component
    v.x = rhs.x * column0.x + rhs.y * column1.x + rhs.z * column2.x + column3.x;
    v.y = rhs.x * column0.y + rhs.y * column1.y + rhs.z * column2.y + column3.y;
    v.z = rhs.x * column0.z + rhs.y * column1.z + rhs.z * column2.z + column3.z;
#endif
    return v;
}

/*
 ================================
 Matrix::operator *
 ================================
 */
inline Vec4d Matrix::operator*( const Vec4d & rhs ) const {
    Vec4d v;
#if 1
    v.x = rhs.x * column0.x + rhs.y * column0.y + rhs.z * column0.z + rhs.w * column0.w;
    v.y = rhs.x * column1.x + rhs.y * column1.y + rhs.z * column1.z + rhs.w * column1.w;
    v.z = rhs.x * column2.x + rhs.y * column2.y + rhs.z * column2.z + rhs.w * column2.w;
    v.w = rhs.x * column3.x + rhs.y * column3.y + rhs.z * column3.z + rhs.w * column3.w;
#else
    v.x = rhs.x * column0.x + rhs.y * column1.y + rhs.z * column2.z;
    v.y = rhs.x * column0.x + rhs.y * column1.y + rhs.z * column2.z;
    v.z = rhs.x * column0.x + rhs.y * column1.y + rhs.z * column2.z;
#endif
    return v;
}

/*
 ================================
 Matrix::ToPtr
 ================================
 */
inline float * Matrix::ToPtr() {
    return column0.ToPtr();
}

/*
 ================================
 Matrix::ToPtr
 ================================
 */
inline const float * Matrix::ToPtr() const {
    return column0.ToPtr();
}

/*
 ================================
 Matrix::ToFloatArray
 ================================
 */
inline void Matrix::ToFloatArray( float * out ) const {
    out[ 0 ] = column0[ 0 ];
    out[ 1 ] = column0[ 1 ];
    out[ 2 ] = column0[ 2 ];
    out[ 3 ] = column0[ 3 ];
    
    out[ 4 ] = column1[ 0 ];
    out[ 5 ] = column1[ 1 ];
    out[ 6 ] = column1[ 2 ];
    out[ 7 ] = column1[ 3 ];
    
    out[ 8 ] = column2[ 0 ];
    out[ 9 ] = column2[ 1 ];
    out[ 10 ] = column2[ 2 ];
    out[ 11 ] = column2[ 3 ];
    
    out[ 12 ] = column3[ 0 ];
    out[ 13 ] = column3[ 1 ];
    out[ 14 ] = column3[ 2 ];
    out[ 15 ] = column3[ 3 ];
}









/*
====================================================
Mat2
====================================================
*/
class Mat2 {
public:
	Mat2() {}
	Mat2( const Mat2 & rhs );
	Mat2( const float * mat );
	Mat2( const Vec2d & row0, const Vec2d & row1 );
	Mat2 & operator = ( const Mat2 & rhs );

	const Mat2 & operator *= ( const float rhs );
	const Mat2 & operator += ( const Mat2 & rhs );

	float Determinant() const { return rows[ 0 ].x * rows[ 1 ].y - rows[ 0 ].y * rows[ 1 ].x; }

public:
	Vec2d rows[ 2 ];
};

inline Mat2::Mat2( const Mat2 & rhs ) {
	rows[ 0 ] = rhs.rows[ 0 ];
	rows[ 1 ] = rhs.rows[ 1 ];
}

inline Mat2::Mat2( const float * mat ) {
	rows[ 0 ] = mat + 0;
	rows[ 1 ] = mat + 2;
}

inline Mat2::Mat2( const Vec2d & row0, const Vec2d & row1 ) {
	rows[ 0 ] = row0;
	rows[ 1 ] = row1;
}

inline Mat2 & Mat2::operator = ( const Mat2 & rhs ) {
	rows[ 0 ] = rhs.rows[ 0 ];
	rows[ 1 ] = rhs.rows[ 1 ];
	return *this;
}

inline const Mat2 & Mat2::operator *= ( const float rhs ) {
	rows[ 0 ] *= rhs;
	rows[ 1 ] *= rhs;
	return *this;
}

inline const Mat2 & Mat2::operator += ( const Mat2 & rhs ) {
	rows[ 0 ] += rhs.rows[ 0 ];
	rows[ 1 ] += rhs.rows[ 1 ];
	return *this;
}

/*
====================================================
Mat3
====================================================
*/
class Mat3 {
public:
	Mat3() {}
	Mat3( const Mat3 & rhs );
	Mat3( const float * mat );
	Mat3( const Vec3d & row0, const Vec3d & row1, const Vec3d & row2 );
	Mat3 & operator = ( const Mat3 & rhs );

	void Zero();
	void Identity();

	float Trace() const;
	float Determinant() const;
	Mat3 Transpose() const;
	Mat3 Inverse() const;
	Mat2 Minor( const int i, const int j ) const;
	float Cofactor( const int i, const int j ) const;

	Vec3d operator * ( const Vec3d & rhs ) const;
	Mat3 operator * ( const float rhs ) const;
	Mat3 operator * ( const Mat3 & rhs ) const;
	Mat3 operator + ( const Mat3 & rhs ) const;
	const Mat3 & operator *= ( const float rhs );
	const Mat3 & operator += ( const Mat3 & rhs );

public:
	Vec3d rows[ 3 ];
};

inline Mat3::Mat3( const Mat3 & rhs ) {
	rows[ 0 ] = rhs.rows[ 0 ];
	rows[ 1 ] = rhs.rows[ 1 ];
	rows[ 2 ] = rhs.rows[ 2 ];
}

inline Mat3::Mat3( const float * mat ) {
	rows[ 0 ] = mat + 0;
	rows[ 1 ] = mat + 3;
	rows[ 2 ] = mat + 6;
}

inline Mat3::Mat3( const Vec3d & row0, const Vec3d & row1, const Vec3d & row2 ) {
	rows[ 0 ] = row0;
	rows[ 1 ] = row1;
	rows[ 2 ] = row2;
}

inline Mat3 & Mat3::operator = ( const Mat3 & rhs ) {
	rows[ 0 ] = rhs.rows[ 0 ];
	rows[ 1 ] = rhs.rows[ 1 ];
	rows[ 2 ] = rhs.rows[ 2 ];
	return *this;
}

inline const Mat3 & Mat3::operator *= ( const float rhs ) {
	rows[ 0 ] *= rhs;
	rows[ 1 ] *= rhs;
	rows[ 2 ] *= rhs;
	return *this;
}

inline const Mat3 & Mat3::operator += ( const Mat3 & rhs ) {
	rows[ 0 ] += rhs.rows[ 0 ];
	rows[ 1 ] += rhs.rows[ 1 ];
	rows[ 2 ] += rhs.rows[ 2 ];
	return *this;
}

inline void Mat3::Zero() {
	rows[ 0 ].Zero();
	rows[ 1 ].Zero();
	rows[ 2 ].Zero();
}

inline void Mat3::Identity() {
	rows[ 0 ] = Vec3d( 1, 0, 0 );
	rows[ 1 ] = Vec3d( 0, 1, 0 );
	rows[ 2 ] = Vec3d( 0, 0, 1 );
}

inline float Mat3::Trace() const {
	const float xx = rows[ 0 ][ 0 ] * rows[ 0 ][ 0 ];
	const float yy = rows[ 1 ][ 1 ] * rows[ 1 ][ 1 ];
	const float zz = rows[ 2 ][ 2 ] * rows[ 2 ][ 2 ];
	return ( xx + yy + zz );
}

inline float Mat3::Determinant() const {
	const float i = rows[ 0 ][ 0 ] * ( rows[ 1 ][ 1 ] * rows[ 2 ][ 2 ] - rows[ 1 ][ 2 ] * rows[ 2 ][ 1 ] );
	const float j = rows[ 0 ][ 1 ] * ( rows[ 1 ][ 0 ] * rows[ 2 ][ 2 ] - rows[ 1 ][ 2 ] * rows[ 2 ][ 0 ] );
	const float k = rows[ 0 ][ 2 ] * ( rows[ 1 ][ 0 ] * rows[ 2 ][ 1 ] - rows[ 1 ][ 1 ] * rows[ 2 ][ 0 ] );
	return ( i - j + k );
}

inline Mat3 Mat3::Transpose() const {
	Mat3 transpose;
	for ( int i = 0; i < 3; i++ ) {
		for ( int j = 0; j < 3; j++ ) {
			transpose.rows[ i ][ j ] = rows[ j ][ i ];
		}
	}
	return transpose;
}

inline Mat3 Mat3::Inverse() const {
	Mat3 inv;
	for ( int i = 0; i < 3; i++ ) {
		for ( int j = 0; j < 3; j++ ) {
			inv.rows[ j ][ i ] = Cofactor( i, j );	// Perform the transpose while calculating the cofactors
		}
	}
	float det = Determinant();
	float invDet = 1.0f / det;
	inv *= invDet;
	return inv;
}

inline Mat2 Mat3::Minor( const int i, const int j ) const {
	Mat2 minor;

	int yy = 0;
	for ( int y = 0; y < 3; y++ ) {
		if ( y == j ) {
			continue;
		}

		int xx = 0;
		for ( int x = 0; x < 3; x++ ) {
			if ( x == i ) {
				continue;
			}

			minor.rows[ xx ][ yy ] = rows[ x ][ y ];
			xx++;
		}

		yy++;
	}
	return minor;
}

inline float Mat3::Cofactor( const int i, const int j ) const {
	const Mat2 minor = Minor( i, j );
	const float C = float( pow( -1, i + 1 + j + 1 ) ) * minor.Determinant();
	return C;
}

inline Vec3d Mat3::operator * ( const Vec3d & rhs ) const {
	Vec3d tmp;
	tmp[ 0 ] = rows[ 0 ].Dot( rhs );
	tmp[ 1 ] = rows[ 1 ].Dot( rhs );
	tmp[ 2 ] = rows[ 2 ].Dot( rhs );
	return tmp;
}

inline Mat3 Mat3::operator * ( const float rhs ) const {
	Mat3 tmp;
	tmp.rows[ 0 ] = rows[ 0 ] * rhs;
	tmp.rows[ 1 ] = rows[ 1 ] * rhs;
	tmp.rows[ 2 ] = rows[ 2 ] * rhs;
	return tmp;
}

inline Mat3 Mat3::operator * ( const Mat3 & rhs ) const {
	Mat3 tmp;
	for ( int i = 0; i < 3; i++ ) {
		tmp.rows[ i ].x = rows[ i ].x * rhs.rows[ 0 ].x + rows[ i ].y * rhs.rows[ 1 ].x + rows[ i ].z * rhs.rows[ 2 ].x;
		tmp.rows[ i ].y = rows[ i ].x * rhs.rows[ 0 ].y + rows[ i ].y * rhs.rows[ 1 ].y + rows[ i ].z * rhs.rows[ 2 ].y;
		tmp.rows[ i ].z = rows[ i ].x * rhs.rows[ 0 ].z + rows[ i ].y * rhs.rows[ 1 ].z + rows[ i ].z * rhs.rows[ 2 ].z;
	}
	return tmp;
}

inline Mat3 Mat3::operator + ( const Mat3 & rhs ) const {
	Mat3 tmp;
	for ( int i = 0; i < 3; i++ ) {
		tmp.rows[ i ] = rows[ i ] + rhs.rows[ i ];
	}
	return tmp;
}

/*
====================================================
Mat4
====================================================
*/
class Mat4 {
public:
	Mat4() {}
	Mat4( const Mat4 & rhs );
	Mat4( const float * mat );
	Mat4( const Vec4d & row0, const Vec4d & row1, const Vec4d & row2, const Vec4d & row3 );
	Mat4 & operator = ( const Mat4 & rhs );
	~Mat4() {}

	void Zero();
	void Identity();

	float Trace() const;
	float Determinant() const;
	Mat4 Transpose() const;
	Mat4 Inverse() const;
	Mat3 Minor( const int i, const int j ) const;
	float Cofactor( const int i, const int j ) const;

	void Orient( Vec3d pos, Vec3d fwd, Vec3d up );
	void LookAt( Vec3d pos, Vec3d lookAt, Vec3d up );
	void PerspectiveOpenGL( float fovy, float aspect_ratio, float near, float far );
	void PerspectiveVulkan( float fovy, float aspect_ratio, float near, float far );
	void OrthoOpenGL( float xmin, float xmax, float ymin, float ymax, float znear, float zfar );
	void OrthoVulkan( float xmin, float xmax, float ymin, float ymax, float znear, float zfar );

	const float * ToPtr() const { return rows[ 0 ].ToPtr(); }
	float * ToPtr() { return rows[ 0 ].ToPtr(); }

	Vec4d operator * ( const Vec4d & rhs ) const;
	Mat4 operator * ( const float rhs ) const;
	Mat4 operator * ( const Mat4 & rhs ) const;
	const Mat4 & operator *= ( const float rhs );

public:
	Vec4d rows[ 4 ];
};

inline Mat4::Mat4( const Mat4 & rhs ) {
	rows[ 0 ] = rhs.rows[ 0 ];
	rows[ 1 ] = rhs.rows[ 1 ];
	rows[ 2 ] = rhs.rows[ 2 ];
	rows[ 3 ] = rhs.rows[ 3 ];
}

inline Mat4::Mat4( const float * mat ) {
	rows[ 0 ] = mat + 0;
	rows[ 1 ] = mat + 4;
	rows[ 2 ] = mat + 8;
	rows[ 3 ] = mat + 12;
}

inline Mat4::Mat4( const Vec4d & row0, const Vec4d & row1, const Vec4d & row2, const Vec4d & row3 ) {
	rows[ 0 ] = row0;
	rows[ 1 ] = row1;
	rows[ 2 ] = row2;
	rows[ 3 ] = row3;
}

inline Mat4 & Mat4::operator = ( const Mat4 & rhs ) {
	rows[ 0 ] = rhs.rows[ 0 ];
	rows[ 1 ] = rhs.rows[ 1 ];
	rows[ 2 ] = rhs.rows[ 2 ];
	rows[ 3 ] = rhs.rows[ 3 ];
	return *this;
}

inline const Mat4 & Mat4::operator *= ( const float rhs ) {
	rows[ 0 ] *= rhs;
	rows[ 1 ] *= rhs;
	rows[ 2 ] *= rhs;
	rows[ 3 ] *= rhs;
	return *this;
}

inline void Mat4::Zero() {
	rows[ 0 ].Zero();
	rows[ 1 ].Zero();
	rows[ 2 ].Zero();
	rows[ 3 ].Zero();
}

inline void Mat4::Identity() {
	rows[ 0 ] = Vec4d( 1, 0, 0, 0 );
	rows[ 1 ] = Vec4d( 0, 1, 0, 0 );
	rows[ 2 ] = Vec4d( 0, 0, 1, 0 );
	rows[ 3 ] = Vec4d( 0, 0, 0, 1 );
}

inline float Mat4::Trace() const {
	const float xx = rows[ 0 ][ 0 ] * rows[ 0 ][ 0 ];
	const float yy = rows[ 1 ][ 1 ] * rows[ 1 ][ 1 ];
	const float zz = rows[ 2 ][ 2 ] * rows[ 2 ][ 2 ];
	const float ww = rows[ 3 ][ 3 ] * rows[ 3 ][ 3 ];
	return ( xx + yy + zz + ww );
}

inline float Mat4::Determinant() const {
	float det = 0.0f;
	float sign = 1.0f;
	for ( int j = 0; j < 4; j++ ) {
		Mat3 minor = Minor( 0, j );

		det += rows[ 0 ][ j ] * minor.Determinant() * sign;
		sign = sign * -1.0f;
	}
	return det;
}

inline Mat4 Mat4::Transpose() const {
	Mat4 transpose;
	for ( int i = 0; i < 4; i++ ) {
		for ( int j = 0; j < 4; j++ ) {
			transpose.rows[ i ][ j ] = rows[ j ][ i ];
		}
	}
	return transpose;
}

inline Mat4 Mat4::Inverse() const {
	Mat4 inv;
	for ( int i = 0; i < 4; i++ ) {
		for ( int j = 0; j < 4; j++ ) {
			inv.rows[ j ][ i ] = Cofactor( i, j );	// Perform the transpose while calculating the cofactors
		}
	}
	float det = Determinant();
	float invDet = 1.0f / det;
	inv *= invDet;
	return inv;
}

inline Mat3 Mat4::Minor( const int i, const int j ) const {
	Mat3 minor;

	int yy = 0;
	for ( int y = 0; y < 4; y++ ) {
		if ( y == j ) {
			continue;
		}

		int xx = 0;
		for ( int x = 0; x < 4; x++ ) {
			if ( x == i ) {
				continue;
			}

			minor.rows[ xx ][ yy ] = rows[ x ][ y ];
			xx++;
		}

		yy++;
	}
	return minor;
}

inline float Mat4::Cofactor( const int i, const int j ) const {
	const Mat3 minor = Minor( i, j );
	const float C = float( pow( -1, i + 1 + j + 1 ) ) * minor.Determinant();
	return C;
}

inline void Mat4::Orient( Vec3d pos, Vec3d fwd, Vec3d up ) {
	Vec3d left = up.Cross( fwd );

	// For our coordinate system where:
	// +x-axis = fwd
	// +y-axis = left
	// +z-axis = up
	rows[ 0 ] = Vec4d( fwd.x, left.x, up.x, pos.x );
	rows[ 1 ] = Vec4d( fwd.y, left.y, up.y, pos.y );
	rows[ 2 ] = Vec4d( fwd.z, left.z, up.z, pos.z );
	rows[ 3 ] = Vec4d( 0, 0, 0, 1 );
}

inline void Mat4::LookAt( Vec3d pos, Vec3d lookAt, Vec3d up ) {
	Vec3d fwd = pos - lookAt;
	fwd.Normalize();

	Vec3d right = up.Cross( fwd );
	right.Normalize();

	up = fwd.Cross( right );
	up.Normalize();

	// For NDC coordinate system where:
	// +x-axis = right
	// +y-axis = up
	// +z-axis = fwd
	rows[ 0 ] = Vec4d( right.x, right.y, right.z, -pos.Dot( right ) );
	rows[ 1 ] = Vec4d( up.x, up.y, up.z, -pos.Dot( up ) );
	rows[ 2 ] = Vec4d( fwd.x, fwd.y, fwd.z, -pos.Dot( fwd ) );
	rows[ 3 ] = Vec4d( 0, 0, 0, 1 );
}

inline void Mat4::PerspectiveOpenGL( float fovy, float aspect_ratio, float nearf, float farf ) {
	const float pi = acosf( -1.0f );
	const float fovy_radians = fovy * pi / 180.0f;
	const float f = 1.0f / tanf( fovy_radians * 0.5f );
	const float xscale = f;
	const float yscale = f / aspect_ratio;
	const float zscale = ( farf + nearf ) / ( nearf - farf );
	const float w = ( 2.0f * farf * nearf ) / ( nearf - farf );

	rows[ 0 ] = Vec4d( xscale, 0, 0, 0 );
	rows[ 1 ] = Vec4d( 0, yscale, 0, 0 );
	rows[ 2 ] = Vec4d( 0, 0, zscale, w );
	rows[ 3 ] = Vec4d( 0, 0, -1, 0 );
}

inline void Mat4::PerspectiveVulkan( float fovy, float aspect_ratio, float nearf, float farf ) {
	// Vulkan changed its NDC.  It switch from a left handed coordinate system to a right handed one.
	// +x points to the right, +z points into the screen, +y points down (it used to point up, in opengl).
	// It also changed the range from [-1,1] to [0,1] for the z.
	// Clip space remains [-1,1] for x and y.
	// Check section 23 of the specification.
	Mat4 matVulkan;
	matVulkan.rows[ 0 ] = Vec4d( 1, 0, 0, 0 );
	matVulkan.rows[ 1 ] = Vec4d( 0, -1, 0, 0 );
	matVulkan.rows[ 2 ] = Vec4d( 0, 0, 0.5f, 0.5f );
	matVulkan.rows[ 3 ] = Vec4d( 0, 0, 0, 1 );

	Mat4 matOpenGL;
	matOpenGL.PerspectiveOpenGL( fovy, aspect_ratio, nearf, farf );

	*this = matVulkan * matOpenGL;
}

inline void Mat4::OrthoOpenGL( float xmin, float xmax, float ymin, float ymax, float znear, float zfar ) {
	const float width	= xmax - xmin;
	const float height	= ymax - ymin;
	const float depth	= zfar - znear;

	const float tx = -( xmax + xmin ) / width;
	const float ty = -( ymax + ymin ) / height;
	const float tz = -( zfar + znear ) / depth;

	rows[ 0 ] = Vec4d( 2.0f / width, 0, 0, tx );
	rows[ 1 ] = Vec4d( 0, 2.0f / height, 0, ty );
	rows[ 2 ] = Vec4d( 0, 0, -2.0f / depth, tz );
	rows[ 3 ] = Vec4d( 0, 0, 0, 1 );
}

inline void Mat4::OrthoVulkan( float xmin, float xmax, float ymin, float ymax, float znear, float zfar ) {
	// Vulkan changed its NDC.  It switch from a left handed coordinate system to a right handed one.
	// +x points to the right, +z points into the screen, +y points down (it used to point up, in opengl).
	// It also changed the range from [-1,1] to [0,1] for the z.
	// Clip space remains [-1,1] for x and y.
	// Check section 23 of the specification.
	Mat4 matVulkan;
	matVulkan.rows[ 0 ] = Vec4d( 1, 0, 0, 0 );
	matVulkan.rows[ 1 ] = Vec4d( 0, -1, 0, 0 );
	matVulkan.rows[ 2 ] = Vec4d( 0, 0, 0.5f, 0.5f );
	matVulkan.rows[ 3 ] = Vec4d( 0, 0, 0, 1 );

	Mat4 matOpenGL;
	matOpenGL.OrthoOpenGL( xmin, xmax, ymin, ymax, znear, zfar );

	*this = matVulkan * matOpenGL;
}

inline Vec4d Mat4::operator * ( const Vec4d & rhs ) const {
	Vec4d tmp;
	tmp[ 0 ] = rows[ 0 ].Dot( rhs );
	tmp[ 1 ] = rows[ 1 ].Dot( rhs );
	tmp[ 2 ] = rows[ 2 ].Dot( rhs );
	tmp[ 3 ] = rows[ 3 ].Dot( rhs );
	return tmp;
}

inline Mat4 Mat4::operator * ( const float rhs ) const {
	Mat4 tmp;
	tmp.rows[ 0 ] = rows[ 0 ] * rhs;
	tmp.rows[ 1 ] = rows[ 1 ] * rhs;
	tmp.rows[ 2 ] = rows[ 2 ] * rhs;
	tmp.rows[ 3 ] = rows[ 3 ] * rhs;
	return tmp;
}

inline Mat4 Mat4::operator * ( const Mat4 & rhs ) const {
	Mat4 tmp;
	for ( int i = 0; i < 4; i++ ) {
		tmp.rows[ i ].x = rows[ i ].x * rhs.rows[ 0 ].x + rows[ i ].y * rhs.rows[ 1 ].x + rows[ i ].z * rhs.rows[ 2 ].x + rows[ i ].w * rhs.rows[ 3 ].x;
		tmp.rows[ i ].y = rows[ i ].x * rhs.rows[ 0 ].y + rows[ i ].y * rhs.rows[ 1 ].y + rows[ i ].z * rhs.rows[ 2 ].y + rows[ i ].w * rhs.rows[ 3 ].y;
		tmp.rows[ i ].z = rows[ i ].x * rhs.rows[ 0 ].z + rows[ i ].y * rhs.rows[ 1 ].z + rows[ i ].z * rhs.rows[ 2 ].z + rows[ i ].w * rhs.rows[ 3 ].z;
		tmp.rows[ i ].w = rows[ i ].x * rhs.rows[ 0 ].w + rows[ i ].y * rhs.rows[ 1 ].w + rows[ i ].z * rhs.rows[ 2 ].w + rows[ i ].w * rhs.rows[ 3 ].w;
	}
	return tmp;
}

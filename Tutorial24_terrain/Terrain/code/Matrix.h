/*
 *  Matrix.h
 *
 */
#pragma once
#include "Vector.h"
#include "MatrixOps.h"
#include "Graphics.h"


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

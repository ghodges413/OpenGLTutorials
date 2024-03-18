//
//  MatrixOps.h
//
#pragma once
#include "Math/Vector.h"

void myMatrix16Copy( const float * in, float * out );

void myIdentityMatrix( float * out );
void myRotateX( float angleDegrees, float * out );
void myRotateY( float angleDegrees, float * out );
void myRotateZ( float angleDegrees, float * out );
void myRotate( float angleDegrees, float x, float y, float z, float * out );
void myTranslate( float tx, float ty, float tz, float * out );
void myScale( float sx, float sy, float sz, float * out );
void myMatrixMultiply( const float * a, const float * b, float * out );

/*
 ====================================
 myFrustum
 * This is equivalent to glFrustrumf().  It generates a perspective
 * projection matrix and stores it in float * out
 ====================================
 */
void myFrustum( float xmin, float xmax, float ymin, float ymax, float zNear, float zFar, float * out );

/*
 ====================================
 myPerspective
 * This is my version of glut's convenient perspective
 * matrix generation
 ====================================
 */
void myPerspective( float fovy, float aspect_ratio, float near, float far, float * out );

/*
 ===================================
 myOrtho
 * This is equivalent to opengl's glOrthof().  It generates
 * an orthographic projection matrix.
 ===================================
 */
void myOrtho( float xmin, float xmax, float ymin, float ymax, float znear, float zfar, float * out );

/*
 ===================================
 myLookAt
 * These two functions behave like the
 * gluLookAt function
 ===================================
 */
void myLookAt( Vec3 Eye, Vec3 At, Vec3 Up, float * out );
void myLook( const Vec3 & Eye, const Vec3 & Look, const Vec3 & Up, float * out );
void myLookAt( float eyex, float eyey, float eyez,
                  float atx, float aty, float atz,
                  float upx, float upy, float upz, float * m );

/*
 ===================================
 myOrient
 * Create an orientation matrix from pos,
 * fwd and up vectors
 ===================================
 */
void myOrient( Vec3 Pos, Vec3 Fwd, Vec3 Up, float * out );


/*
 ===================================
 hbLerp
 ===================================
 */
template <class T>
inline T hbLerp( const T& a, const T& b, const float& t ) {
    return a * ( 1.0f - t ) + b * t;
}

/*
 ===================================
 hbPrintMatrix4x4
 ===================================
 */
void hbPrintMatrix4x4( const float * mat );

/*
 ===================================
 myMatrixDeterminant
 ===================================
 */
float myMatrixDeterminant2x2( const float * mat );
float myMatrixDeterminant3x3( const float * mat );
float myMatrixDeterminant4x4( const float * mat );
/*
 ===================================
 myMatrixInverse
 ===================================
 */
void myMatrixInverse2x2( const float * mat, float * b );
void myMatrixInverse3x3( const float * mat, float * b );
void myMatrixInverse4x4( const float * mat, float * b );

/*
 ===================================
 myTransformVector4D
 * Transform a 4D vector by a 4x4 matrix
 * Assumes opengl column major matrix
 ===================================
 */
void myTransformVector4D( const float * mat, const float * vec, float * vec_out );
void TestMatrixOps();

/*
 ===================================
 myUnProject
  ===================================
 */
bool myUnProject( const float& screenX, const float& screenY,
                    const float * matViewProj, 
                    const float& screenWidth, const float& screenHeight,
                    float& worldX, float& worldY, float& worldZ );

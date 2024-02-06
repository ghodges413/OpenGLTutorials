//
//  MatrixOps.cpp
//
#include "Math/MatrixOps.h"
#include <math.h>
#include <string>

#define MY_PI 3.1415926f

/*
 ====================================
 myMatrix16Copy
 ====================================
 */
void myMatrix16Copy( const float * in, float * out ) {
	memcpy( out, in, sizeof( float ) * 16 );
}

/*
 ====================================
 myIdentityMatrix
 ====================================
 */
void myIdentityMatrix( float * out ) {
	out[ 0 ] = 1;
	out[ 1 ] = 0;
	out[ 2 ] = 0;
	out[ 3 ] = 0;
	
	out[ 4 ] = 0;
	out[ 5 ] = 1;
	out[ 6 ] = 0;
	out[ 7 ] = 0;
	
	out[ 8 ] = 0;
	out[ 9 ] = 0;
	out[ 10 ] = 1;
	out[ 11 ] = 0;
	
	out[ 12 ] = 0;
	out[ 13 ] = 0;
	out[ 14 ] = 0;
	out[ 15 ] = 1;
}

/*
 ====================================
 myRotateX
 ====================================
 */
void myRotateX( float angleDegrees, float * out ) {
	const float cosA = cosf( angleDegrees * MY_PI / 180.0f );
	const float sinA = sinf( angleDegrees * MY_PI / 180.0f );
	
	out[ 0 ] = 1;
	out[ 1 ] = 0;
	out[ 2 ] = 0;
	out[ 3 ] = 0;
	
	out[ 4 ] = 0;
	out[ 5 ] = cosA;
	out[ 6 ] = sinA;
	out[ 7 ] = 0;
	
	out[ 8 ] = 0;
	out[ 9 ] = -sinA;
	out[ 10 ] = cosA;
	out[ 11 ] = 0;
	
	out[ 12 ] = 0;
	out[ 13 ] = 0;
	out[ 14 ] = 0;
	out[ 15 ] = 1;
}

/*
 ====================================
 myRotateY
 ====================================
 */
void myRotateY( float angleDegrees, float * out ) {
	const float cosA = cosf( angleDegrees * MY_PI / 180.0f );
	const float sinA = sinf( angleDegrees * MY_PI / 180.0f );
	
	out[ 0 ] = cosA;
	out[ 1 ] = 0;
	out[ 2 ] = -sinA;
	out[ 3 ] = 0;
	
	out[ 4 ] = 0;
	out[ 5 ] = 1;
	out[ 6 ] = 0;
	out[ 7 ] = 0;
	
	out[ 8 ] = sinA;
	out[ 9 ] = 0;
	out[ 10 ] = cosA;
	out[ 11 ] = 0;
	
	out[ 12 ] = 0;
	out[ 13 ] = 0;
	out[ 14 ] = 0;
	out[ 15 ] = 1;
}

/*
 ====================================
 myRotateZ
 ====================================
 */
void myRotateZ( float angleDegrees, float * out ) {
	const float cosA = cosf( angleDegrees * MY_PI / 180.0f );
	const float sinA = sinf( angleDegrees * MY_PI / 180.0f );
	
	out[ 0 ] = cosA;
	out[ 1 ] = sinA;
	out[ 2 ] = 0;
	out[ 3 ] = 0;
	
	out[ 4 ] = -sinA;
	out[ 5 ] = cosA;
	out[ 6 ] = 0;
	out[ 7 ] = 0;
	
	out[ 8 ] = 0;
	out[ 9 ] = 0;
	out[ 10 ] = 1;
	out[ 11 ] = 0;
	
	out[ 12 ] = 0;
	out[ 13 ] = 0;
	out[ 14 ] = 0;
	out[ 15 ] = 1;
}

/*
 ====================================
 myRotate
 ====================================
 */
void myRotate( float angleDegrees, float x, float y, float z, float * out ) {
	const float cosA = cosf( angleDegrees * MY_PI / 180.0f );
	const float sinA = sinf( angleDegrees * MY_PI / 180.0f );
	
	out[ 0 ] =	cosA					+ x * x * ( 1.0f - cosA );
	out[ 1 ] =	y * x * ( 1.0f - cosA ) + z * sinA;
	out[ 2 ] =	z * x * ( 1.0f - cosA ) - y * sinA;
	out[ 3 ] =	0;
	
	out[ 4 ] =	x * y * ( 1.0f - cosA ) - z * sinA;
	out[ 5 ] =	cosA					+ y * y * ( 1.0f - cosA );
	out[ 6 ] =	z * y * ( 1.0f - cosA ) + x * sinA;
	out[ 7 ] =	0;
	
	out[ 8 ] =	x * z * ( 1.0f - cosA ) + y * sinA;
	out[ 9 ] =	y * z * ( 1.0f - cosA ) - x * sinA;
	out[ 10 ] =	cosA					+ z * z * ( 1.0f - cosA );
	out[ 11 ] =	0;
	
	out[ 12 ] = 0;
	out[ 13 ] = 0;
	out[ 14 ] = 0;
	out[ 15 ] = 1;
}

/*
 ====================================
 myTranslate
 ====================================
 */
void myTranslate( float tx, float ty, float tz, float * out ) {
	out[ 0 ] = 1;
	out[ 1 ] = 0;
	out[ 2 ] = 0;
	out[ 3 ] = 0;
	
	out[ 4 ] = 0;
	out[ 5 ] = 1;
	out[ 6 ] = 0;
	out[ 7 ] = 0;
	
	out[ 8 ] = 0;
	out[ 9 ] = 0;
	out[ 10 ] = 1;
	out[ 11 ] = 0;
	
	out[ 12 ] = tx;
	out[ 13 ] = ty;
	out[ 14 ] = tz;
	out[ 15 ] = 1;
}

/*
 ====================================
 myScale
 ====================================
 */
void myScale( float sx, float sy, float sz, float * out ) {
	out[ 0 ] = sx;
	out[ 1 ] = 0;
	out[ 2 ] = 0;
	out[ 3 ] = 0;
	
	out[ 4 ] = 0;
	out[ 5 ] = sy;
	out[ 6 ] = 0;
	out[ 7 ] = 0;
	
	out[ 8 ] = 0;
	out[ 9 ] = 0;
	out[ 10 ] = sz;
	out[ 11 ] = 0;
	
	out[ 12 ] = 0;
	out[ 13 ] = 0;
	out[ 14 ] = 0;
	out[ 15 ] = 1;
}

/*
 ====================================
 myMatrixMultiply
 ====================================
 */
void myMatrixMultiply( const float * a, const float * b, float * out ) {
	out[ 0 ]  = a[ 0 ]*b[ 0 ] + a[ 1 ]*b[ 4 ] + a[ 2 ]*b[ 8 ]  + a[3 ]*b[12];
	out[ 1 ]  = a[ 0 ]*b[ 1 ] + a[ 1 ]*b[ 5 ] + a[ 2 ]*b[ 9 ]  + a[3 ]*b[13];
	out[ 2 ]  = a[ 0 ]*b[ 2 ] + a[ 1 ]*b[ 6 ] + a[ 2 ]*b[ 10 ] + a[3 ]*b[14];
	out[ 3 ]  = a[ 0 ]*b[ 3 ] + a[ 1 ]*b[ 7 ] + a[ 2 ]*b[ 11 ] + a[3 ]*b[15];
	
	out[ 4 ]  = a[ 4 ]*b[ 0 ] + a[ 5 ]*b[ 4 ] + a[ 6 ]*b[ 8 ]  + a[7 ]*b[12];
	out[ 5 ]  = a[ 4 ]*b[ 1 ] + a[ 5 ]*b[ 5 ] + a[ 6 ]*b[ 9 ]  + a[7 ]*b[13];
	out[ 6 ]  = a[ 4 ]*b[ 2 ] + a[ 5 ]*b[ 6 ] + a[ 6 ]*b[ 10 ] + a[7 ]*b[14];
	out[ 7 ]  = a[ 4 ]*b[ 3 ] + a[ 5 ]*b[ 7 ] + a[ 6 ]*b[ 11 ] + a[7 ]*b[15];
	
	out[ 8 ]  = a[ 8 ]*b[ 0 ] + a[ 9 ]*b[ 4 ] + a[ 10 ]*b[ 8 ]  + a[11]*b[12];
	out[ 9 ]  = a[ 8 ]*b[ 1 ] + a[ 9 ]*b[ 5 ] + a[ 10 ]*b[ 9 ]  + a[11]*b[13];
	out[ 10 ] = a[ 8 ]*b[ 2 ] + a[ 9 ]*b[ 6 ] + a[ 10 ]*b[ 10 ] + a[11]*b[14];
	out[ 11 ] = a[ 8 ]*b[ 3 ] + a[ 9 ]*b[ 7 ] + a[ 10 ]*b[ 11 ] + a[11]*b[15];
	
	out[ 12 ] = a[ 12 ]*b[ 0 ] + a[ 13 ]*b[ 4 ] + a[ 14 ]*b[ 8 ]  + a[15]*b[12];
	out[ 13 ] = a[ 12 ]*b[ 1 ] + a[ 13 ]*b[ 5 ] + a[ 14 ]*b[ 9 ]  + a[15]*b[13];
	out[ 14 ] = a[ 12 ]*b[ 2 ] + a[ 13 ]*b[ 6 ] + a[ 14 ]*b[ 10 ] + a[15]*b[14];
	out[ 15 ] = a[ 12 ]*b[ 3 ] + a[ 13 ]*b[ 7 ] + a[ 14 ]*b[ 11 ] + a[15]*b[15];
}

/*
 ====================================
 myPerspective
 * This is my version of glut's convenient perspective
 * matrix generation
 ====================================
 */
void myPerspective( float fovy, float aspect_ratio, float near, float far, float * out ) {
    // This perspective matrix definition was defined in http://www.opengl.org/sdk/docs/man/xhtml/gluPerspective.xml
    const float fovy_radians = fovy * MY_PI / 180.0f;
    const float f = 1.0f / tanf( fovy_radians * 0.5f );
    const float xscale = f;
    const float yscale = f / aspect_ratio;
    
    out[ 0 ] = xscale;
    out[ 1 ] = 0;
    out[ 2 ] = 0;
    out[ 3 ] = 0;
    
    out[ 4 ] = 0;
    out[ 5 ] = yscale;
    out[ 6 ] = 0;
    out[ 7 ] = 0;
    
    out[ 8 ] = 0;
    out[ 9 ] = 0;
    out[ 10] = ( far + near ) / ( near - far );
    out[ 11] = -1;
    
    out[ 12] = 0;
    out[ 13] = 0;
    out[ 14] = ( 2.0f * far * near ) / ( near - far );
    out[ 15] = 0;
}

/*
 ====================================
 myFrustum
 * This is equivalent to glFrustrumf().  It generates a perspective
 * projection matrix and stores it in float * out
 ====================================
 */
void myFrustum( float xmin, float xmax, float ymin, float ymax, float zNear, float zFar, float * out ) {
	const float width = xmax - xmin;
	const float height = ymax - ymin;
	
	out[ 0 ] = 2.0f * zNear / width;
	out[ 1 ] = 0;
	out[ 2 ] = 0;
	out[ 3 ] = 0;
	
	out[ 4 ] = 0;
	out[ 5 ] = 2.0f * zNear / height;
	out[ 6 ] = 0;
	out[ 7 ] = 0;
	
	const float A = ( xmax + xmin ) / width;
	const float B = ( ymax + ymin ) / height;
	const float C = - ( zFar + zNear ) / ( zFar - zNear );
	const float D = - 2.0f * ( zFar + zNear ) / ( zFar - zNear );
	
	out[ 8 ] = A;
	out[ 9 ] = B;
	out[ 10 ] = C;
	out[ 11 ] = -1;
	
	out[ 12 ] = 0;
	out[ 13 ] = 0;
	out[ 14 ] = D;
	out[ 15 ] = 0;
}

/*
 ===================================
 myOrtho
 * This is equivalent to opengl's glOrthof().  It generates
 * an orthographic projection matrix.
 ===================================
 */
void myOrtho( float xmin, float xmax, float ymin, float ymax, float znear, float zfar, float * out ) {
	const float width	= xmax - xmin;
	const float height	= ymax - ymin;
	const float depth	= zfar - znear;
	
	const float tx = -( xmax + xmin ) / width;
	const float ty = -( ymax + ymin ) / height;
	const float tz = -( zfar + znear ) / depth;
	
    // scaling
	out[ 0 ] = 2.0f / width;
	out[ 1 ] = 0;
	out[ 2 ] = 0;
	out[ 3 ] = 0;
	
	out[ 4 ] = 0;
	out[ 5 ] = 2.0f / height;
	out[ 6 ] = 0;
	out[ 7 ] = 0;
	
	out[ 8 ] = 0;
	out[ 9 ] = 0;
	out[ 10 ] = -2.0f / depth;
	out[ 11 ] = 0;
	
	// translation
	out[ 12 ] = tx;
	out[ 13 ] = ty;
	out[ 14 ] = tz;
	out[ 15 ] = 1;
}

/*
 ===================================
 myLookAt
 * A translation and rotation matrix to transform
 * a world space vertex to camera space
 ===================================
 */
#include "Vector.h"
void myLookAt( Vec3d Eye, Vec3d At, Vec3d Up, float * out ) {
//  From DirectX documentation:
//    zaxis = normal(At - Eye)
//    xaxis = normal(cross(Up, zaxis))
//    yaxis = cross(zaxis, xaxis)
//
//    xaxis.x           yaxis.x           zaxis.x          0
//    xaxis.y           yaxis.y           zaxis.y          0
//    xaxis.z           yaxis.z           zaxis.z          0
//    -dot(xaxis, eye)  -dot(yaxis, eye)  -dot(zaxis, eye)  1
//  Since DirectX handles its matrices as row major...
//  and OpenGL is column major...
//  for OpenGL it should be the transverse of this matrix:
//    xaxis.x           xaxis.y           xaxis.z          -dot(xaxis, eye)
//    yaxis.x           yaxis.y           yaxis.z          -dot(yaxis, eye)
//    zaxis.x           zaxis.y           zaxis.z          -dot(zaxis, eye)
//    0                 0                 0                 1

    
    Vec3d Look = Eye - At;                // The look direction
    Look.Normalize();
    Vec3d Left = Up.Cross( Look );        // The left direction
    Left.Normalize();
    Vec3d trueUp = Look.Cross( Left );    // The up direction
    trueUp.Normalize();
    
    myLook( Eye, Look, trueUp, out );
}
void myLook( const Vec3d & Eye, const Vec3d & Look, const Vec3d & Up, float * out ) {
    const Vec3d & zaxis = Look;               // The look direction
    const Vec3d   xaxis = Up.Cross( Look );   // The left direction
    const Vec3d & yaxis = Up;                 // The up direction
    
#ifdef DIRECTX
    out[0] = xaxis.x;
    out[1] = xaxis.y;
    out[2] = xaxis.z;
    out[3] = -xaxis.DotProduct( Eye );
    
    out[4] = yaxis.x;
    out[5] = yaxis.y;
    out[6] = yaxis.z;
    out[7] = -yaxis.DotProduct( Eye );
    
    out[8] = zaxis.x;
    out[9] = zaxis.y;
    out[10] = zaxis.z;
    out[11] = -zaxis.DotProduct( Eye );
    
    out[12] = 0;
    out[13] = 0;
    out[14] = 0;
    out[15] = 1;
#else // OPENGL
    out[0] = xaxis.x;
    out[1] = yaxis.x;
    out[2] = zaxis.x;
    out[3] = 0;
    
    out[4] = xaxis.y;
    out[5] = yaxis.y;
    out[6] = zaxis.y;
    out[7] = 0;
    
    out[8] = xaxis.z;
    out[9] = yaxis.z;
    out[10] = zaxis.z;
    out[11] = 0;
    
    out[12] = -xaxis.DotProduct( Eye );
    out[13] = -yaxis.DotProduct( Eye );
    out[14] = -zaxis.DotProduct( Eye );
    out[15] = 1;
#endif
}

/*
 ===================================
 myLookAt
 ===================================
 */
float Dot( const float * a, const float * b ) {
    return ( a[0] * b[0] + a[1] * b[1] + a[2] * b[2] );
}
void Normalize( float * v ) {
    float mag = sqrt( Dot( v, v ) );
    v[0] /= mag;
    v[1] /= mag;
    v[2] /= mag;
}
void Cross( const float * a, const float * b, float * v ) {
    v[0] =  a[1] * b[2] - a[2] * b[1];
    v[1] = -a[0] * b[2] + a[2] * b[0];
    v[2] =  a[0] * b[1] - a[1] * b[0];
}
void myLookAt( float eyex, float eyey, float eyez,
                  float atx, float aty, float atz,
                  float upx, float upy, float upz, float * m ) {
    float right[3];
    float up[3];
    float look[3];
    
    // temporary up
    float y[3];
    y[0] = upx;
    y[1] = upy;
    y[2] = upz;
    
#if 1
    // calculate normalized look vector
    look[0] = eyex - atx;
    look[1] = eyey - aty;
    look[2] = eyez - atz;
    Normalize( look );
    
    // calculate the right vector
    Cross( y, look, right );
    Normalize( right );
    
    // calculate the up vector
    Cross( look, right, up );
    Normalize( up );
#elif 0
    // calculate normalized look vector
    look[0] = atx - eyex;
    look[1] = atx - eyey;
    look[2] = atx - eyez;
    Normalize( look );
    
    // calculate the right vector
    Cross( look, y, right );
    Normalize( right );
    
    // calculate the up vector
    Cross( right, look, up );
    Normalize( up );
#endif
    
    //
    // Store the final matrix
    //
    m[0] = right[0];
    m[1] = up[0];
    m[2] = look[0];
    m[3] = 0;
    
    m[4] = right[1];
    m[5] = up[1];
    m[6] = look[1];
    m[7] = 0;
    
    m[8] = right[2];
    m[9] = up[2];
    m[10] = look[2];
    m[11] = 0;
    
    float eye[3] = { eyex, eyey, eyez };
    m[12] = -Dot( right, eye );
    m[13] = -Dot( up, eye );
    m[14] = -Dot( look, eye );
    m[15] = 1;    
}

/*
 ===================================
 myOrient
 * Create an orientation matrix from pos,
 * fwd and up vectors
 * Only works properly when the default axis is:
 * +x = fwd
 * +y = left
 * +z = up
 ===================================
 */
void myOrient( Vec3d Pos, Vec3d Fwd, Vec3d Up, float * out ) {
//    xaxis = Fwd
//    yaxis = Left
//    zaxis = Up
//
//  For DirectX it should be:
//    xaxis.x           xaxis.y           xaxis.z   0
//    yaxis.x           yaxis.y           yaxis.z   0
//    zaxis.x           zaxis.y           zaxis.z   0
//    Pos.x             Pos.y             Pos.z     1
//  Since DirectX handles its matrices as row major...
//  and OpenGL is column major...
//  for OpenGL it should be the transverse of this matrix:
//    xaxis.x           yaxis.x           zaxis.x   Pos.x
//    xaxis.y           yaxis.y           zaxis.y   Pos.y
//    xaxis.z           yaxis.z           zaxis.z   Pos.z
//    0                 0                 0         1

    Up.Normalize();
    Fwd.Normalize();
    Vec3d xaxis = Fwd;
    Vec3d yaxis = Up.Cross( Fwd ); // get left vector
    yaxis.Normalize();
    Vec3d zaxis = Up;

#ifdef DIRECTX
    out[0] = xaxis.x;
    out[1] = yaxis.x;
    out[2] = zaxis.x;
    out[3] = Pos.x;
    
    out[4] = xaxis.y;
    out[5] = yaxis.y;
    out[6] = zaxis.y;
    out[7] = Pos.y;
    
    out[8] = xaxis.z;
    out[9] = yaxis.z;
    out[10] = zaxis.z;
    out[11] = Pos.z;
    
    out[12] = 0;
    out[13] = 0;
    out[14] = 0;
    out[15] = 1;
#else // OPENGL
    out[0] = xaxis.x;
    out[1] = xaxis.y;
    out[2] = xaxis.z;
    out[3] = 0;
    
    out[4] = yaxis.x;
    out[5] = yaxis.y;
    out[6] = yaxis.z;
    out[7] = 0;
    
    out[8] = zaxis.x;
    out[9] = zaxis.y;
    out[10] = zaxis.z;
    out[11] = 0;
    
    out[12] = Pos.x;
    out[13] = Pos.y;
    out[14] = Pos.z;
    out[15] = 1;
#endif
}


/*
 ===================================
 hbPrintMatrix4x4
 ===================================
 */
void hbPrintMatrix4x4( const float * mat ) {
    for ( int i = 0; i < 4; ++i ) {
        printf( "%f   %f   %f   %f\n", mat[0+i], mat[4+i], mat[8+i], mat[12+i] );
    }
}




//#include<stdio.h>
//#include<math.h>
//float detrm( float[][], float );
//void cofact( float[][], float );
//void trans( float[][], float[][], float );


/******************FUNCTION TO FIND THE DETERMINANT OF THE MATRIX************************/

float detrm( float a[ 25 ][ 25 ], float k ) {
    float s = 1;
    float det = 0;
    float b[ 25 ][ 25 ];
    int m = 0;
    int n = 0;
    
    if ( k == 1 ) {
        return ( a[ 0 ][ 0 ] );
    } else {
        det = 0;
        
        for ( int c = 0; c < k; c++ ) {
            m = 0;
            n = 0;
            
            for ( int i = 0; i < k; i++ ) {
                for ( int j = 0; j < k; j++ ) {
                    b[ i ][ j ] = 0;
                    
                    if ( i != 0 && j != c ) {
                        b[ m ][ n ] = a[ i ][ j ];
                        
                        float dk2 = k - 2.0f;
                        if ( n < dk2 ) {
                            n++;
                        } else {
                            n = 0;
                            m++;
                        }
                    }
                }
            }
            
            float dk1 = k - 1;
            det = det + s * ( a[ 0 ][ c ] * detrm( b, dk1 ) );
            s = -1 * s;
        }
    }
    
    return ( det );
}

/*************FUNCTION TO FIND TRANSPOSE AND INVERSE OF A MATRIX**************************/

void trans( float num[ 25 ][ 25 ], float fac[ 25 ][ 25 ], float r ) {
    float b[ 25 ][ 25 ];
    float inv[ 25 ][ 25 ];
    float d = 0;
    
    for ( int i = 0;i < r; i++ ) {
        for ( int j = 0;j < r; j++ ) {
            b[ i ][ j ] = fac[ j ][ i ];
        }
    }
    
    d = detrm( num, r );
    
    for ( int i = 0; i < r; i++ ){
        for ( int j = 0; j < r; j++ ) {
            inv[ i ][ j ] = b[ i ][ j ] / d;
        }
    }
    
    printf( "\nTHE INVERSE OF THE MATRIX:\n" );
    
    for ( int i = 0; i < r; i++ ) {
        for ( int j = 0; j < r; j++ ) {
            printf( "\t%f", inv[ i ][ j ] );
        }
        
        printf( "\n" );
    }
}

/*******************FUNCTION TO FIND COFACTOR*********************************/

void cofact( float num[ 25 ][ 25 ], float f ) {
    float b[ 25 ][ 25 ];
    float fac[ 25 ][ 25 ];
    int m = 0;
    int n = 0;
    
    for ( int q = 0; q < f; q++ ) {
        for ( int p = 0; p < f; p++ ) {
            m = 0;
            n = 0;
            
            for ( int i = 0; i < f; i++ ) {
                for ( int j = 0; j < f; j++ ) {
                    b[ i ][ j ] = 0;
                    
                    if ( i != q && j != p ) {
                        b[ m ][ n ] = num[ i ][ j ];
                        
                        float df2 = f - 2.0f;
                        if ( n < df2 ) {
                            n++;
                        } else {
                            n = 0;
                            m++;
                        }
                    }
                }
            }
            
            float df1 = f - 1.0f;
            fac[ q ][ p ] = pow( -1.0f, q + p ) * detrm( b, df1 );
        }
    }
    
    trans( num, fac, f );
}



/******************FUNCTION TO FIND THE DETERMINANT OF THE MATRIX************************/

float detrm2( float * a, float k ) {
    float s = 1;
    float det = 0;
    float b[ 25 ][ 25 ];
    int m = 0;
    int n = 0;
    
    if ( k == 1 ) {
        return ( a[ 0 ] );
    } else {
        det = 0;
        
        for ( int c = 0; c < k; c++ ) {
            m = 0;
            n = 0;
            
            for ( int i = 0; i < k; i++ ) {
                for ( int j = 0; j < k; j++ ) {
                    b[ i ][ j ] = 0;
                    
                    if ( i != 0 && j != c ) {
                        int ik = k;
                        b[ m ][ n ] = a[ ik * i + j ];
                        
                        float dk2 = k - 2.0f;
                        if ( n < dk2 ) {
                            n++;
                        } else {
                            n = 0;
                            m++;
                        }
                    }
                }
            }
            
            float dk1 = k - 1;
            int ik = k;
            det = det + s * ( a[ ik * 0 + c ] * detrm( b, dk1 ) );
            s = -1 * s;
        }
    }
    
    return ( det );
}

float myMatrixDeterminant( float * mat, const int& k ) {
	assert( k > 0 );
    float s = 1;
    float det = 0;
//  float b[ 25 ][ 25 ];
//    float b[ k * k ];
	float b[ 4 * 4 ];
    int m = 0;
    int n = 0;
    
    if ( k == 1 ) {
        return ( mat[ 0 ] );
    } else {
        det = 0;
        
        for ( int c = 0; c < k; c++ ) {
            m = 0;
            n = 0;
            
            for ( int i = 0; i < k; i++ ) {
                for ( int j = 0; j < k; j++ ) {
                    b[ k * i + j ] = 0;
                    
                    if ( i != 0 && j != c ) {
                        int ik = k;
                        b[ k * m + n ] = mat[ ik * i + j ];
                        
                        float dk2 = k - 2.0f;
                        if ( n < dk2 ) {
                            n++;
                        } else {
                            n = 0;
                            m++;
                        }
                    }
                }
            }
            
            float dk1 = k - 1;
            int ik = k;
            det = det + s * ( mat[ ik * 0 + c ] * myMatrixDeterminant( b, dk1 ) );
            s = -1 * s;
        }
    }
    
    return ( det );
}

/*************FUNCTION TO FIND TRANSPOSE AND INVERSE OF A MATRIX**************************/

void hbTransposeMatrix( const float * mat, float * mat_out, const int& r ) {
    for ( int i = 0; i < r; ++i ) {
        for ( int j = 0; j < r; ++j ) {
            mat_out[ r * j + i ] = mat[ r * i + j ];
        }
    }
}

//void hbInverseMatrix( const float * mat, float * inv_out, const int& r ) {
//    float b[ r * r ];
//}

void trans2( float * num, float * fac, float r, float * inv_out ) {
    float d = 0;
    
    const int rank = r;
    //float b[ rank * rank ];
	float b[ 4 * 4 ];
    hbTransposeMatrix( fac, b, rank );
    
    d = detrm2( num, r );
//    d = myMatrixDeterminant( num, r );
    
    for ( int i = 0; i < r; i++ ){
        for ( int j = 0; j < r; j++ ) {
            inv_out[ rank * i + j ] = b[ rank * i + j ] / d;
        }
    }
}

/*******************FUNCTION TO FIND COFACTOR*********************************/

void cofact2( float * num, float f, float * inv_out ) {
    float b[ 25 ][ 25 ];
    float fac[ 25 * 25 ];
    int m = 0;
    int n = 0;
    
    for ( int q = 0; q < f; q++ ) {
        for ( int p = 0; p < f; p++ ) {
            m = 0;
            n = 0;
            
            for ( int i = 0; i < f; i++ ) {
                for ( int j = 0; j < f; j++ ) {
                    b[ i ][ j ] = 0;
                    
                    if ( i != q && j != p ) {
                        int iif = f;
                        b[ m ][ n ] = num[ iif * i + j ];
                        
                        float df2 = f - 2.0f;
                        if ( n < df2 ) {
                            n++;
                        } else {
                            n = 0;
                            m++;
                        }
                    }
                }
            }
            
            int iif = f;
            float df1 = f - 1.0f;
            fac[ iif * q + p ] = pow( -1.0f, q + p ) * detrm( b, df1 );
        }
    }
    
    trans2( num, fac, f, inv_out );
}



void TestMatrixOps() {
    float a[ 25 ][ 25 ];
    const int k = 4;
    float a2[ 16 ];
    float inv_mat[16];
    float d;
    
    myRotateX( 3.14f * 0.25f, a2 );
    myTranslate( 10, 5, 20, a2 );
    printf( "ORIGINAL MATRIX:\n" );
    hbPrintMatrix4x4( a2 );
    
    for ( int i = 0; i < k; i++ ) {
        for ( int j = 0; j < k; j++ ) {
            a[i][j] = a2[ k * i + j ];
        }
    }
    
    d = detrm2( a2, k );
//    d = myMatrixDeterminant( a2, k );
    printf( "THE DETERMINANT IS=%f\n", d );
    
    if ( d == 0 ) {
        printf( "\nMATRIX IS NOT INVERSIBLE\n" );
    } else {
        cofact2( a2, k, inv_mat );
        
        printf( "Inverse matrix:\n" );
        hbPrintMatrix4x4( inv_mat );
    }
    
}


/*
 ===================================
 myMatrixDeterminant
 * Algorithm source: http://www.cg.info.hiroshima-cu.ac.jp/~miyazaki/knowledge/teche23.html
 ===================================
 */
//#define RC( row, column ) ( ( 4 * column ) + row )
#define RC( row, column ) ( ( 4 * row ) + column )
float myMatrixDeterminant4x4( const float * mat ) {
    const float * a = mat;
    float determinant =
    a[RC(1,1)] * a[RC(2,2)] * a[RC(3,3)] * a[RC(3,3)] +
    a[RC(1,1)] * a[RC(2,3)] * a[RC(3,4)] * a[RC(4,2)] +
    a[RC(1,1)] * a[RC(2,4)] * a[RC(3,2)] * a[RC(4,3)] +
    
    a[RC(1,2)] * a[RC(2,1)] * a[RC(3,4)] * a[RC(4,3)] +
    a[RC(1,2)] * a[RC(2,3)] * a[RC(3,1)] * a[RC(4,4)] +
    a[RC(1,2)] * a[RC(2,4)] * a[RC(3,3)] * a[RC(4,1)] +
    
    a[RC(1,3)] * a[RC(2,1)] * a[RC(3,2)] * a[RC(4,4)] +
    a[RC(1,3)] * a[RC(2,2)] * a[RC(3,4)] * a[RC(4,1)] +
    a[RC(1,3)] * a[RC(2,4)] * a[RC(4,1)] * a[RC(4,2)] +
    
    a[RC(1,4)] * a[RC(2,1)] * a[RC(3,3)] * a[RC(4,2)] +
    a[RC(1,4)] * a[RC(2,2)] * a[RC(3,1)] * a[RC(4,3)] +
    a[RC(1,4)] * a[RC(2,3)] * a[RC(3,2)] * a[RC(4,1)] -
    
    a[RC(1,1)] * a[RC(2,2)] * a[RC(3,4)] * a[RC(4,3)] -
    a[RC(1,1)] * a[RC(2,3)] * a[RC(3,2)] * a[RC(4,4)] -
    a[RC(1,1)] * a[RC(2,4)] * a[RC(3,3)] * a[RC(4,2)] -
    
    a[RC(1,2)] * a[RC(2,1)] * a[RC(3,3)] * a[RC(4,4)] -
    a[RC(1,2)] * a[RC(2,3)] * a[RC(3,4)] * a[RC(4,1)] -
    a[RC(1,2)] * a[RC(2,4)] * a[RC(3,1)] * a[RC(4,3)] -
    
    a[RC(1,3)] * a[RC(2,1)] * a[RC(3,4)] * a[RC(4,2)] -
    a[RC(1,3)] * a[RC(2,2)] * a[RC(3,1)] * a[RC(4,4)] -
    a[RC(1,3)] * a[RC(2,4)] * a[RC(3,2)] * a[RC(4,1)] -
    
    a[RC(1,4)] * a[RC(2,1)] * a[RC(3,2)] * a[RC(4,3)] -
    a[RC(1,4)] * a[RC(2,2)] * a[RC(3,3)] * a[RC(4,1)] -
    a[RC(1,4)] * a[RC(2,3)] * a[RC(3,1)] * a[RC(4,2)];
    
    return determinant;
}
/*
 ===================================
 myMatrixInverse
 * Algorithm source: http://www.cg.info.hiroshima-cu.ac.jp/~miyazaki/knowledge/teche23.html
 ===================================
 */
void myMatrixInverse4x4( const float * mat, float * b ) {
#if 1
    const int k = 4;
    float a2[ 16 ];
//    float inv_mat[16];
    
    
    myMatrix16Copy( mat, a2 );
    
    float d = detrm2( a2, k );
//    float d = myMatrixDeterminant( a2, k );
    
    if ( d == 0 ) {
        printf( "\nMATRIX IS NOT INVERSIBLE\n" );
        assert( 0 );
    } else {
        cofact2( a2, k, b );
        
//        printf( "Inverse matrix:\n" );
//        hbPrintMatrix4x4( inv_mat );
    }
#else
    // get the determinant;
    const float determinant = myMatrixDeterminant4x4( mat );
    const float invDeterminant = 1.0f / determinant;
    
    const float * a = mat;
    
    b[RC(1,1)] = a[RC(2,2)]*a[RC(3,3)]*a[RC(4,4)] + a[RC(2,3)]*a[RC(3,4)]*a[RC(4,2)] + a[RC(2,4)]*a[RC(3,2)]*a[RC(4,3)] - 
    a[RC(2,2)]*a[RC(3,4)]*a[RC(4,3)] - a[RC(2,3)]*a[RC(3,2)]*a[RC(4,4)] - a[RC(2,4)]*a[RC(3,3)]*a[RC(4,2)];
    
    b[RC(1,2)] = a[RC(1,2)]*a[RC(3,4)]*a[RC(4,3)] + a[RC(1,3)]*a[RC(3,2)]*a[RC(4,4)] + a[RC(1,4)]*a[RC(3,3)]*a[RC(4,2)] - 
    a[RC(1,2)]*a[RC(3,3)]*a[RC(4,4)] - a[RC(1,3)]*a[RC(3,4)]*a[RC(4,2)] - a[RC(1,4)]*a[RC(3,2)]*a[RC(4,3)];
    
    b[RC(1,3)] = a[RC(1,2)]*a[RC(2,3)]*a[RC(4,4)] + a[RC(1,3)]*a[RC(2,4)]*a[RC(4,2)] + a[RC(1,4)]*a[RC(2,2)]*a[RC(4,3)] - 
    a[RC(1,2)]*a[RC(2,4)]*a[RC(4,3)] - a[RC(1,3)]*a[RC(2,2)]*a[RC(4,4)] - a[RC(1,4)]*a[RC(2,3)]*a[RC(4,2)];
    
    b[RC(1,4)] = a[RC(1,2)]*a[RC(2,4)]*a[RC(3,3)] + a[RC(1,3)]*a[RC(2,2)]*a[RC(3,4)] + a[RC(1,4)]*a[RC(2,3)]*a[RC(3,2)] - 
    a[RC(1,2)]*a[RC(2,3)]*a[RC(3,4)] - a[RC(1,3)]*a[RC(2,4)]*a[RC(3,2)] - a[RC(1,4)]*a[RC(2,2)]*a[RC(3,3)];
    
    
    b[RC(2,1)] = a[RC(2,1)]*a[RC(3,4)]*a[RC(4,3)] + a[RC(2,3)]*a[RC(3,1)]*a[RC(4,4)] + a[RC(2,4)]*a[RC(3,3)]*a[RC(4,1)] - 
    a[RC(2,1)]*a[RC(3,3)]*a[RC(4,1)] - a[RC(2,3)]*a[RC(3,4)]*a[RC(4,1)] - a[RC(2,4)]*a[RC(3,1)]*a[RC(4,3)];
    
    b[RC(2,2)] = a[RC(1,1)]*a[RC(3,3)]*a[RC(4,4)] + a[RC(1,3)]*a[RC(3,4)]*a[RC(4,1)] + a[RC(1,4)]*a[RC(3,1)]*a[RC(4,3)] - 
    a[RC(1,1)]*a[RC(3,4)]*a[RC(4,3)] - a[RC(1,3)]*a[RC(3,1)]*a[RC(4,4)] - a[RC(1,4)]*a[RC(3,3)]*a[RC(4,1)];
    
    b[RC(2,3)] = a[RC(1,1)]*a[RC(2,4)]*a[RC(4,3)] + a[RC(1,3)]*a[RC(2,1)]*a[RC(4,4)] + a[RC(1,4)]*a[RC(2,3)]*a[RC(4,1)] - 
    a[RC(1,1)]*a[RC(2,3)]*a[RC(4,1)] - a[RC(1,3)]*a[RC(2,4)]*a[RC(4,1)] - a[RC(1,4)]*a[RC(2,1)]*a[RC(4,3)];
    
    b[RC(2,4)] = a[RC(1,1)]*a[RC(2,3)]*a[RC(3,4)] + a[RC(1,3)]*a[RC(2,4)]*a[RC(3,1)] + a[RC(1,4)]*a[RC(2,1)]*a[RC(3,4)] - 
    a[RC(1,1)]*a[RC(2,4)]*a[RC(3,3)] - a[RC(1,3)]*a[RC(2,1)]*a[RC(3,4)] - a[RC(1,4)]*a[RC(2,3)]*a[RC(3,4)];
    
    
    b[RC(3,1)] = a[RC(2,1)]*a[RC(3,2)]*a[RC(4,4)] + a[RC(2,2)]*a[RC(3,4)]*a[RC(4,1)] + a[RC(2,4)]*a[RC(3,1)]*a[RC(4,2)] - 
    a[RC(2,1)]*a[RC(3,4)]*a[RC(4,2)] - a[RC(2,2)]*a[RC(3,1)]*a[RC(4,4)] - a[RC(2,4)]*a[RC(3,2)]*a[RC(4,1)];
    
    b[RC(3,2)] = a[RC(1,1)]*a[RC(3,4)]*a[RC(4,2)] + a[RC(1,2)]*a[RC(3,1)]*a[RC(4,4)] + a[RC(1,4)]*a[RC(3,2)]*a[RC(4,1)] - 
    a[RC(1,1)]*a[RC(3,2)]*a[RC(4,4)] - a[RC(1,2)]*a[RC(3,4)]*a[RC(4,1)] - a[RC(1,4)]*a[RC(3,1)]*a[RC(4,2)];
    
    b[RC(3,3)] = a[RC(1,1)]*a[RC(2,2)]*a[RC(4,4)] + a[RC(1,2)]*a[RC(2,4)]*a[RC(4,1)] + a[RC(1,4)]*a[RC(2,1)]*a[RC(4,2)] - 
    a[RC(1,1)]*a[RC(2,4)]*a[RC(4,2)] - a[RC(1,2)]*a[RC(2,1)]*a[RC(4,4)] - a[RC(1,4)]*a[RC(2,2)]*a[RC(4,1)];
    
    b[RC(3,4)] = a[RC(1,1)]*a[RC(2,4)]*a[RC(3,2)] + a[RC(1,2)]*a[RC(2,1)]*a[RC(3,4)] + a[RC(1,4)]*a[RC(2,2)]*a[RC(3,1)] - 
    a[RC(1,1)]*a[RC(2,2)]*a[RC(3,4)] - a[RC(1,2)]*a[RC(2,4)]*a[RC(3,1)] - a[RC(1,4)]*a[RC(2,1)]*a[RC(3,2)];
    
    
    b[RC(4,1)] = a[RC(2,1)]*a[RC(3,3)]*a[RC(4,2)] + a[RC(2,2)]*a[RC(3,1)]*a[RC(4,3)] + a[RC(2,3)]*a[RC(3,2)]*a[RC(4,1)] - 
    a[RC(2,1)]*a[RC(3,2)]*a[RC(4,3)] - a[RC(2,2)]*a[RC(3,3)]*a[RC(4,1)] - a[RC(2,3)]*a[RC(3,1)]*a[RC(4,2)];
    
    b[RC(4,2)] = a[RC(1,1)]*a[RC(3,2)]*a[RC(4,3)] + a[RC(1,2)]*a[RC(3,3)]*a[RC(4,1)] + a[RC(1,3)]*a[RC(3,1)]*a[RC(4,2)] - 
    a[RC(1,1)]*a[RC(3,3)]*a[RC(4,2)] - a[RC(1,2)]*a[RC(3,1)]*a[RC(4,3)] - a[RC(1,3)]*a[RC(3,2)]*a[RC(4,1)];
    
    b[RC(4,3)] = a[RC(1,1)]*a[RC(2,3)]*a[RC(4,2)] + a[RC(1,2)]*a[RC(2,1)]*a[RC(4,3)] + a[RC(1,3)]*a[RC(2,2)]*a[RC(4,3)] - 
    a[RC(1,1)]*a[RC(2,2)]*a[RC(4,3)] - a[RC(1,2)]*a[RC(2,3)]*a[RC(4,1)] - a[RC(1,3)]*a[RC(2,1)]*a[RC(4,2)];
    
    b[RC(4,4)] = a[RC(1,1)]*a[RC(2,2)]*a[RC(3,3)] + a[RC(1,2)]*a[RC(2,3)]*a[RC(3,1)] + a[RC(1,3)]*a[RC(2,1)]*a[RC(3,2)] - 
    a[RC(1,1)]*a[RC(2,3)]*a[RC(3,2)] - a[RC(1,2)]*a[RC(2,1)]*a[RC(3,3)] - a[RC(1,3)]*a[RC(2,2)]*a[RC(3,1)];
    
    // multiply by the determinant
    for ( int i = 0; i < 16; ++i ) {
        b[i] *= invDeterminant;
    }
#endif
}

/*
 ===================================
 myTransformVector4D
 * Transform a 4D vector by a 4x4 matrix
 * Assumes opengl column major matrix
 ===================================
 */
void myTransformVector4D( const float * mat, const float * vec, float * vec_out ) {
    for ( int i = 0; i < 4; ++i ) {
        vec_out[i] = mat[ i + 0 ] * vec[0] +
                     mat[ i + 4 ] * vec[1] +
                     mat[ i + 8 ] * vec[2] +
                     mat[ i + 12] * vec[3];
    }
}


// This code comes from http://code.google.com/p/iphone-glu/downloads/detail?name=iGLU-1.0.0.tar.gz&can=2&q=
//GLint GLAPIENTRY
//gluUnProject(GLdouble winx, GLdouble winy, GLdouble winz,
//             const GLdouble modelMatrix[16], 
//             const GLdouble projMatrix[16],
//             const GLint viewport[4],
//             GLdouble *objx, GLdouble *objy, GLdouble *objz)
//{
//    double finalMatrix[16];
//    double in[4];
//    double out[4];
//    
//    __gluMultMatricesd(modelMatrix, projMatrix, finalMatrix);
//    if (!__gluInvertMatrixd(finalMatrix, finalMatrix)) return(GL_FALSE);
//    
//    in[0]=winx;
//    in[1]=winy;
//    in[2]=winz;
//    in[3]=1.0;
//    
//    /* Map x and y from window coordinates */
//    in[0] = (in[0] - viewport[0]) / viewport[2];
//    in[1] = (in[1] - viewport[1]) / viewport[3];
//    
//    /* Map to range -1 to 1 */
//    in[0] = in[0] * 2 - 1;
//    in[1] = in[1] * 2 - 1;
//    in[2] = in[2] * 2 - 1;
//    
//    __gluMultMatrixVecd(finalMatrix, in, out);
//    if (out[3] == 0.0) return(GL_FALSE);
//    out[0] /= out[3];
//    out[1] /= out[3];
//    out[2] /= out[3];
//    *objx = out[0];
//    *objy = out[1];
//    *objz = out[2];
//    return(GL_TRUE);
//}
//
// This code comes from http://www.opengl.org/wiki/GluProject_and_gluUnProject_code
//int glhProjectf(float objx, float objy, float objz, float *modelview, float *projection, int *viewport, float *windowCoordinate)
//{
//    //Transformation vectors
//    float fTempo[8];
//    //Modelview transform
//    fTempo[0]=modelview[0]*objx+modelview[4]*objy+modelview[8]*objz+modelview[12];  //w is always 1
//    fTempo[1]=modelview[1]*objx+modelview[5]*objy+modelview[9]*objz+modelview[13];
//    fTempo[2]=modelview[2]*objx+modelview[6]*objy+modelview[10]*objz+modelview[14];
//    fTempo[3]=modelview[3]*objx+modelview[7]*objy+modelview[11]*objz+modelview[15];
//    //Projection transform, the final row of projection matrix is always [0 0 -1 0]
//    //so we optimize for that.
//    fTempo[4]=projection[0]*fTempo[0]+projection[4]*fTempo[1]+projection[8]*fTempo[2]+projection[12]*fTempo[3];
//    fTempo[5]=projection[1]*fTempo[0]+projection[5]*fTempo[1]+projection[9]*fTempo[2]+projection[13]*fTempo[3];
//    fTempo[6]=projection[2]*fTempo[0]+projection[6]*fTempo[1]+projection[10]*fTempo[2]+projection[14]*fTempo[3];
//    fTempo[7]=-fTempo[2];
//    //The result normalizes between -1 and 1
//    if(fTempo[7]==0.0)	//The w value
//        return 0;
//    fTempo[7]=1.0/fTempo[7];
//    //Perspective division
//    fTempo[4]*=fTempo[7];
//    fTempo[5]*=fTempo[7];
//    fTempo[6]*=fTempo[7];
//    //Window coordinates
//    //Map x, y to range 0-1
//    windowCoordinate[0]=(fTempo[4]*0.5+0.5)*viewport[2]+viewport[0];
//    windowCoordinate[1]=(fTempo[5]*0.5+0.5)*viewport[3]+viewport[1];
//    //This is only correct when glDepthRange(0.0, 1.0)
//    windowCoordinate[2]=(1.0+fTempo[6])*0.5;	//Between 0 and 1
//    return 1;
//}

/*
 ===================================
 myUnProject
 ===================================
 */
bool myUnProject( const float & screenX, const float & screenY,
                           const float * matViewProj,
                           const float & screenWidth, const float & screenHeight,
                           float & worldX, float & worldY, float & worldZ ) {
    // get a 4d vector of the touch position
    // go ahead and place it at the far end of the depth
    Vec4d touch_position;
    
	const float halfWidth = 0.5f * screenWidth;
	const float halfHeight = 0.5f * screenHeight;

	// Convert from screen space to -1.0f,1.0f gl screen space
	touch_position.x = ( screenX / halfWidth ) - 1.0f;
    touch_position.y = ( screenY / halfHeight ) - 1.0f;
    
    touch_position.z = 1;
    touch_position.w = 1;
    
    //
    // TODO: Add a check that the matViewProj is invertible.
    //
    
    // Get the inverse view/projection transformation
    float matInverseViewProj[16];
    myMatrixInverse4x4( matViewProj, matInverseViewProj );
    
    // get the touch position in world space
    Vec4d touch_world_position;
    myTransformVector4D( matInverseViewProj, touch_position.ToPtr(), touch_world_position.ToPtr() );

    // output the world coordinates
    worldX = touch_world_position.x / touch_world_position.w;
    worldY = touch_world_position.y / touch_world_position.w;
    worldZ = touch_world_position.z / touch_world_position.w;
    return true;
}


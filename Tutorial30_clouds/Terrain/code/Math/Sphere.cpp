//
//  Sphere.cpp
//
#include "Math/Sphere.h"
#include "Miscellaneous/Array.h"

/*
 ===============================
 Sphere::Sphere
 ===============================
 */
Sphere::Sphere() :
mCenter( 0 ),
mRadius( 0 ),
mInitialized( false ) {
}

/*
 ===============================
 Sphere::Sphere
 ===============================
 */
Sphere::Sphere( const Vec3d & center, const float radius ) :
mCenter( center ),
mRadius( radius ),
mInitialized( true ) {
}

/*
 ===============================
 Sphere::Sphere
 ===============================
 */
Sphere::Sphere( const float x, const float y, const float z, const float r ) :
mCenter( x, y, z ),
mRadius( r ),
mInitialized( true ) {
}

/*
 ===============================
 Sphere::Sphere
 ===============================
 */
Sphere::Sphere( const Sphere & rhs ) :
mCenter( rhs.mCenter ),
mRadius( rhs.mRadius ),
mInitialized( rhs.mInitialized ) {
}

/*
 ===============================
 Sphere::Sphere
 ===============================
 */
Sphere::Sphere( const Vec3d * pts, const int num_pts ) {
    assert( num_pts > 0 );
    mRadius = 0;
    mCenter = pts[0];
    mInitialized = true;
    
    for ( int i = 1; i < num_pts; ++i ) {
        Inflate( pts[i] );
    }
}

/*
 ===============================
 Sphere::operator =
 ===============================
 */
const Sphere & Sphere::operator = ( const Sphere & rhs ) {
    mCenter         = rhs.mCenter;
    mRadius         = rhs.mRadius;
    mInitialized    = rhs.mInitialized;
    return *this;
}

/*
 ===============================
 Sphere::Inflate
 * this uses Ritter's method...
 * a better result can be obtained from
 * EPOS
 ===============================
 */
void Sphere::Inflate( const Vec3d& pt ) {
    if ( !mInitialized ) {
        mRadius = 0;
        mCenter = pt;
        mInitialized = true;
        return;
    }
    
    // get the squared distance between center and the point
    Vec3d d = pt - mCenter;
    float dist_sqr = d.DotProduct( d );
    if ( dist_sqr > mRadius * mRadius ) {
        float dist = sqrtf( dist_sqr );
        float new_radius = (mRadius + dist) * 0.5f;
        float k = (new_radius - mRadius) / dist;
        mRadius = new_radius;
        mCenter += d * k;
    }
}

/*
 ===============================
 Sphere::Inflate
 ===============================
 */
void Sphere::Inflate( const Vec3d * pts, const int& num_pts ) {
    for ( int i = 0; i < num_pts; ++i ) {
        Inflate( pts[i] );
    }
}

/*
 ===============================
 Sphere::TightestFit
 * Mathematically this will produce the absolute tightest fitting
 * sphere.  However, the penalty is that it's slow and
 * takes approximately O(n) = 1/2 * n^2 time.
 *
 * Only works on rectangular/quadric like objects
 * does not work on triangular/tetrahedral
 ===============================
 */
#include <stdio.h>
void Sphere::TightestFit( const Vec3d * pts, const int& num_pts ) {
    assert( num_pts > 2 );
    
    // compare distances to discover which two points are the furthest apart
    float max_distance = 0;
    int max_pts[2] = { -1 }; // the id's of the max pts
    Vec3d pt0;
    Vec3d pt1;
    for ( int i = 0; i < num_pts; ++i ) {
        pt0 = pts[i];
        for ( int j = i + 1; j < num_pts; ++j ) {
            pt1 = pts[j];
            float dist = (pt0 - pt1).GetMagnitude();
            if ( dist > max_distance ) {
                max_distance = dist;
                max_pts[0] = i;
                max_pts[1] = j;
            }
        }
    }
    pt0 = pts[ max_pts[0] ];
    pt1 = pts[ max_pts[1] ];
    
    mCenter = pt0 + pt1;
    mCenter *= 0.5f;
    
    mRadius = max_distance * 0.5f;
    mInitialized = true;
    
    printf( "Center: xyz%f  %f  %f  R: %f\n", mCenter.x, mCenter.y, mCenter.z, mRadius );
}

/*
 ===============================
 Sphere::GeometricFit
 ===============================
 */
void Sphere::GeometricFit( const Vec3d *pts, const int &num_pts ) {
    assert( num_pts > 1 );
    
    // find the center point
    mCenter = Vec3d( 0, 0, 0 );
    for ( int i = 0; i < num_pts; ++i ) {
        mCenter += pts[i];
    };
    mCenter /= (float)num_pts;
    
    // find radius
    mRadius = 0;
    for ( int i = 0; i < num_pts; ++i ) {
        Vec3d ray = mCenter - pts[i];
        float distance = ray.GetMagnitude();
        if ( distance > mRadius ) {
            mRadius = distance;
        }
    }
    
    mInitialized = true;
}

/*
 ===============================
 Sphere::TesellateTriangle
 
 Breaks the incoming triangle into 4 triangles, with the same ordering
 as the incoming triangle.  The 4 sub-triangles are geometrically similar
 to the original.
 ===============================
 */
void TessellateTriangle( const Vec3d & a, const Vec3d & b, const Vec3d & c, Array< Vec3d > & out ) {
//    out.Expand( 12 );
    out.Clear();
    Vec3d midAB = 0.5f * ( a + b );
    Vec3d midBC = 0.5f * ( b + c );
    Vec3d midCA = 0.5f * ( c + a );
    
    // top tri
    out.Append( a );
    out.Append( midAB );
    out.Append( midCA );
    
    // lower left tri
    out.Append( midAB );
    out.Append( b );
    out.Append( midBC );
    
    // lower right tri
    out.Append( midCA );
    out.Append( midBC );
    out.Append( c );
    
    // center tri
    out.Append( midBC );
    out.Append( midCA );
    out.Append( midAB );
}

/*
 ===============================
 Sphere::TessellateAndNormalize
 
 Takes a list of triangles and tessellates each one into 4 geometrically similar
 triangles.  Each point in the new triangles is normalized.  The new list of
 triangles is stored in the out array.
 ===============================
 */
void TessellateAndNormalize( const Array< Vec3d > & in, Array< Vec3d > & out ) {
//    out.Resize( in.Num() * 4 );
    out.Clear();
    
    Array< Vec3d > temp;
    for ( int i = 0; i < in.Num(); i += 3 ) {
        // Tesellate the triangle
        TessellateTriangle( in[ i ], in[ i + 1 ], in[ i + 2 ], temp );
        
        // Normalize the points
        for ( int j = 0; j < temp.Num(); ++j ) {
            temp[ j ].Normalize();
        }
        
        // Copy the tessellated triangle into the out array
        for ( int j = 0; j < temp.Num(); ++j ) {
            out.Append( temp[ j ] );
        }
    }
}

/*
 ===============================
 Sphere::BuildHalfSphereGeo
 Builds a half sphere, similar to the uniformly distributed sphere.
 These are only points that in the xy plane and above
 ===============================
 */
void Sphere::BuildHalfSphereGeo( Array< Vec3d > & out, const int & numSteps ) {
    // Begin with an pyramid
    const float pi = acosf( -1.0f );
	const float angle = pi * 2.0f / 3.0f;
	const float x = cosf( angle );
	const float y = sinf( angle );

	Vec3d points[ 4 ];
	points[ 0 ] = Vec3d( 1, 0, 0 );
    points[ 1 ] = Vec3d( x, y, 0 );
    points[ 2 ] = Vec3d( x,-y, 0 );
    points[ 3 ] = Vec3d( 0, 0, 1 );
    
    const int numStartTris = 4;
    const int numPoints = numStartTris * 3;
    Vec3d octahedron[ numPoints ];
    
    // TOP (CCW)
    octahedron[ 0 ] = points[ 3 ];
    octahedron[ 1 ] = points[ 0 ];
    octahedron[ 2 ] = points[ 1 ];
    
    octahedron[ 3 ] = points[ 3 ];
    octahedron[ 4 ] = points[ 1 ];
    octahedron[ 5 ] = points[ 2 ];
    
    octahedron[ 6 ] = points[ 3 ];
    octahedron[ 7 ] = points[ 2 ];
    octahedron[ 8 ] = points[ 0 ];
    
	// Bottom (CCW)
    octahedron[ 9 ] = points[ 0 ];
    octahedron[ 10 ] = points[ 2 ];
    octahedron[ 11 ] = points[ 1 ];
        
    Array< Vec3d > buffer0;
    Array< Vec3d > buffer1;
    
    const int numFinalTris	= numStartTris * powf( 4, numSteps );
    const int finalSize		= numFinalTris * 3;
    buffer0.Expand( finalSize );
    buffer1.Expand( finalSize );
    buffer0.Clear();
    buffer1.Clear();
    
    // copy the octahedron into the beginning buffer
    for ( int i = 0; i < numPoints; ++i ) {
        buffer0.Append( octahedron[ i ] );
    }
    
    for ( int i = 0; i < numSteps; ++i ) {
        // Use the data in buffer 0 to fill buffer 1 with tessellated sphere data
        TessellateAndNormalize( buffer0, buffer1 );
        
        // Copy the final buffer stuff into the original
        buffer0 = buffer1;
    }
    
    // Copy the final data
    out = buffer0;
}

/*
 ===============================
 Sphere::BuildGeometry
 Builds a sphere with vertices that are uniformly distributed around
 ===============================
 */
void Sphere::BuildGeometry( Array< Vec3d > & out, const int & numSteps ) {
    // Begin with an octahedron
    Vec3d points[ 6 ];
    points[ 0 ] = Vec3d( 1, 0, 0 );
    points[ 1 ] = Vec3d( 0, 1, 0 );
    points[ 2 ] = Vec3d( 0, 0, 1 );
    points[ 3 ] = Vec3d(-1, 0, 0 );
    points[ 4 ] = Vec3d( 0,-1, 0 );
    points[ 5 ] = Vec3d( 0, 0,-1 );
    
    const int numStartTris = 4 * 2;
    const int numPoints = numStartTris * 3;
    Vec3d octahedron[ numPoints ];
    
    // TOP (CCW)
    octahedron[ 0 ] = points[ 2 ];
    octahedron[ 1 ] = points[ 0 ];
    octahedron[ 2 ] = points[ 1 ];
    
    octahedron[ 3 ] = points[ 2 ];
    octahedron[ 4 ] = points[ 1 ];
    octahedron[ 5 ] = points[ 3 ];
    
    octahedron[ 6 ] = points[ 2 ];
    octahedron[ 7 ] = points[ 3 ];
    octahedron[ 8 ] = points[ 4 ];
    
    octahedron[ 9 ] = points[ 2 ];
    octahedron[ 10 ] = points[ 4 ];
    octahedron[ 11 ] = points[ 0 ];
    
    // Bottom (CCW)
    octahedron[ 12 ] = points[ 5 ];
    octahedron[ 13 ] = points[ 1 ];
    octahedron[ 14 ] = points[ 0 ];
    
    octahedron[ 15 ] = points[ 5 ];
    octahedron[ 16 ] = points[ 3 ];
    octahedron[ 17 ] = points[ 1 ];
    
    octahedron[ 18 ] = points[ 5 ];
    octahedron[ 19 ] = points[ 4 ];
    octahedron[ 20 ] = points[ 3 ];
    
    octahedron[ 21 ] = points[ 5 ];
    octahedron[ 22 ] = points[ 0 ];
    octahedron[ 23 ] = points[ 4 ];
    
    Array< Vec3d > buffer0;
    Array< Vec3d > buffer1;
    
    const int numFinalTris	= numStartTris * powf( 4, numSteps );
    const int finalSize		= numFinalTris * 3;
    buffer0.Expand( finalSize );
    buffer1.Expand( finalSize );
    buffer0.Clear();
    buffer1.Clear();
    
    // copy the octahedron into the beginning buffer
    for ( int i = 0; i < numPoints; ++i ) {
        buffer0.Append( octahedron[ i ] );
    }
    
    for ( int i = 0; i < numSteps; ++i ) {
        // Use the data in buffer 0 to fill buffer 1 with tessellated sphere data
        TessellateAndNormalize( buffer0, buffer1 );
        
        // Copy the final buffer stuff into the original
        buffer0 = buffer1;
    }
    
    // Copy the final data
    out = buffer0;
}

/*
 ===============================
 Sphere::Expand
 ===============================
 */
Sphere Sphere::Expand( const Vec3d & poleA, const Vec3d & poleB, const Vec3d & ext ) {
    const Vec3d center = 0.5f * ( poleA + poleB );
    const float diameter = ( poleA - poleB ).GetMagnitude();
    
    Sphere sphere( center, 0.5f * diameter );
    
    // So here's the deal... I think the damn sphere can only expand horizontally
    // (perpendicular to the pole)
    
    return sphere;
}

/*
 ================================
 Sphere::IsPointInSphere
 ================================
 */
bool Sphere::IsPointInSphere( const Vec3d & point ) const {
	if ( false == mInitialized ) {
		return false;
	}

	const Vec3d diff	= point - mCenter;
	const float distSqr	= diff.DotProduct( diff );

	return distSqr < mRadius * mRadius;
}

/*
 ================================
 Sphere::IntersectSphere
 ================================
 */
bool Sphere::IntersectSphere( const Sphere & rhs ) const {
	if ( false == mInitialized || false == rhs.mInitialized ) {
		return false;
	}

	const Vec3d diff	= mCenter - rhs.mCenter;
	const float distSqr	= diff.DotProduct( diff );

	const float radSum	= mRadius + rhs.mRadius;

	return distSqr < radSum * radSum;
}














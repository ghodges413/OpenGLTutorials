//
//	Frustum.cpp
//
#include "Math/Frustum.h"
#include "Math/MatrixOps.h"
#include "Math/Quat.h"

/*
 ================================
 Frustum::Frustum
 ================================
 */
Frustum::Frustum() :
m_isInitialized( false ) {
}

/*
 ================================
 Frustum::Frustum
 ================================
 */
Frustum::Frustum( const Frustum & rhs ) :
m_isInitialized( rhs.m_isInitialized ) {
    for ( int i = 0; i < 6; ++i ) {
        m_planes[ i ] = rhs.m_planes[ i ];
    }
	for ( int i = 0; i < 8; ++i ) {
        m_corners[ i ] = rhs.m_corners[ i ];
    }
	m_bounds = rhs.m_bounds;
}

/*
 ================================
 Frustum::Frustum
 ================================
 */
Frustum::Frustum( const float & nearz,
                             const float & farz,
                             const float & fovy_degrees,
                             const float & screenWidth,
                             const float & screenHeight,
                             const Vec3 & camPos,
                             const Vec3 & camUp,
                             const Vec3 & camLook ) {
    Build( nearz, farz, fovy_degrees, screenWidth, screenHeight, camPos, camUp, camLook );
    m_isInitialized = true;
}

/*
 ================================
 Frustum::Frustum
 ================================
 */
Frustum::Frustum( const float * mat ) {
    Build( mat );
    m_isInitialized = true;
}

/*
 ================================
 Frustum::Frustum
 ================================
 */
Frustum::Frustum( const float * proj, const float * view ) {
    Build( proj, view );
    m_isInitialized = true;
}

/*
 ================================
 Frustum::operator=
 ================================
 */
Frustum& Frustum::operator = ( const Frustum & rhs ) {
    for ( int i = 0; i < 6; ++i ) {
        m_planes[ i ] = rhs.m_planes[ i ];
    }
	m_bounds = rhs.m_bounds;
	m_isInitialized = rhs.m_isInitialized;
	return *this;
}

/*
 ================================
 Frustum::~Frustum
 ================================
 */
Frustum::~Frustum() {
}


/*
 ================================
 Frustum::Build
 ================================
 */
void Frustum::Build(  const float & nearz,
                            const float & farz,
                            const float & fovy_degrees,
                            const float & screenWidth,
                            const float & screenHeight,
                            const Vec3 & camPos,
                            const Vec3 & camUp,
                            const Vec3 & camLook ) {
    assert( fovy_degrees > 0 );
    assert( screenWidth > 0 );
    assert( screenHeight > 0 );
    Plane & nearPlane = m_planes[ 0 ];
    Plane & farPlane  = m_planes[ 1 ];
    Plane & top       = m_planes[ 2 ];
    Plane & bottom    = m_planes[ 3 ];
    Plane & right     = m_planes[ 4 ];
    Plane & left      = m_planes[ 5 ];
    const float aspect  = screenWidth / screenHeight;
    
    nearPlane   = Plane( camPos + camLook * nearz, camLook * -1.0f );
    farPlane    = Plane( camPos + camLook * farz, camLook );

	const Vec3 camDown	= camUp * -1.0f;
	const Vec3 camRight	= ( camLook.Cross( camUp ) ).Normalize();
	const Vec3 camLeft	= camRight * -1.0f;

	Vec3 fwdUp		= camLook;
	Vec3	fwdDown		= camLook;
	Vec3 fwdRight	= camLook;
	Vec3 fwdLeft		= camLook;

    //
    // calculate the top and bottom planes
    //
    {
		Quat quat;
		Mat3 matRot;
        const float halfAngleY = fovy_degrees * 0.5f;

		quat = Quat( camRight, halfAngleY );
		matRot = quat.ToMat3();
        //matRot.SetRotationMatrix( camRight, halfAngleY );
		fwdUp = matRot * camLook;
        const Vec3 up = matRot * camUp;
        top = Plane( camPos, up );
        
		quat = Quat( camRight, -halfAngleY );
		matRot = quat.ToMat3();
        //matRot.SetRotationMatrix( camRight, -halfAngleY );
		fwdDown = matRot * camLook;
        const Vec3 down = matRot * camDown;
        bottom = Plane( camPos, down );
    }
    
    //
    // calculate the right and left planes
    //
    {
		Quat quat;
		Mat3 matRot;
        const float halfAngle = fovy_degrees * 0.5f * aspect;
        
		quat = Quat( camUp, halfAngle );
		matRot = quat.ToMat3();
        //matRot.SetRotationMatrix( camUp, halfAngle );
		fwdLeft = matRot * camLook;
        const Vec3 leftNorm = matRot * camLeft;
        left = Plane( camPos, leftNorm );
        
		quat = Quat( camUp, -halfAngle );
		matRot = quat.ToMat3();
        //matRot.SetRotationMatrix( camUp, -halfAngle );
		fwdRight = matRot * camLook;
        const Vec3 rightNorm = matRot * camRight;
        right = Plane( camPos, rightNorm );
    }

	//
	//	Calculate corner points
	//	Use the fwd rays, intersecting with near/far planes to calculate
	//
	float t = 0.0f;
	nearPlane.IntersectRay(	camPos, fwdLeft + fwdUp, t, m_corners[ 0 ] );
	farPlane.IntersectRay(	camPos, fwdLeft + fwdUp, t, m_corners[ 1 ] );

	nearPlane.IntersectRay(	camPos, fwdRight + fwdUp, t, m_corners[ 2 ] );
	farPlane.IntersectRay(	camPos, fwdRight + fwdUp, t, m_corners[ 3 ] );

	nearPlane.IntersectRay(	camPos, fwdRight + fwdDown, t, m_corners[ 4 ] );
	farPlane.IntersectRay(	camPos, fwdRight + fwdDown, t, m_corners[ 5 ] );

	nearPlane.IntersectRay(	camPos, fwdLeft + fwdDown, t, m_corners[ 6 ] );
	farPlane.IntersectRay(	camPos, fwdLeft + fwdDown, t, m_corners[ 7 ] );

	// Expand the bounding box
	Bounds bounds;
	for ( int i = 0; i < 8; ++i ) {
		bounds.Expand( m_corners[ i ] );
	}
	m_bounds = bounds;
}

/*
 ================================
 Frustum::MoveNearPlane
 ================================
 */
void Frustum::MoveNearPlane( const float & nearz, const Vec3 & camPos, const Vec3 & camLook ) {
    Plane & nearPlane = m_planes[0];
    nearPlane           = Plane( camPos + camLook * nearz, camLook * -1.0f );
}

/*
 ================================
 Frustum::Build
 ================================
 */
void Frustum::Build( const float * mat ) {
#if 1
	Build( Mat4( mat ) );
#else
	// TODO:   Get this actually working!
    float a = 0;
    float b = 0;
    float c = 0;
    float d = 0;
    
    // Right plane
    a = mat[ 3] - mat[ 0];
    b = mat[ 7] - mat[ 4];
    c = mat[11] - mat[ 8];
    d = mat[15] - mat[12];
    const Plane right     = Plane( a, b, c, d );
    
    // Left plane
    a = mat[ 3] + mat[ 0];
    b = mat[ 7] + mat[ 4];
    c = mat[11] + mat[ 8];
    d = mat[15] + mat[12];
    const Plane left      = Plane( a, b, c, d );
    
    // Bottom plane
    a = mat[ 3] + mat[ 1];
    b = mat[ 7] + mat[ 5];
    c = mat[11] + mat[ 9];
    d = mat[15] + mat[13];
    const Plane bottom    = Plane( a, b, c, d );
    
    // Top plane
    a = mat[ 3] - mat[ 1];
    b = mat[ 7] - mat[ 5];
    c = mat[11] - mat[ 9];
    d = mat[15] - mat[13];
    const Plane top       = Plane( a, b, c, d );
    
    // Near plane
    a = mat[ 3] + mat[ 2];
    b = mat[ 7] + mat[ 6];
    c = mat[11] + mat[10];
    d = mat[15] + mat[14];
    const Plane nearz      = Plane( a, b, c, d );
    
    // Far plane
    a = mat[ 3] - mat[ 2];
    b = mat[ 7] - mat[ 6];
    c = mat[11] - mat[10];
    d = mat[15] - mat[14];
    const Plane farz       = Plane( a, b, c, d );
    
    // copy it over
    m_planes[0] = nearz;
    m_planes[1] = farz;
    m_planes[2] = top;
    m_planes[3] = bottom;
    m_planes[4] = right;
    m_planes[5] = left;
#endif
}

/*
 ================================
 Frustum::Build
 ================================
 */
void Frustum::Build( const float * proj, const float * view ) {
    Mat4 matViewProj;
	myMatrixMultiply( view, proj, matViewProj.ToPtr() );
    
    Build( matViewProj );
}

/*
 ================================
 Frustum::CalculateSideNormal
 ================================
 */
Vec3 Frustum::CalculateSideNormal( const Vec4 & sideNear,
											const Vec4 & sideFar,
											const Vec4 & ptNear ) {
	const Vec3 sideNearToFar	= sideFar.xyz() - sideNear.xyz();
	const Vec3 sideToNear	= sideNear.xyz() - ptNear.xyz();
	const Vec3 sideTangent	= sideToNear.Cross( sideNearToFar );
	const Vec3 normRight		= ( sideNearToFar.Cross( sideTangent ) ).Normalize();
	return normRight;
}

/*
 ================================
 Frustum::ApplyInverseProject
 ================================
 */
Vec4 Frustum::ApplyInverseProject( const Mat4 & matProjInv, const Vec4 & pt ) {
	Vec4 invPt;
	myTransformVector4D( matProjInv.ToPtr(), pt.ToPtr(), invPt.ToPtr() );
	if ( invPt.w > 0.000001f ) {
		invPt /= invPt.w;
	}
	return invPt;
}

/*
 ================================
 Frustum::Build
 ================================
 */
void Frustum::Build( const Mat4 & matViewProj ) {
	Plane & nearPlane		= m_planes[ 0 ];
    Plane & farPlane		= m_planes[ 1 ];
    Plane & topPlane		= m_planes[ 2 ];
    Plane & bottomPlane	= m_planes[ 3 ];
    Plane & rightPlane	= m_planes[ 4 ];
    Plane & leftPlane		= m_planes[ 5 ];

	// Get the middle points of the planes, in normalized device coordinates
	// With exception of the side planes, those need to be at the near plane.
	// The near plane gives us the greatest accuracy for calculating the
	// points on the side planes.
	// We still calculate the far plane side points, in order to accurately
	// calculate 
	const Vec4 ptLeftNear	= Vec4( -1, 0,-1, 1 );
	const Vec4 ptRightNear	= Vec4(  1, 0,-1, 1 );
	const Vec4 ptBottomNear	= Vec4(  0,-1,-1, 1 );
	const Vec4 ptTopNear		= Vec4(  0, 1,-1, 1 );
	const Vec4 ptLeftFar		= Vec4( -1, 0, 1, 1 );
	const Vec4 ptRightFar	= Vec4(  1, 0, 1, 1 );
	const Vec4 ptBottomFar	= Vec4(  0,-1, 1, 1 );
	const Vec4 ptTopFar		= Vec4(  0, 1, 1, 1 );
	const Vec4 ptNear		= Vec4(  0, 0,-1, 1 );
	const Vec4 ptFar			= Vec4(  0, 0, 1, 1 );
	
	// Get the inverse of the view projection matrix
	Mat4 matInverse;
	myMatrixInverse4x4( matViewProj.ToPtr(), matInverse.ToPtr() );

	// Transform the planar points from normalized device coordinates to world space coordinates
	const Vec4 invLeftNear	= ApplyInverseProject( matInverse, ptLeftNear );
	const Vec4 invRightNear	= ApplyInverseProject( matInverse, ptRightNear );
	const Vec4 invBottomNear	= ApplyInverseProject( matInverse, ptBottomNear );
	const Vec4 invTopNear	= ApplyInverseProject( matInverse, ptTopNear );
	const Vec4 invLeftFar	= ApplyInverseProject( matInverse, ptLeftFar );
	const Vec4 invRightFar	= ApplyInverseProject( matInverse, ptRightFar );
	const Vec4 invBottomFar	= ApplyInverseProject( matInverse, ptBottomFar );
	const Vec4 invTopFar		= ApplyInverseProject( matInverse, ptTopFar );
	const Vec4 invNear		= ApplyInverseProject( matInverse, ptNear );
	const Vec4 invFar		= ApplyInverseProject( matInverse, ptFar );

	// Calculate the normals from the points on the planes
	Vec3 normFar = invFar.xyz() - invNear.xyz();
	normFar.Normalize();

	// Calculate side normals
	const Vec3 normRight		= CalculateSideNormal( invRightNear, invRightFar, invNear );
	const Vec3 normLeft		= CalculateSideNormal( invLeftNear, invLeftFar, invNear );
	const Vec3 normTop		= CalculateSideNormal( invTopNear, invTopFar, invNear );
	const Vec3 normBottom	= CalculateSideNormal( invBottomNear, invBottomFar, invNear );

	// Store out the planes
	leftPlane	= Plane( invLeftNear.xyz(),	normLeft );
	rightPlane	= Plane( invRightNear.xyz(),	normRight );
	bottomPlane	= Plane( invBottomNear.xyz(),	normBottom );
	topPlane	= Plane( invTopNear.xyz(),	normTop );
	nearPlane	= Plane( invNear.xyz(),		normFar * -1.0f );
	farPlane	= Plane( invFar.xyz(),		normFar );

	//
	// Calculate the bounding box of the view frustum
	//
	Vec4 corners[ 8 ];
	corners[ 0 ] = Vec4( 1, 1, 1, 1 );
	corners[ 1 ] = Vec4( 1, 1,-1, 1 );
	corners[ 2 ] = Vec4( 1,-1, 1, 1 );
	corners[ 3 ] = Vec4(-1, 1, 1, 1 );
	corners[ 4 ] = Vec4(-1,-1,-1, 1 );
	corners[ 5 ] = Vec4(-1,-1, 1, 1 );
	corners[ 6 ] = Vec4(-1, 1,-1, 1 );
	corners[ 7 ] = Vec4( 1,-1,-1, 1 );

	Bounds box;
	for ( int i = 0; i < 8; ++i ) {
		Vec4 corner = ApplyInverseProject( matInverse, corners[ i ] );
		m_corners[ i ] = corner.xyz();

		box.Expand( corner.xyz() );
	}
	m_bounds = box;
}

/*
 ================================
 Frustum::IsSphereInFrustum
 ================================
 */
// bool Frustum::IsSphereInFrustum( const Sphere & sphere ) const {
//     // If the normals of all the planes point inward, then any sphere behind any plane
//     // is not in the frustum at all
//     
//     for ( int i = 0; i < 6; ++i ) {
//         //        if ( IsSphereBehindPlane( sphere, frustum.planes[i] ) ) {
//         if ( IsSphereInFrontOfPlane( sphere, mPlanes[i] ) ) {
//             return false;
//         }
//     }
//     return true;
// }

/*
 ================================
 Frustum::IsSphereInFrustum
 ================================
 */
bool Frustum::IsBoxInFrustum( const Bounds & box ) const {
#if 1
    Vec3 min;
    Vec3 max;
    
    for ( int i = 0; i < 6; ++i ) {
		const Vec3 & normal = m_planes[ i ].mNormal;

        for ( int j = 0; j < 3; ++j ) {
            if ( normal[ j ] > 0 ) {
                min[ j ] = box.mins[ j ];
                max[ j ] = box.maxs[ j ];
            } else {
                min[ j ] = box.maxs[ j ];
                max[ j ] = box.mins[ j ];
            }
        }
        if ( normal.Dot( min ) + m_planes[ i ].mD > 0 ) {
            // entirely outside of frustum
            return false;
        }
        if ( normal.Dot( max ) + m_planes[ i ].mD >= 0 ) {
            // not fully inside of frustum, if it is inside (intersecting)
            // We should set the return value to INTERSECT
        }
    }
    return true;
#else
	Vec3 pts[ 8 ];
	pts[ 0 ] = Vec3( box.mins.x, box.mins.y, box.mins.z );
	pts[ 1 ] = Vec3( box.mins.x, box.mins.y, box.maxs.z );
	pts[ 2 ] = Vec3( box.mins.x, box.maxs.y, box.mins.z );
	pts[ 3 ] = Vec3( box.mins.x, box.maxs.y, box.maxs.z );

	pts[ 4 ] = Vec3( box.maxs.x, box.mins.y, box.mins.z );
	pts[ 5 ] = Vec3( box.maxs.x, box.mins.y, box.maxs.z );
	pts[ 6 ] = Vec3( box.maxs.x, box.maxs.y, box.mins.z );
	pts[ 7 ] = Vec3( box.maxs.x, box.maxs.y, box.maxs.z );

	// If any of the points are fully behind all the planes,
	// then the box intersects the frustum
	for ( int i = 0; i < 8; ++i ) {
		const Vec3 & pt = pts[ i ];

		for ( int j = 0; j < 6; ++j ) {
			const Plane & plane = m_planes[ j ];

			// get a ray from the planar point to the box pt
			const Vec3 ray = pt - plane.mPlanarPoint;

			if ( ray.Dot( plane.mNormal ) > 0 ) {
				// this pt is in front of the plane (outside)
				break;
			} else if ( 5 == j ) {
				// The pt turned out to be behind all planes (inside)
				// which means that the box definitely intersects the frustum
				return true;
			}
		}
	}

	// Note:	the above test is not perfect.  It is still possible that all points
	//			are outside of the frustum, yet the box could still intersect the
	//			with the frustum.

	return false;
#endif
}

/*
 ================================
 Frustum::IntersectBox
 ================================
 */
bool Frustum::IntersectBox( const Bounds & box ) const {
	if ( !m_bounds.DoesIntersect( box ) ) {
		return false;
	}
#if 1
	Vec3 pts[ 8 ];
	pts[ 0 ] = Vec3( box.mins.x, box.mins.y, box.mins.z );
	pts[ 1 ] = Vec3( box.mins.x, box.mins.y, box.maxs.z );
	pts[ 2 ] = Vec3( box.mins.x, box.maxs.y, box.mins.z );
	pts[ 3 ] = Vec3( box.mins.x, box.maxs.y, box.maxs.z );

	pts[ 4 ] = Vec3( box.maxs.x, box.mins.y, box.mins.z );
	pts[ 5 ] = Vec3( box.maxs.x, box.mins.y, box.maxs.z );
	pts[ 6 ] = Vec3( box.maxs.x, box.maxs.y, box.mins.z );
	pts[ 7 ] = Vec3( box.maxs.x, box.maxs.y, box.maxs.z );

	// If all of the points are fully outside each plane, then no intersection
	for ( int j = 0; j < 6; ++j ) {
		const Plane & plane = m_planes[ j ];

		for ( int i = 0; i < 8; ++i ) {
			const Vec3 & pt = pts[ i ];

			if ( plane.DistanceFromPlane( pt ) < 0.0f ) {
				break;
			}

			if ( 7 == i ) {
				return false;
			}
		}
	}

	Plane boxPlanes[ 6 ];
	boxPlanes[ 0 ] = Plane( box.maxs, Vec3( 1, 0, 0 ) );
	boxPlanes[ 1 ] = Plane( box.maxs, Vec3( 0, 1, 0 ) );
	boxPlanes[ 2 ] = Plane( box.maxs, Vec3( 0, 0, 1 ) );

	boxPlanes[ 3 ] = Plane( box.mins, Vec3(-1, 0, 0 ) );
	boxPlanes[ 4 ] = Plane( box.mins, Vec3( 0,-1, 0 ) );
	boxPlanes[ 5 ] = Plane( box.mins, Vec3( 0, 0,-1 ) );

	// If all of the points are fully outside each plane, then no intersection
	for ( int j = 0; j < 6; ++j ) {
		const Plane & plane = boxPlanes[ j ];

		for ( int i = 0; i < 8; ++i ) {
			const Vec3 & pt = m_corners[ i ];

			if ( plane.DistanceFromPlane( pt ) < 0.0f ) {
				break;
			}

			if ( 7 == i ) {
				return false;
			}
		}
	}

	struct edge_t {
		edge_t() {}
		edge_t( Vec3 a, Vec3 b ) { mFirst = a; mSecond = b; }
		Vec3 mFirst;
		Vec3 mSecond;
	};

	edge_t viewEdges[ 12 ];
	viewEdges[ 0 ] = edge_t( m_corners[ 0 ], m_corners[ 2 ] );
	viewEdges[ 1 ] = edge_t( m_corners[ 2 ], m_corners[ 4 ] );
	viewEdges[ 2 ] = edge_t( m_corners[ 4 ], m_corners[ 6 ] );
	viewEdges[ 3 ] = edge_t( m_corners[ 6 ], m_corners[ 0 ] );

	viewEdges[ 4 ] = edge_t( m_corners[ 1 ], m_corners[ 3 ] );
	viewEdges[ 5 ] = edge_t( m_corners[ 3 ], m_corners[ 5 ] );
	viewEdges[ 6 ] = edge_t( m_corners[ 5 ], m_corners[ 7 ] );
	viewEdges[ 7 ] = edge_t( m_corners[ 7 ], m_corners[ 1 ] );

	viewEdges[ 8 ] = edge_t( m_corners[ 0 ], m_corners[ 1 ] );
	viewEdges[ 9 ] = edge_t( m_corners[ 2 ], m_corners[ 3 ] );
	viewEdges[10 ] = edge_t( m_corners[ 4 ], m_corners[ 5 ] );
	viewEdges[11 ] = edge_t( m_corners[ 6 ], m_corners[ 7 ] );

	edge_t boxEdges[ 12 ];
	boxEdges[ 0 ] = edge_t( pts[ 0 ], pts[ 1 ] );
	boxEdges[ 1 ] = edge_t( pts[ 1 ], pts[ 3 ] );
	boxEdges[ 2 ] = edge_t( pts[ 3 ], pts[ 2 ] );
	boxEdges[ 3 ] = edge_t( pts[ 2 ], pts[ 0 ] );

	boxEdges[ 4 ] = edge_t( pts[ 4 ], pts[ 5 ] );
	boxEdges[ 5 ] = edge_t( pts[ 5 ], pts[ 7 ] );
	boxEdges[ 6 ] = edge_t( pts[ 7 ], pts[ 6 ] );
	boxEdges[ 7 ] = edge_t( pts[ 6 ], pts[ 4 ] );

	boxEdges[ 8 ] = edge_t( pts[ 0 ], pts[ 4 ] );
	boxEdges[ 9 ] = edge_t( pts[ 1 ], pts[ 5 ] );
	boxEdges[10 ] = edge_t( pts[ 2 ], pts[ 6 ] );
	boxEdges[11 ] = edge_t( pts[ 3 ], pts[ 7 ] );

	// Test cross product of pairs of edges, one from each polyhedron.
	for ( int i = 0; i < 12; ++i ) {
		const edge_t & edgeA = viewEdges[ i ];
		const Vec3 rayA = ( edgeA.mSecond - edgeA.mFirst ).Normalize();

		for ( int j = 0; j < 12; ++j ) {
			const edge_t & edgeB = boxEdges[ j ];
			const Vec3 rayB = ( edgeB.mSecond - edgeB.mFirst ).Normalize();

			// Create a separating axis
			const Vec3 norm = ( rayA.Cross( rayB ) ).Normalize();
			const Plane plane( edgeA.mFirst, norm );
			
			// Not a separating axis if the points straddle
			const Plane::planeSide_t sideA = plane.SideDistribution( m_corners, 8 );
			if ( Plane::PS_STRADDLE == sideA ) {
				continue;
			}
			
			// Not a separating axis if the points straddle
			const Plane::planeSide_t sideB = plane.SideDistribution( pts, 8 );
			if ( Plane::PS_STRADDLE == sideB ) {
				continue;
			}

			// If the points of polyA and polyB are on entirely separate sides of the plane, then no intersection
			if ( Plane::PS_NEGATIVE == sideA && Plane::PS_POSITIVE == sideB ) {
				return false;
			}
			
			// If the points of polyA and polyB are on entirely separate sides of the plane, then no intersection
			if ( Plane::PS_NEGATIVE == sideB && Plane::PS_POSITIVE == sideA ) {
				return false;
			}
		}
	}
#endif
	return true;
}

/*
 ================================
 Frustum::IntersectSphere
 ================================
 */
// bool Frustum::IntersectSphere( const Sphere & sphere ) const {
// 	// If the sphere is fully outside any plane, it doesn't intersect
// 	for ( int i = 0; i < 6; ++i ) {
// 		const Plane & plane = mPlanes[ i ];
// 
// 		const float distance = plane.DistanceFromPlane( sphere.mCenter );
// 		if ( distance > sphere.mRadius ) {
// 			return false;
// 		}
// 	}
// 
// 	// It's still possible that the sphere doesn't intersect.
// 	// But for right now, this is good enough.
// 	return true;
// }

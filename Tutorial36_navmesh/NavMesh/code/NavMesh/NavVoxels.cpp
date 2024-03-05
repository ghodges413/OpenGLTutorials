//
//  NavVoxels.cpp
//
#include "BSP/Map.h"
#include "Math/Matrix.h"
#include "Math/Bounds.h"
#include "Math/Plane.h"
#include "Graphics/Graphics.h"
#include "Graphics/TextureManager.h"
#include "Graphics/ShaderManager.h"
#include "NavMesh/NavMesh.h"
#include <math.h>
#include <algorithm>
#include <stdlib.h>
#include <assert.h>
#include <vector>

#define VOXEL_RES 1.0f	// In meters

struct navVoxel_t {
	Vec3 pos;	// This is the sample position (not snapped to grid
	Vec3 normal;

	Vec3 posGrid;	// This is the voxel position, snapped to the grid

	int x, y, z;	// The position in the voxel volume (probably not necessary)

	Bounds bounds;	// Used by the voxelizing brushes method
};

static std::vector< navVoxel_t > s_navVoxelsDraw;	// only for debug purposes

static std::vector< navVoxel_t > s_navVoxelsRaw;
static std::vector< navVoxel_t > s_navVoxels;
std::vector< winding_t > s_navWindings;

std::vector< vert_t > s_navVoxelsVerts;
std::vector< int > s_navVoxelsIndices;


static VertexBufferObject s_navVoxelsVBO;
static VertexArrayObject s_navVoxelsVAO;
static VertexBufferObject s_navVoxelsIBO;

#define NAV_VOXEL_DRAW 0


/*
================================================================

NavVoxels

================================================================
*/


/*
================================
Determinant2d
================================
*/
float Determinant2d( Vec2 a, Vec2 b ) {
	return a.x * b.y - a.y * b.x;
}

/*
================================
BarycentricCoord
================================
*/
Vec3 BarycentricCoord( Vec2 pt, Vec2 a, Vec2 b, Vec2 c ) {
	Vec2 ab = b - a;
	Vec2 bc = c - b;
	Vec2 ca = a - c;

	float area = Determinant2d( ca, ab );

	Vec3 bary;
	bary.x = Determinant2d( bc, ( pt - b ) ) / area;
	bary.y = Determinant2d( ca, ( pt - c ) ) / area;
	bary.z = Determinant2d( ab, ( pt - a ) ) / area;
	return bary;
}

/*
================================
IsPointInsideTriangle
================================
*/
bool IsPointInsideTriangle( const Vec2 & pt, const Vec2 & a, const Vec2 & b, const Vec2 & c ) {
	Vec2 ab = b - a;
	Vec2 bc = c - b;
	Vec2 ca = a - c;

	float det0 = Determinant2d( ab, ( pt - a ) );
	float det1 = Determinant2d( bc, ( pt - b ) );
	float det2 = Determinant2d( ca, ( pt - c ) );

	if ( det0 >= 0 && det1 >= 0 && det2 >= 0 ) {
		return true;
	}
	if ( det0 <= 0 && det1 <= 0 && det2 <= 0 ) {
		return true;
	}
	return false;
}

bool DoesSegmentBoundsIntersect( const Vec2 & mins, const Vec2 & maxs, const Vec2 & a, const Vec2 & b ) {
	Vec2 dir = b - a;

	float tmin = 1;
	float tmax = 0;
	for ( int axis = 0; axis < 2; axis++ ) {
		float invD = 1.0f / dir[ axis ];
		float t0 = ( mins[ axis ] - a[ axis ] ) * invD;
		float t1 = ( maxs[ axis ] - a[ axis ] ) * invD;
		if ( invD < 0.0f ) {
			std::swap( t0, t1 );
		}

		tmin = t0 > tmin ? t0 : tmin;
		tmax = t1 < tmax ? t1 : tmax;
		if ( tmax <= tmin ) {
			return false;
		}
	}
	return true;
}

bool DoesVoxelIntersectTriangle( const Vec2 & mins, const Vec2 & maxs, const Vec2 & a, const Vec2 & b, const Vec2 & c ) {
	Vec2 center = ( mins + maxs ) * 0.5f;
	if ( IsPointInsideTriangle( center, a, b, c ) ) {
		return true;
	}
	if ( DoesSegmentBoundsIntersect( mins, maxs, a, b ) ) {
		return true;
	}
	if ( DoesSegmentBoundsIntersect( mins, maxs, b, c ) ) {
		return true;
	}
	if ( DoesSegmentBoundsIntersect( mins, maxs, c, a ) ) {
		return true;
	}
	return false;
}

void RasterizeTriangle( Vec3 a3, Vec3 b3, Vec3 c3, Vec3 norm ) {
	// Default project along z
	int axis = 2;
	Vec2 a = Vec2( a3.x , a3.y );
	Vec2 b = Vec2( b3.x , b3.y );
	Vec2 c = Vec2( c3.x , c3.y );
	if ( fabsf( norm.x ) > fabsf( norm.y ) && fabsf( norm.x ) > fabsf( norm.z ) ) {
		// If the normal points mostly x, project along x
		axis = 0;
		a = Vec2( a3.y , a3.z );
		b = Vec2( b3.y , b3.z );
		c = Vec2( c3.y , c3.z );
	}
	if ( fabsf( norm.y ) > fabsf( norm.x ) && fabsf( norm.y ) > fabsf( norm.z ) ) {
		// If the normal points mostly y, project along y
		axis = 1;
		a = Vec2( a3.x , a3.z );
		b = Vec2( b3.x , b3.z );
		c = Vec2( c3.x , c3.z );
	}

	Bounds bounds;
	bounds.Expand( Vec3( a.x, a.y, 0 ) );
	bounds.Expand( Vec3( b.x, b.y, 0 ) );
	bounds.Expand( Vec3( c.x, c.y, 0 ) );
	bounds.mins.x = floorf( bounds.mins.x );
	bounds.mins.y = floorf( bounds.mins.y );
	bounds.maxs.x = ceilf( bounds.maxs.x );
	bounds.maxs.y = ceilf( bounds.maxs.y );
	Vec2 mins = Vec2( bounds.mins.x, bounds.mins.y ) - Vec2( VOXEL_RES, VOXEL_RES ) * 0.5f;
	Vec2 maxs = Vec2( bounds.maxs.x, bounds.maxs.y ) + Vec2( VOXEL_RES, VOXEL_RES ) * 0.5f;

	for ( float y = mins.y; y < maxs.y; y += VOXEL_RES ) {
		for ( float x = mins.x; x < maxs.x; x += VOXEL_RES ) {
			Vec2 voxelMin = Vec2( x, y );
			Vec2 voxelMax = Vec2( x, y );
			Vec2 voxelCenter = ( voxelMin + voxelMax ) * 0.5f;
			if ( !IsPointInsideTriangle( voxelCenter, a, b, c ) ) {
				continue;
			}

// 			if ( !DoesVoxelIntersectTriangle( voxelMin, voxelMax, a, b, c ) ) {
// 				continue;
// 			}

			Vec3 bary = BarycentricCoord( voxelCenter, a, b, c );
			
			navVoxel_t voxel;
			voxel.pos = a3 * bary.x + b3 * bary.y + c3 * bary.z;
			voxel.normal = norm;

			voxel.posGrid = voxel.pos;
			voxel.posGrid[ axis ] /= VOXEL_RES;
			voxel.posGrid[ axis ] = floorf( voxel.posGrid[ axis ] );
			voxel.posGrid[ axis ] += 0.5f;
			voxel.posGrid[ axis ] *= VOXEL_RES;
			if ( voxel.normal[ axis ] < 0 ) {
				voxel.posGrid = voxel.pos;
				voxel.posGrid[ axis ] /= VOXEL_RES;
				voxel.posGrid[ axis ] = ceilf( voxel.posGrid[ axis ] );
				voxel.posGrid[ axis ] -= 0.5f;
				voxel.posGrid[ axis ] *= VOXEL_RES;
			}

			s_navVoxelsRaw.push_back( voxel );
		}
	}
}

/*
================================
BuildByRasterizationTriangles
================================
*/
void BuildByRasterizationTriangles() {
	// First we need to build a bounding brush around the entire map (fortunately, our map is small)
	// We will use the bounds to create the "FrameBuffer" for rendering
	// Nah, we don't need to do that.  Just create the bounds about the brush, and create
	// on the fly framebuffers per triangle.
	// Maybe that's a bad idea too... it's possible, in theory to have one large triangle spanning the map.
	// We're over thinking this... just do the one giant frame buffer

	//
	//	Build the bounds of the whole map
	//

	//
	//	Rasterize triangles into voxels
	//
	for ( int b = 0; b < g_brushes.size(); b++ ) {
		const brush_t & brush = g_brushes[ b ];

		for ( int w = 0; w < brush.numPlanes; w++ ) {
			const winding_t & winding = brush.windings[ w ];
			if ( winding.pts.size() < 3 ) {
				continue;
			}

			// Triangle fan indices for the winding
			for ( int v = 2; v < winding.pts.size(); v++ ) {
				Vec3 a = winding.pts[ 0 ];
				Vec3 b = winding.pts[ v - 1 ];
				Vec3 c = winding.pts[ v ];
				RasterizeTriangle( a, b, c, brush.planes[ w ].normal );
			}
		}
	}

	//
	//	Copy over the voxels that we want to keep (non-forbidden voxels)
	//
	for ( int i = 0; i < s_navVoxelsRaw.size(); i++ ) {
		const navVoxel_t & voxel = s_navVoxelsRaw[ i ];

		if ( voxel.normal.z < 0.5f ) {
			// Don't keep voxels that npc's can't walk on
			continue;
		}

#if 0
		// Don't keep voxels that overlap forbidden voxels
		bool isForbidden = false;
		for ( int j = 0; j < s_navVoxelsRaw.size(); j++ ) {
			if ( i == j ) {
				continue;
			}

			const navVoxel_t & voxelB = s_navVoxelsRaw[ j ];
			if ( voxelB.normal.z >= 0.5f ) {
				continue;
			}

			Vec3 ab = voxelB.pos - voxel.pos;
			if ( ab.GetMagnitude() < VOXEL_RES ) {
				isForbidden = true;
				break;
			}
		}
		if ( isForbidden ) {
			continue;
		}
#endif
		s_navVoxels.push_back( voxel );
	}
}

bool IsPointInBrush( const brush_t & brush, const Vec3 & pt ) {
	if ( !brush.bounds.DoesIntersect( pt ) ) {
		return false;
	}

	for ( int p = 0; p < brush.numPlanes; p++ ) {
		const plane_t & plane = brush.planes[ p ];
		const winding_t & winding = brush.windings[ p ];

		Vec3 ab = winding.pts[ 1 ] - winding.pts[ 0 ];
		Vec3 ac = winding.pts[ 2 ] - winding.pts[ 0 ];
		Vec3 planeToPt = pt - winding.pts[ 0 ];
		Vec3 normal = ab.Cross( ac );
		
		planeToPt = pt - plane.pts[ 0 ];
		normal = plane.normal;

		if ( normal.Dot( planeToPt ) > 0.0f ) {
			return false;
		}
	}

	return true;
}

bool SegmentPlaneIntersection( const plane_t & plane, const Vec3 & a, const Vec3 & b, Vec3 & pt ) {
	Vec3 pa = a - plane.pts[ 0 ];
	Vec3 pb = b - plane.pts[ 0 ];
	float ta = plane.normal.Dot( pa );
	float tb = plane.normal.Dot( pb );

	if ( ta > 0 && tb > 0 ) {
		return false;
	}
	if ( ta < 0 && tb < 0 ) {
		return false;
	}

	Vec3 ab = b - a;
	float t = fabsf( ta ) / ( fabsf( ta ) + fabsf( tb ) );
	pt = a + ab * t;

	return true;
}

Vec3 WindingNormal( const winding_t & w ) {
	Vec3 a = w.pts[ 0 ];
	Vec3 b = w.pts[ 1 ];
	Vec3 c = w.pts[ 2 ];
	Vec3 norm = ( b - a ).Cross( c - a );
	norm.Normalize();
	return norm;
}

/*
================================
BuildByRasterizationTriangles
================================
*/
void BuildByVoxelizingBrushes() {
	// Build bounds around the whole map
	Bounds bounds;
	for ( int b = 0; b < g_brushes.size(); b++ ) {
		const brush_t & brush = g_brushes[ b ];

		bounds.Expand( brush.bounds );
	}

	Vec3 mins;
	Vec3 maxs;
	for ( int i = 0; i < 3; i++ ) {
		mins[ i ] = floorf( bounds.mins[ i ] );
		maxs[ i ] = ceilf( bounds.maxs[ i ] );
	}
	mins = mins - Vec3( VOXEL_RES * 0.5f );
	maxs = maxs + Vec3( VOXEL_RES * 0.5f );

	//
	//	Loop over the whole space and add voxels that intersect solid geo
	//
	for ( float z = mins.z; z < maxs.z; z += VOXEL_RES ) {
		for ( float y = mins.y; y < maxs.y; y += VOXEL_RES ) {
			for ( float x = mins.x; x < maxs.x; x += VOXEL_RES ) {
				Bounds voxelBounds;
				voxelBounds.mins = Vec3( x, y, z ) - Vec3( VOXEL_RES * 0.5f );
				voxelBounds.maxs = Vec3( x, y, z ) + Vec3( VOXEL_RES * 0.5f );
				voxelBounds.mins.z -= 0.01f;

				for ( int b = 0; b < g_brushes.size(); b++ ) {
					const brush_t & brush = g_brushes[ b ];

					if ( !voxelBounds.DoesIntersect( brush.bounds ) ) {
						continue;
					}

					Vec3 maxs2 = voxelBounds.maxs;
					Vec3 mins2 = voxelBounds.mins;

					Vec3 corners[ 8 ];
					corners[ 0 ] = Vec3( maxs2.x, maxs2.y, maxs2.z );
					corners[ 1 ] = Vec3( mins2.x, maxs2.y, maxs2.z );
					corners[ 2 ] = Vec3( mins2.x, mins2.y, maxs2.z );
					corners[ 3 ] = Vec3( maxs2.x, mins2.y, maxs2.z );

					corners[ 4 ] = Vec3( maxs2.x, maxs2.y, mins2.z );
					corners[ 5 ] = Vec3( mins2.x, maxs2.y, mins2.z );
					corners[ 6 ] = Vec3( mins2.x, mins2.y, mins2.z );
					corners[ 7 ] = Vec3( maxs2.x, mins2.y, mins2.z );

					bool didIntersect = false;
					for ( int i = 0; i < 8; i++ ) {
						if ( IsPointInBrush( brush, corners[ i ] ) ) {
							didIntersect = true;
							break;
						}
					}

					if ( didIntersect ) {
						navVoxel_t voxel;
						voxel.pos = Vec3( x, y, z );
						voxel.normal.Zero();
						voxel.bounds = voxelBounds;

						s_navVoxelsRaw.push_back( voxel );
						break;
					}
				}
			}
		}
	}

#if ( NAV_VOXEL_DRAW == 1 )
	s_navVoxelsDraw = s_navVoxelsRaw;
#endif

	//
	//	Voxels can be vertically stacked.  In a stack, we only want the top voxel.
	//	So copy only the top voxels into the final voxel list
	//
	for ( int i = 0; i < s_navVoxelsRaw.size(); i++ ) {
		navVoxel_t voxel = s_navVoxelsRaw[ i ];

		bool isTopVoxel = true;
		for ( int j = 0; j < s_navVoxelsRaw.size(); j++ ) {
			if ( i == j ) {
				continue;
			}

			const navVoxel_t & voxelB = s_navVoxelsRaw[ j ];
			Vec3 testPt = voxel.pos + Vec3( 0, 0, VOXEL_RES );

			if ( voxelB.bounds.DoesIntersect( testPt ) ) {
				isTopVoxel = false;
				break;
			}
		}

		if ( isTopVoxel ) {
			s_navVoxels.push_back( voxel );
		}
		if ( 0 == ( i % 1000 ) ) {
			printf( "Voxel: %i of %i\n", i, s_navVoxelsRaw.size() );
		}
	}

	printf( "Raw   Voxels: %i\n", s_navVoxelsRaw.size() );
	printf( "Final Voxels: %i\n", s_navVoxels.size() );

	//
	//	With the top voxels, we want to project them onto the brushes beneath them.
	// 	If a voxel has a dangling vertex that can't be projected onto any brushes,
	// 	or if it is projected further than a voxel height away then we need to remove
	// 	that voxel.
	//
	s_navVoxelsRaw.clear();
	for ( int v = 0; v < s_navVoxels.size(); v++ ) {
		const navVoxel_t & voxel = s_navVoxels[ v ];

		Vec3 pts[ 4 ];
		pts[ 0 ] = Vec3( voxel.bounds.maxs.x, voxel.bounds.maxs.y, -1e6 );
		pts[ 1 ] = Vec3( voxel.bounds.mins.x, voxel.bounds.maxs.y, -1e6 );
		pts[ 2 ] = Vec3( voxel.bounds.mins.x, voxel.bounds.mins.y, -1e6 );
		pts[ 3 ] = Vec3( voxel.bounds.maxs.x, voxel.bounds.mins.y, -1e6 );

		for ( int b = 0; b < g_brushes.size(); b++ ) {
			const brush_t & brush = g_brushes[ b ];
			if ( !voxel.bounds.DoesIntersect( brush.bounds ) ) {
				continue;
			}

			if ( b > 0 ) {
				static volatile int poo = 0;
				poo++;
			}

			for ( int w = 0; w < brush.numPlanes; w++ ) {
				const plane_t & plane = brush.planes[ w ];
				const winding_t & winding = brush.windings[ w ];
				if ( plane.normal.z < 0.5f ) {
					// Don't bother projecting onto geo that AI can't path over
					continue;
				}

				
				Vec3 as[ 4 ];
				Vec3 bs[ 4 ];
				const float halfres = VOXEL_RES * 0.5f;
				as[ 0 ] = voxel.pos + Vec3( halfres, halfres, VOXEL_RES * 2.0f );
				as[ 1 ] = voxel.pos + Vec3(-halfres, halfres, VOXEL_RES * 2.0f );
				as[ 2 ] = voxel.pos + Vec3(-halfres,-halfres, VOXEL_RES * 2.0f );
				as[ 3 ] = voxel.pos + Vec3( halfres,-halfres, VOXEL_RES * 2.0f );

				bs[ 0 ] = voxel.pos + Vec3( halfres, halfres,-VOXEL_RES * 2.0f );
				bs[ 1 ] = voxel.pos + Vec3(-halfres, halfres,-VOXEL_RES * 2.0f );
				bs[ 2 ] = voxel.pos + Vec3(-halfres,-halfres,-VOXEL_RES * 2.0f );
				bs[ 3 ] = voxel.pos + Vec3( halfres,-halfres,-VOXEL_RES * 2.0f );

				Vec3 tmp[ 4 ];
				tmp[ 0 ] = pts[ 0 ];
				tmp[ 1 ] = pts[ 1 ];
				tmp[ 2 ] = pts[ 2 ];
				tmp[ 3 ] = pts[ 3 ];
				
				for ( int i = 0; i < 4; i++ ) {
					if ( !SegmentPlaneIntersection( plane, as[ i ], bs[ i ], tmp[ i ] ) ) {
						tmp[ i ].z = -1e6;
					}
				}

				// Check that these points are inside the winding
				for ( int i = 0; i < 4; i++ ) {
					const int num = winding.pts.size();
					for ( int j = 0; j < num; j++ ) {
						Vec3 a = winding.pts[ ( j + 0 ) % num ];
						Vec3 b = winding.pts[ ( j + 1 ) % num ];
						Vec3 c = tmp[ i ];

						Vec3 norm = ( b - a ).Cross( c - a );
						norm.Normalize();

						if ( norm.Dot( plane.normal ) < 0.0f ) {
							// Allow for a tolerance, sometimes a voxel is just on the edge of a winding
							Vec3 ab = b - a;
							ab.Normalize();
							Vec3 ac = c - a;
							Vec3 proj = ab * ac.Dot( ab );
							Vec3 ortho = c - proj;
							if ( ortho.GetMagnitude() > 0.01f ) {
								tmp[ i ].z = -1e6;
								break;
							}
						}
					}
				}

				// If there was a successful projection, copy it over
				for ( int i = 0; i < 4; i++ ) {
					if ( tmp[ i ].z > pts[ i ].z ) {
						pts[ i ] = tmp[ i ];
					}
				}
			}
		}

		bool didProject = true;
		for ( int i = 0; i < 4; i++ ) {
			didProject = didProject && ( pts[ i ].z > -1e6 );
			float delta = pts[ i ].z - voxel.pos.z;
			didProject = didProject && ( fabsf( delta ) < VOXEL_RES * 2.0f );
		}
		if ( !didProject ) {
			continue;
		}

#if ( NAV_VOXEL_DRAW == 3 )
		const int numVerts = s_navVoxelsVerts.size();
		for ( int i = 0; i < 4; i++ ) {
			vert_t vert;
			vert.pos = pts[ i ] + Vec3( 0, 0, 0.05f );
			vert.st = Vec2( 0, 0 );
			Vec3ToByte4_n11( vert.norm, Vec3( 0, 0, 1 ) );
			Vec3ToByte4_n11( vert.tang, Vec3( 1, 0, 0 ) );
			Vec3ToByte4_n11( vert.buff, Vec3( 0.5f, 0.5f, 1.0f ) );
			s_navVoxelsVerts.push_back( vert );
		}
		s_navVoxelsIndices.push_back( numVerts + 0 );
		s_navVoxelsIndices.push_back( numVerts + 1 );
		s_navVoxelsIndices.push_back( numVerts + 2 );

		s_navVoxelsIndices.push_back( numVerts + 0 );
		s_navVoxelsIndices.push_back( numVerts + 2 );
		s_navVoxelsIndices.push_back( numVerts + 3 );
#endif
		// Add this projected voxel
		s_navVoxelsRaw.push_back( voxel );

		// Also build a winding for this voxel
		winding_t navWinding;
		navWinding.pts.push_back( pts[ 0 ] );
		navWinding.pts.push_back( pts[ 1 ] );
		navWinding.pts.push_back( pts[ 2 ] );
		navWinding.pts.push_back( pts[ 3 ] );
		s_navWindings.push_back( navWinding );
	}

	s_navVoxels = s_navVoxelsRaw;
#if ( NAV_VOXEL_DRAW == 2 )
	s_navVoxelsDraw = s_navVoxels;
#endif

	std::vector< winding_t > pingpong;

	int numNoMerge = 0;
	int numIters = 0;
	bool didMerge = true;
	while ( numNoMerge < 2 ) {
		const int axis = ( numIters & 1 );
		didMerge = false;
		
		std::vector< winding_t > & source = s_navWindings;
		std::vector< winding_t > & target = pingpong;
		target.clear();
		
		// Merge neighboring windings
		for ( int i = 0; i < source.size(); i++ ) {
			const winding_t & windingA = source[ i ];

			for ( int j = 0; j < source.size(); j++ ) {
				const winding_t & windingB = source[ j ];
				if ( i == j ) {
					continue;
				}				

				Vec3 normA = WindingNormal( windingA );
				Vec3 normB = WindingNormal( windingB );
				if ( normA.Dot( normB ) < 0.99f ) {
					continue;
				}

				Vec3 a0, a1, b0, b1;
				if ( 0 == axis ) {
					// Choose right/left shared edge
					a0 = windingA.pts[ 0 ];
					a1 = windingA.pts[ 3 ];
					b0 = windingB.pts[ 1 ];
					b1 = windingB.pts[ 2 ];
				} else {
					// Choose top/Bottom shared edge
					a0 = windingA.pts[ 2 ];
					a1 = windingA.pts[ 3 ];
					b0 = windingB.pts[ 1 ];
					b1 = windingB.pts[ 0 ];
				}

				Vec3 delta0 = ( a0 - b0 );
				Vec3 delta1 = ( a1 - b1 );
				float d0 = delta0.GetMagnitude();
				float d1 = delta1.GetMagnitude();
				if ( d0 < 0.01f && d1 < 0.01f ) {
					// merge these windings
					didMerge = true;
					winding_t merged;
					if ( 0 == axis ) {
						// merge right/left
						merged.pts.push_back( windingB.pts[ 0 ] );
						merged.pts.push_back( windingA.pts[ 1 ] );
						merged.pts.push_back( windingA.pts[ 2 ] );
						merged.pts.push_back( windingB.pts[ 3 ] );
					} else {
						// merge top/bottom
						merged.pts.push_back( windingA.pts[ 0 ] );
						merged.pts.push_back( windingA.pts[ 1 ] );
						merged.pts.push_back( windingB.pts[ 2 ] );
						merged.pts.push_back( windingB.pts[ 3 ] );
					}

					// add the merged winding to the target list
					target.push_back( merged );

					// Remove these windings from the source list
					if ( i > j ) {
						source.erase( source.begin() + i );
						source.erase( source.begin() + j );
					} else {
						source.erase( source.begin() + j );
						source.erase( source.begin() + i );
					}
					--i;
					--j;
					break;
				}
			}
		}

		// Copy over any windings that didn't get merged
		for ( int i = 0; i < source.size(); i++ ) {
			target.push_back( source[ i ] );
		}
		source.clear();

		// Swap back
		source = target;
		target.clear();

		// Keep count of how many times we merged the whole set
		++numIters;

		if ( !didMerge ) {
			numNoMerge++;
		} else {
			numNoMerge = 0;
		}
	}

	//
	//	Convert the merged windings into render geo
	//
#if ( NAV_VOXEL_DRAW == 4 )
	s_navVoxelsVerts.clear();
	s_navVoxelsIndices.clear();
	for ( int w = 0; w < s_navWindings.size(); w++ ) {
		const winding_t & winding = s_navWindings[ w ];

		const int numVerts = s_navVoxelsVerts.size();
		for ( int i = 0; i < 4; i++ ) {
			vert_t vert;
			vert.pos = winding.pts[ i ] + Vec3( 0, 0, 0.05f );
			vert.st = Vec2( 0, 0 );
			Vec3ToByte4_n11( vert.norm, Vec3( 0, 0, 1 ) );
			Vec3ToByte4_n11( vert.tang, Vec3( 1, 0, 0 ) );
			Vec3ToByte4_n11( vert.buff, Vec3( 0.5f, 0.5f, 1.0f ) );
			s_navVoxelsVerts.push_back( vert );
		}
		s_navVoxelsIndices.push_back( numVerts + 0 );
		s_navVoxelsIndices.push_back( numVerts + 1 );
		s_navVoxelsIndices.push_back( numVerts + 2 );

		s_navVoxelsIndices.push_back( numVerts + 0 );
		s_navVoxelsIndices.push_back( numVerts + 2 );
		s_navVoxelsIndices.push_back( numVerts + 3 );
	}
#endif
}

/*
================================
BuildNavVoxels
================================
*/
void BuildNavVoxels() {
	//BuildByRasterizationTriangles();
	BuildByVoxelizingBrushes();

	//
	// Build voxel render geo for debug rendering
	//
#if ( NAV_VOXEL_DRAW > 0 && NAV_VOXEL_DRAW < 3 )
	s_navVoxelsVerts.clear();
	s_navVoxelsVerts.reserve( s_navVoxelsDraw.size() * 8 );
	s_navVoxelsIndices.clear();
	s_navVoxelsIndices.reserve( s_navVoxelsDraw.size() * 6 * 3 * 2 );
	for ( int v = 0; v < s_navVoxelsDraw.size(); v++ ) {
		Vec3 center = s_navVoxelsDraw[ v ].pos;
		//Vec3 center = s_navVoxels[ v ].posGrid;
		Vec3 pts[ 8 ];
		pts[ 0 ] = center + Vec3( VOXEL_RES, VOXEL_RES, VOXEL_RES ) * 0.5f;
		pts[ 1 ] = center + Vec3(-VOXEL_RES, VOXEL_RES, VOXEL_RES ) * 0.5f;
		pts[ 2 ] = center + Vec3(-VOXEL_RES,-VOXEL_RES, VOXEL_RES ) * 0.5f;
		pts[ 3 ] = center + Vec3( VOXEL_RES,-VOXEL_RES, VOXEL_RES ) * 0.5f;

		pts[ 4 ] = center + Vec3( VOXEL_RES, VOXEL_RES,-VOXEL_RES ) * 0.5f;
		pts[ 5 ] = center + Vec3( VOXEL_RES,-VOXEL_RES,-VOXEL_RES ) * 0.5f;
		pts[ 6 ] = center + Vec3(-VOXEL_RES,-VOXEL_RES,-VOXEL_RES ) * 0.5f;
		pts[ 7 ] = center + Vec3(-VOXEL_RES, VOXEL_RES,-VOXEL_RES ) * 0.5f;

		const int numVerts = s_navVoxelsVerts.size();
		for ( int i = 0; i < 8; i++ ) {
			vert_t vert;
			vert.pos = pts[ i ];
			vert.st = Vec2( 0, 0 );
			Vec3ToByte4_n11( vert.norm, Vec3( 0, 0, 1 ) );
			Vec3ToByte4_n11( vert.tang, Vec3( 1, 0, 0 ) );
			Vec3ToByte4_n11( vert.buff, Vec3( 0.5f, 0.5f, 1.0f ) );
			s_navVoxelsVerts.push_back( vert );
		}

		int indices[ 6 * 3 * 2 ] = {
			// +z
			0, 1, 2,
			0, 2, 3,
			// -z
			4, 5, 6,
			4, 6, 7,
			// +x
			0, 3, 6,
			0, 6, 5,
			// -x
			1, 7, 6,
			1, 6, 2,
			// +y
			0, 4, 7,
			0, 7, 1,
			// -y
			2, 6, 5,
			2, 5, 3
		};
		for ( int i = 0; i < 6 * 3 * 2; i++ ) {
			s_navVoxelsIndices.push_back( indices[ i ] + numVerts );
		}
	}
#endif

#if NAV_VOXEL_DRAW
	const int sizeIBO = s_navVoxelsIndices.size() * sizeof( unsigned int );
	s_navVoxelsIBO.Generate( GL_ELEMENT_ARRAY_BUFFER, sizeIBO, &s_navVoxelsIndices[ 0 ], GL_STATIC_DRAW );

	const int stride = sizeof( vert_t );
	const int sizeVBO = s_navVoxelsVerts.size() * stride;
	s_navVoxelsVBO.Generate( GL_ARRAY_BUFFER, sizeVBO, &s_navVoxelsVerts[ 0 ], GL_DYNAMIC_DRAW );

	s_navVoxelsVAO.Generate();
	s_navVoxelsVAO.Bind();
    
	s_navVoxelsVBO.Bind();
	s_navVoxelsIBO.Bind();

	unsigned long offset = 0;
	glEnableVertexAttribArray( 0 );
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset );
	offset += sizeof( Vec3 );
    
	glEnableVertexAttribArray( 1 );
	glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, stride, (const void *)offset );
	offset += sizeof( Vec2 );
    
	glEnableVertexAttribArray( 2 );
	glVertexAttribPointer( 2, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, (const void *)offset );
	offset += sizeof( unsigned char ) * 4;
    
	glEnableVertexAttribArray( 3 );
	glVertexAttribPointer( 3, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, (const void *)offset );
	offset += sizeof( unsigned char ) * 4;

	glEnableVertexAttribArray( 4 );
	glVertexAttribPointer( 4, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, (const void *)offset );
	offset += sizeof( unsigned char ) * 4;

	s_navVoxelsVAO.UnBind();
	s_navVoxelsVBO.UnBind();
	s_navVoxelsIBO.UnBind();
#endif
}

/*
================================
DrawNavVoxels
================================
*/
void DrawNavVoxels() {
#if NAV_VOXEL_DRAW
	// Bind the VAO
    s_navVoxelsVAO.Bind();
    
    // Draw
	const int num = s_navVoxelsIndices.size();
	glDrawElements( GL_TRIANGLES, num, GL_UNSIGNED_INT, 0 );

	// Unbind the VAO
	s_navVoxelsVAO.UnBind();
#endif
}

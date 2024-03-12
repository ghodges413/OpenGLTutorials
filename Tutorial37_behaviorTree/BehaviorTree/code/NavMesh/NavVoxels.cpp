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
IsPointInBrush
================================
*/
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
		navVoxel_t & voxel = s_navVoxelsRaw[ i ];

		// Check for Top Voxel
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

	printf( "Collaspe Stack Voxels:\n" );
	printf( "Raw   Voxels: %i\n", s_navVoxelsRaw.size() );
	printf( "Final Voxels: %i\n", s_navVoxels.size() );

	
	//
	//	Voxels that are too close to an edge also need to be removed.
	// 	Check the 8 neighboring voxel distances for geometry underneath
	//
	s_navVoxelsRaw = s_navVoxels;
	s_navVoxels.clear();
	for ( int i = 0; i < s_navVoxelsRaw.size(); i++ ) {
		navVoxel_t voxel = s_navVoxelsRaw[ i ];

		bool isEdge = false;
		for ( int y = -1; y < 2; y++ ) {
			for ( int x = -1; x < 2; x++ ) {
				if ( x == 0 && y == 0 ) {
					continue;
				}

				Vec3 offset = Vec3( float( x ) * VOXEL_RES, float( y ) * VOXEL_RES, 0 );
				Vec3 testPt = voxel.pos + offset;
				Vec3 testPt2 = testPt + Vec3( 0, 0, -VOXEL_RES );
				Vec3 testPt3 = testPt2 + Vec3( 0, 0, -VOXEL_RES );

				bool hasIntersection = false;
				for ( int b = 0; b < g_brushes.size(); b++ ) {
					const brush_t & brush = g_brushes[ b ];
					if ( IsPointInBrush( brush, testPt ) ) {
						hasIntersection = true;
						break;
					}
					if ( IsPointInBrush( brush, testPt2 ) ) {
						hasIntersection = true;
						break;
					}
					if ( IsPointInBrush( brush, testPt3 ) ) {
						hasIntersection = true;
						break;
					}
				}

				if ( !hasIntersection ) {
					isEdge = true;
					break;
				}
			}
			if ( isEdge ) {
				break;
			}
		}

		if ( isEdge ) {
			continue;
		}

		s_navVoxels.push_back( voxel );
	}

	printf( "Edge removal Voxels:\n" );
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

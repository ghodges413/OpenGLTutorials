//
//  NavMesh.cpp
//
#include "BSP/Map.h"
#include "Math/Matrix.h"
#include "Math/Bounds.h"
#include "Graphics/Graphics.h"
#include "Graphics/TextureManager.h"
#include "Graphics/ShaderManager.h"
#include "NavMesh/NavMesh.h"
#include "NavMesh/NavVoxels.h"
#include "NavMesh/NavFile.h"
#include <math.h>
#include <algorithm>
#include <stdlib.h>
#include <assert.h>
#include <vector>

std::vector< vert_t > s_navmeshVerts;
std::vector< int > s_navmeshIndices;
std::vector< Bounds > s_navmeshBounds;

static VertexBufferObject s_navmeshVBO;
static VertexArrayObject s_navmeshVAO;
static VertexBufferObject s_navmeshIBO;

MatN g_navAdjacencyMatrix;

/*
================================================================

NavMesh

================================================================
*/

void BuildNavMeshRenderGeo() {
	const int sizeIBO = s_navmeshIndices.size() * sizeof( unsigned int );
	s_navmeshIBO.Generate( GL_ELEMENT_ARRAY_BUFFER, sizeIBO, &s_navmeshIndices[ 0 ], GL_STATIC_DRAW );

	const int stride = sizeof( vert_t );
	const int sizeVBO = s_navmeshVerts.size() * stride;
	s_navmeshVBO.Generate( GL_ARRAY_BUFFER, sizeVBO, &s_navmeshVerts[ 0 ], GL_DYNAMIC_DRAW );

	s_navmeshVAO.Generate();
	s_navmeshVAO.Bind();
    
	s_navmeshVBO.Bind();
	s_navmeshIBO.Bind();

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

	s_navmeshVAO.UnBind();
	s_navmeshVBO.UnBind();
	s_navmeshIBO.UnBind();
}

/*
================================
IsEdgeShared
================================
*/
bool IsEdgeShared( const int edge0A, const int edge0B, const int edge1A, const int edge1B ) {
	assert( edge0A != edge0B );
	assert( edge1A != edge1B );

	if ( edge0A == edge1A && edge0B == edge1B ) {
		return true;
	}
	if ( edge0A == edge1B && edge0B == edge1A ) {
		return true;
	}
	return false;
}

#define USE_NAV_VOXELS
extern std::vector< winding_t > s_navWindings;

/*
================================
LoadNavMesh
================================
*/
void LoadNavMesh() {
	if ( ReadNavFile() ) {
		return;
	}
	// First we build the bounds around the brushes
	// Then voxelize the brushes
	// Then square off contiguous voxels
	// Then triangulate the squared off voxels
	// Build a graph of the triangles?

	// For now, we're going to hack this by just grabbing the upward facing triangles.
	// This allows us to skip the voxelization/retriangulation step.

	std::vector< Vec3 > triPoints;
	std::vector< int > triIndices;

#if defined( USE_NAV_VOXELS )
	BuildNavVoxels();

	for ( int w = 0; w < s_navWindings.size(); w++ ) {
		const winding_t & winding = s_navWindings[ w ];

		int numVerts = triPoints.size();
		for ( int v = 0; v < winding.pts.size(); v++ ) {
			Vec3 pt = winding.pts[ v ];
			triPoints.push_back( pt );
		}

		// Triangle fan indices for the winding
		for ( int v = 2; v < winding.pts.size(); v++ ) {
			triIndices.push_back( numVerts + 0 );
			triIndices.push_back( numVerts + v - 1 );
			triIndices.push_back( numVerts + v );
		}
	}
#else
	// This version uses the raw brushes


	//
	// Convert upward facing brushes into triangles
	//
	for ( int b = 0; b < g_brushes.size(); b++ ) {
		const brush_t & brush = g_brushes[ b ];

		for ( int w = 0; w < brush.numPlanes; w++ ) {
			// Only accept upward facing windings
			Vec3 normal = brush.planes[ w ].normal;
			if ( normal.z < 0.025f ) {
				continue;
			}

			const winding_t & winding = brush.windings[ w ];
			if ( winding.pts.size() < 3 ) {
				continue;
			}

			if ( winding.pts[ 0 ].z > 10.0f ) {
				continue;
			}

			int numVerts = triPoints.size();
			for ( int v = 0; v < winding.pts.size(); v++ ) {
				Vec3 pt = winding.pts[ v ];
				triPoints.push_back( pt );
			}

			// Triangle fan indices for the winding
			for ( int v = 2; v < winding.pts.size(); v++ ) {
				triIndices.push_back( numVerts + 0 );
				triIndices.push_back( numVerts + v - 1 );
				triIndices.push_back( numVerts + v );
			}
		}
	}
#endif

	// Now we need to merge neighboring vertices and re-index triangle indices
	// We're also going to do this brute force.  This would be super slow on real world
	// geometry.  But since we're using a small test map, this should be fine.
	// But for a real game, we would want to use an acceleration structure to not
	// make designers wait an extra hour to compile their navmesh.
	for ( int v0 = 0; v0 < triPoints.size(); v0++ ) {
		const Vec3 & pt0 = triPoints[ v0 ];

		for ( int v1 = v0 + 1; v1 < triPoints.size(); v1++ ) {
			const Vec3 & pt1 = triPoints[ v1 ];

			// Any points that are close to this point, need to be merged to it
			Vec3 delta = pt1 - pt0;
			if ( delta.GetMagnitude() < 0.01f ) {	// 1cm tolerance
				for ( int i = 0; i < triIndices.size(); i++ ) {
					int idx = triIndices[ i ];
					if ( idx == v1 ) {
						triIndices[ i ] = v0;
					}
				}				
			}
		}
	}

	//
	// Remove any dangling verts
	//
	for ( int v = 0; v < triPoints.size(); v++ ) {
		bool isUsed = false;

		for ( int i = 0; i < triIndices.size(); i++ ) {
			const int idx = triIndices[ i ];
			if ( idx == v ) {
				isUsed = true;
				break;
			}
		}

		if ( isUsed ) {
			continue;
		}

		// Remove the unused vert and decrement any references to verts greater than it
		for ( int i = 0; i < triIndices.size(); i++ ) {
			const int idx = triIndices[ i ];
			if ( idx > v ) {
				triIndices[ i ] = idx - 1;
			}
		}

		triPoints.erase( triPoints.begin() + v );
		--v;
	}

	//
	// Fix T-junctions
	//
	for ( int tri = 0; tri < triIndices.size() / 3; tri++ ) {
		int indices[ 3 ];
		indices[ 0 ] = triIndices[ tri * 3 + 0 ];
		indices[ 1 ] = triIndices[ tri * 3 + 1 ];
		indices[ 2 ] = triIndices[ tri * 3 + 2 ];

		Vec3 pts[ 3 ];
		pts[ 0 ] = triPoints[ indices[ 0 ] ];
		pts[ 1 ] = triPoints[ indices[ 1 ] ];
		pts[ 2 ] = triPoints[ indices[ 2 ] ];

		for ( int v = 0; v < triPoints.size(); v++ ) {
			if ( v == indices[ 0 ] || v == indices[ 1 ] || v == indices[ 2 ] ) {
				continue;
			}
			
			const Vec3 & pt = triPoints[ v ];
			if ( pt == pts[ 0 ] || pt == pts[ 1 ] || pt == pts[ 2 ] ) {
				continue;
			}
			
			// Check if the test vert is co-linear with any edges of the triangle
			bool didFixTJunction = false;
			for ( int i = 0; i < 3; i++ ) {
				Vec3 a = pts[ ( i + 0 ) % 3 ];
				Vec3 b = pts[ ( i + 1 ) % 3 ];
				Vec3 ab = b - a;
				float lenAB = ab.GetMagnitude();
				ab.Normalize();

				Vec3 ap = pt - a;
				float lenAP = ap.GetMagnitude();
				ap.Normalize();
				
				// Test for co-linearity
				float dot = ap.Dot( ab );
				if ( fabsf( dot ) < 0.99f ) {
					continue;
				}

				// Test if pt is within bounds of (a,b)
				float t = dot * lenAP / lenAB;
				if ( t <= 0.0f ) {
					continue;
				}
				if ( t >= 1.0f ) {
					continue;
				}

				// Test that pt is within 1cm of the edge
				Vec3 proj = a + ab * t * lenAB;
				Vec3 delta = proj - pt;
				float lenDelta = delta.GetMagnitude();
				if ( lenDelta > 0.01f ) {
					continue;
				}

				int idxA = indices[ ( i + 0 ) % 3 ];
				int idxB = indices[ ( i + 1 ) % 3 ];
				int idxC = indices[ ( i + 2 ) % 3 ];

				// Fix t-junction by replacing edge AB of this triangle
				// with two new triangles containing point v
				triIndices.push_back( v );
				triIndices.push_back( idxB );
				triIndices.push_back( idxC );

				triIndices.push_back( v );
				triIndices.push_back( idxC );
				triIndices.push_back( idxA );

				// Remove the old triangle
				triIndices.erase( triIndices.begin() + 3 * tri + 2 );
				triIndices.erase( triIndices.begin() + 3 * tri + 1 );
				triIndices.erase( triIndices.begin() + 3 * tri + 0 );
				--tri;

				didFixTJunction = true;
			}
			if ( didFixTJunction ) {
				break;
			}
		}
	}

	// Build the actual navigation graph using the triangles.
	// Each triangle is a node in the graph.  Each shared triangle edge
	// is an edge in the graph.
	const int N = triIndices.size() / 3;
	g_navAdjacencyMatrix = MatN( N );
	g_navAdjacencyMatrix.Zero();
	for ( int triA = 0; triA < N; triA++ ) {
		int indicesA[ 3 ];
		indicesA[ 0 ] = triIndices[ triA * 3 + 0 ];
		indicesA[ 1 ] = triIndices[ triA * 3 + 1 ];
		indicesA[ 2 ] = triIndices[ triA * 3 + 2 ];

		// Compare the edges of this triangle to edges of all the other triangles.
		for ( int triB = triA + 1; triB < N; triB++ ) {
			int indicesB[ 3 ];
			indicesB[ 0 ] = triIndices[ triB * 3 + 0 ];
			indicesB[ 1 ] = triIndices[ triB * 3 + 1 ];
			indicesB[ 2 ] = triIndices[ triB * 3 + 2 ];

			bool isShared = false;
			for ( int i = 0; i < 3; i++ ) {
				for ( int j = 0; j < 3; j++ ) {
					int a0 = indicesA[ ( i + 0 ) % 3 ];
					int b0 = indicesA[ ( i + 1 ) % 3 ];
					int a1 = indicesB[ ( j + 0 ) % 3 ];
					int b1 = indicesB[ ( j + 1 ) % 3 ];
					if ( IsEdgeShared( a0, b0, a1, b1 ) ) {
						isShared = true;

						// Connect these two triangles in the adjacency matrix
						g_navAdjacencyMatrix.rows[ triA ][ triB ] = 1;
						g_navAdjacencyMatrix.rows[ triB ][ triA ] = 1;
						break;
					}
				}

				if ( isShared ) {
					break;
				}
			}
		}
	}

	// Debug print the adjacency matrix
	for ( int j = 0; j < N; j++ ) {
		printf( "%i: ", j );
		for ( int i = 0; i < N; i++ ) {
			int val = (int)g_navAdjacencyMatrix.rows[ i ][ j ];
			printf( " %i", val );
		}
		printf( "\n" );
	}

	//
	//	Build the bounds for each triangle
	//
	for ( int i = 0; i < triIndices.size() / 3; i++ ) {
		int a = triIndices[ 3 * i + 0 ];
		int b = triIndices[ 3 * i + 1 ];
		int c = triIndices[ 3 * i + 2 ];

		Vec3 ptA = triPoints[ a ];
		Vec3 ptB = triPoints[ b ];
		Vec3 ptC = triPoints[ c ];

		Bounds bounds;
		bounds.Clear();
		bounds.Expand( ptA - Vec3( 0, 0, 0.01f ) );
		bounds.Expand( ptB - Vec3( 0, 0, 0.01f ) );
		bounds.Expand( ptC - Vec3( 0, 0, 0.01f ) );

		bounds.Expand( ptA + Vec3( 0, 0, 2 ) );
		bounds.Expand( ptB + Vec3( 0, 0, 2 ) );
		bounds.Expand( ptC + Vec3( 0, 0, 2 ) );
		s_navmeshBounds.push_back( bounds );
	}

	// Build render geo
	s_navmeshVerts.clear();
	s_navmeshIndices = triIndices;
	for ( int i = 0; i < triPoints.size(); i++ ) {
		vert_t vert;
		vert.pos = triPoints[ i ];
		vert.pos.z += 0.01f;
		vert.st = Vec2( 0, 0 );
		Vec3ToByte4_n11( vert.norm, Vec3( 0, 0, 1 ) );
		Vec3ToByte4_n11( vert.tang, Vec3( 1, 0, 0 ) );
		Vec3ToByte4_n11( vert.buff, Vec3( 0.5f, 0.5f, 1.0f ) );
		s_navmeshVerts.push_back( vert );
	}
	BuildNavMeshRenderGeo();

	// Write the nav mesh file
	WriteNavFile();
}

/*
================================
DrawNavMesh
================================
*/
void DrawNavMesh() {
	// Bind the VAO
    s_navmeshVAO.Bind();
    
    // Draw
	const int num = s_navmeshIndices.size();
	glDrawElements( GL_TRIANGLES, num, GL_UNSIGNED_INT, 0 );

	// Unbind the VAO
	s_navmeshVAO.UnBind();
}


int GetTriangleIndex( Vec3 pt ) {
	for ( int i = 0; i < s_navmeshBounds.size(); i++ ) {
		const Bounds & bounds = s_navmeshBounds[ i ];
		if ( !bounds.DoesIntersect( pt ) ) {
			continue;
		}

		int idx0 = s_navmeshIndices[ 3 * i + 0 ];
		int idx1 = s_navmeshIndices[ 3 * i + 1 ];
		int idx2 = s_navmeshIndices[ 3 * i + 2 ];

		const Vec3 & a = s_navmeshVerts[ idx0 ].pos;
		const Vec3 & b = s_navmeshVerts[ idx1 ].pos;
		const Vec3 & c = s_navmeshVerts[ idx2 ].pos;

		Vec3 norm = ( b - a ).Cross( c - a );
		norm.Normalize();

		Vec3 ap = pt - a;
		Vec3 p = pt - norm * ap.Dot( norm );
		
		float areaC = ( a - p ).Cross( b - p ).Dot( norm );
		float areaA = ( b - p ).Cross( c - p ).Dot( norm );
		float areaB = ( c - p ).Cross( a - p ).Dot( norm );
		if ( areaA < 0 || areaB < 0 || areaC < 0 ) {
			continue;
		}

		return i;
	}
	return -1;
}


void DrawNavMeshTriangle( Vec3 pt, Shader * shader ) {
	int tri = GetTriangleIndex( pt );
	if ( -1 == tri ) {
		return;
	}

	int a = s_navmeshIndices[ tri * 3 + 0 ];
	int b = s_navmeshIndices[ tri * 3 + 1 ];
	int c = s_navmeshIndices[ tri * 3 + 2 ];

	vert_t verts[ 3 ];
	verts[ 0 ] = s_navmeshVerts[ a ];
	verts[ 1 ] = s_navmeshVerts[ b ];
	verts[ 2 ] = s_navmeshVerts[ c ];

	verts[ 0 ].pos.z += 0.02f;
	verts[ 1 ].pos.z += 0.02f;
	verts[ 2 ].pos.z += 0.02f;

	int indices[ 3 ] = { 0, 1, 2 };

	// Update attribute values.
	const int stride = sizeof( vert_t );
	shader->SetVertexAttribPointer( "position", 3, GL_FLOAT, 0, stride, verts );
//	shader->SetVertexAttribPointer( "position", 3, GL_FLOAT, 0, stride, verts[ 0 ].pos.ToPtr() );
//	shader->SetVertexAttribPointer( "st", 2, GL_FLOAT, 0, stride, verts[ 0 ].st.ToPtr() );
//	shader->SetVertexAttribPointer( "normal", 4, GL_UNSIGNED_BYTE, 0, stride, verts[ 0 ].norm );
// 	shader->SetVertexAttribPointer( "tangent", 4, GL_UNSIGNED_BYTE, 0, stride, verts[ 0 ].tang );
// 	shader->SetVertexAttribPointer( "color", 4, GL_UNSIGNED_BYTE, 0, stride, verts[ 0 ].buff );

	// Draw
	//const int num = indices.size();
	glDrawElements( GL_TRIANGLES, 3, GL_UNSIGNED_INT, indices );
}


int GetTriangleNeighborIndices( const Vec3 & pt, int * tris ) {
	int idx = GetTriangleIndex( pt );
	if ( -1 == idx ) {
		return 0;
	}

	int num = 0;
	for ( int i = 0; i < g_navAdjacencyMatrix.numDimensions; i++ ) {
		float val = g_navAdjacencyMatrix.rows[ idx ][ i ];
		if ( val > 0 ) {
			tris[ num ] = i;
			++num;
		}
		if ( 3 == num ) {
			break;
		}
	}

	return num;
}


void DrawNavMeshNeighborTriangle( Vec3 pt, Shader * shader ) {
	int tris[ 3 ] = { -1 };
	int numNeighbors = GetTriangleNeighborIndices( pt, tris );
	if ( numNeighbors <= 0 ) {
		return;
	}

	vert_t verts[ 9 ];
	for ( int i = 0; i < numNeighbors; i++ ) {
		int tri = tris[ i ];

		int a = s_navmeshIndices[ tri * 3 + 0 ];
		int b = s_navmeshIndices[ tri * 3 + 1 ];
		int c = s_navmeshIndices[ tri * 3 + 2 ];

		verts[ 3 * i + 0 ] = s_navmeshVerts[ a ];
		verts[ 3 * i + 1 ] = s_navmeshVerts[ b ];
		verts[ 3 * i + 2 ] = s_navmeshVerts[ c ];

		verts[ 3 * i + 0 ].pos.z += 0.02f;
		verts[ 3 * i + 1 ].pos.z += 0.02f;
		verts[ 3 * i + 2 ].pos.z += 0.02f;
	}

	int indices[ 9 ] = { 0, 1, 2, 3, 4, 5, 6, 7, 8 };

	// Update attribute values.
	const int stride = sizeof( vert_t );
	shader->SetVertexAttribPointer( "position", 3, GL_FLOAT, 0, stride, verts );
//	shader->SetVertexAttribPointer( "position", 3, GL_FLOAT, 0, stride, verts[ 0 ].pos.ToPtr() );
//	shader->SetVertexAttribPointer( "st", 2, GL_FLOAT, 0, stride, verts[ 0 ].st.ToPtr() );
//	shader->SetVertexAttribPointer( "normal", 4, GL_UNSIGNED_BYTE, 0, stride, verts[ 0 ].norm );
// 	shader->SetVertexAttribPointer( "tangent", 4, GL_UNSIGNED_BYTE, 0, stride, verts[ 0 ].tang );
// 	shader->SetVertexAttribPointer( "color", 4, GL_UNSIGNED_BYTE, 0, stride, verts[ 0 ].buff );

	// Draw
	//const int num = indices.size();
	glDrawElements( GL_TRIANGLES, 3 * numNeighbors, GL_UNSIGNED_INT, indices );
}
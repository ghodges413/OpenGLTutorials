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
std::vector< navEdge_t > s_navmeshEdges;	// a list of the edges that connect nodes

extern std::vector< winding_t > s_navWindings;

static VertexBufferObject s_navmeshVBO;
static VertexArrayObject s_navmeshVAO;
static VertexBufferObject s_navmeshIBO;

MatN g_navAdjacencyMatrix;
MatN g_navEdgeListMatrix;		// Just like the adjacency matrix, but stores indices into the edge list

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

/*
================================
AreSegmentsParallel
================================
*/
bool AreSegmentsParallel( const Vec3 & a, const Vec3 & b, const Vec3 & c, const Vec3 & d ) {
	Vec3 ab = b - a;
	Vec3 cd = d - c;
	ab.Normalize();
	cd.Normalize();

	// Check for Parallel
	if ( fabsf( ab.Dot( cd ) ) < 0.99f ) {
		return false;
	}

	return true;
}

/*
================================
AreSegmentsColinear
================================
*/
bool AreSegmentsColinear( const Vec3 & a, const Vec3 & b, const Vec3 & c, const Vec3 & d ) {
	if ( !AreSegmentsParallel( a, b, c, d ) ) {
		return false;
	}

	// Check for co-linear
	Vec3 ab = b - a;
	Vec3 ac = c - a;
	Vec3 ad = d - a;
	Vec3 bc = c - b;
	Vec3 bd = d - b;
	ab.Normalize();
	ac.Normalize();
	ad.Normalize();
	bc.Normalize();
	bd.Normalize();

	// It's possible that a is a shared point with c or d.
	// So, one dot product can be zero... but if both are less than one, then it's not colinear
	float dotAC = ab.Dot( ac );
	float dotAD = ab.Dot( ad );
	float dotBC = ab.Dot( bc );
	float dotBD = ab.Dot( bd );
	if ( fabsf( dotAC ) < 0.999f && fabsf( dotAC ) > 0.001f ) {
		return false;
	}
	if ( fabsf( dotAD ) < 0.999f && fabsf( dotAD ) > 0.001f ) {
		return false;
	}
	if ( fabsf( dotBC ) < 0.999f && fabsf( dotBC ) > 0.001f ) {
		return false;
	}
	if ( fabsf( dotBD ) < 0.999f && fabsf( dotBD ) > 0.001f ) {
		return false;
	}

	return true;
}

/*
================================
DoSegmentsOverlap
================================
*/
bool DoSegmentsOverlap( const Vec3 & a, const Vec3 & b, const Vec3 & c, const Vec3 & d, Vec3 & sharedA, Vec3 & sharedB ) {
	if ( !AreSegmentsColinear( a, b, c, d ) ) {
		return false;
	}	

	// Now we need to project c & d onto ab's line and determine overlap
	Vec3 ab = b - a;
	Vec3 ac = c - a;
	Vec3 ad = d - a;
	float ta = 0;
	float tb = ab.GetMagnitude();
	ab.Normalize();
	float tc = ab.Dot( ac );
	float td = ab.Dot( ad );

	if ( tc > td ) {
		std::swap( tc, td );
	}

	if ( td <= ta ) {
		return false;
	}

	if ( tc >= tb ) {
		return false;
	}

	float tmin = ( tc < ta ) ? ta : tc;
	float tmax = ( td < tb ) ? td : tb;

	sharedA = a + ab * tmin;
	sharedB = a + ab * tmax;

	Vec3 delta = sharedB - sharedA;
	if ( delta.GetMagnitude() < 0.1f ) {
		return false;
	}

	return true;
}

/*
================================
LoadNavMesh
================================
*/
void LoadNavMesh() {
	// Attempt to read the generated navmesh file
	if ( ReadNavFile() ) {
		return;
	}

	// Voxelize the geometry and find valid nodes
	BuildNavVoxels();
	
	//
	// Initialize the adjacency matrices to empty
	//
	s_navmeshEdges.clear();
	const int N = s_navWindings.size();
	g_navAdjacencyMatrix = MatN( N );
	g_navEdgeListMatrix = MatN( N );
	for ( int i = 0; i < N; i++ ) {
		for ( int j = 0; j < N; j++ ) {
			g_navAdjacencyMatrix.rows[ i ][ j ] = 0;
			g_navAdjacencyMatrix.rows[ j ][ i ] = 0;

			g_navEdgeListMatrix.rows[ i ][ j ] = -1;
			g_navEdgeListMatrix.rows[ j ][ i ] = -1;
		}
	}

	//
	//	Find the shared edges and build the adjacency matrix
	//
	for ( int i = 0; i < s_navWindings.size(); i++ ) {
		winding_t & windingA = s_navWindings[ i ];

		for ( int j = i + 1; j < s_navWindings.size(); j++ ) {
			winding_t & windingB = s_navWindings[ j ];

			// for each edge in A, check each edge in B, to determine if there's overlap
			bool doOverlap = false;
			const int numA = windingA.pts.size();
			const int numB = windingB.pts.size();
			for ( int edgeA = 0; edgeA < numA; edgeA++ ) {
				navEdge_t a;
				a.a = windingA.pts[ ( edgeA + 0 ) % numA ];
				a.b = windingA.pts[ ( edgeA + 1 ) % numA ];
				for ( int edgeB = 0; edgeB < windingB.pts.size(); edgeB++ ) {
					navEdge_t b;
					b.a = windingB.pts[ ( edgeB + 0 ) % numB ];
					b.b = windingB.pts[ ( edgeB + 1 ) % numB ];

					// Check for overlap of these windings.  If there is a shared edge,
					// then store the shared edge and record it in the adjacency matrix.
					navEdge_t shared;
					doOverlap = DoSegmentsOverlap( a.a, a.b, b.a, b.b, shared.a, shared.b );
					if ( doOverlap ) {
						const int idx = s_navmeshEdges.size();
						s_navmeshEdges.push_back( shared );

						g_navAdjacencyMatrix.rows[ i ][ j ] = 1;
						g_navAdjacencyMatrix.rows[ j ][ i ] = 1;

						g_navEdgeListMatrix.rows[ i ][ j ] = idx;
						g_navEdgeListMatrix.rows[ j ][ i ] = idx;
						break;
					}
				}
				if ( doOverlap ) {
					break;
				}
			}			
		}
	}

	//
	//	Build the bounds for each winding
	//
	for ( int i = 0; i < s_navWindings.size(); i++ ) {
		const winding_t & winding = s_navWindings[ i ];

		Bounds bounds;
		bounds.Clear();
		for ( int j = 0; j < winding.pts.size(); j++ ) {
			Vec3 ptA = winding.pts[ j ];
			bounds.Expand( ptA - Vec3( 0, 0, 0.1f ) );
			bounds.Expand( ptA + Vec3( 0, 0, 2 ) );
		}
		s_navmeshBounds.push_back( bounds );
	}

	// Debug print the adjacency matrix
	printf( "Adjacency Matrix: %i\n", N );
	for ( int j = 0; j < N; j++ ) {
		printf( "%i: ", j );
		for ( int i = 0; i < N; i++ ) {
			int val = (int)g_navAdjacencyMatrix.rows[ i ][ j ];
			printf( " %i", val );
		}
		printf( "\n" );
	}

	// Debug print the adjacency matrix
	printf( "Edge List Matrix: %i\n", N );
	for ( int j = 0; j < N; j++ ) {
		printf( "%i: ", j );
		for ( int i = 0; i < N; i++ ) {
			int val = (int)g_navEdgeListMatrix.rows[ i ][ j ];
			printf( " %i", val + 1 );
		}
		printf( "\n" );
	}

	printf( "Num Edges: %i\n", s_navmeshEdges.size() );


	//
	//	Build render geo
	//
	s_navmeshVerts.clear();
	s_navmeshIndices.clear();
	for ( int i = 0; i < s_navWindings.size(); i++ ) {
		const winding_t & winding = s_navWindings[ i ];

		const int numVerts = s_navmeshVerts.size();
		for ( int j = 0; j < winding.pts.size(); j++ ) {
			vert_t vert;
			vert.pos = winding.pts[ j ];
			vert.pos.z += 0.01f;
			vert.st = Vec2( 0, 0 );
			Vec3ToByte4_n11( vert.norm, Vec3( 0, 0, 1 ) );
			Vec3ToByte4_n11( vert.tang, Vec3( 1, 0, 0 ) );
			Vec3ToByte4_n11( vert.buff, Vec3( 0.5f, 0.5f, 1.0f ) );
			s_navmeshVerts.push_back( vert );

			if ( j > 0 ) {
				s_navmeshIndices.push_back( numVerts + 0 );
				s_navmeshIndices.push_back( numVerts + j );
				s_navmeshIndices.push_back( numVerts + ( ( j + 1 ) % winding.pts.size() ) );
			}
		}		
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

extern int GetNodeIndex( Vec3 pt );

void DrawNavMeshNode( Vec3 pt, Shader * shader ) {
	int idx = GetNodeIndex( pt );
	if ( -1 == idx ) {
		return;
	}

	const winding_t & winding = s_navWindings[ idx ];

	int vertOffset = 0;
	int indexOffset = 0;
	for ( int i = 0; i < idx; i++ ) {
		const winding_t & winding = s_navWindings[ i ];
		vertOffset += winding.pts.size();
		indexOffset += ( winding.pts.size() - 2 ) * 3;
	}
	int numIndices = ( winding.pts.size() - 2 ) * 3;

	vert_t * verts = (vert_t *)alloca( sizeof( vert_t ) * winding.pts.size() );
	for ( int i = 0; i < winding.pts.size(); i++ ) {
		verts[ i ] = s_navmeshVerts[ vertOffset + i ];
	}
	//verts = s_navmeshVerts.data() + vertOffset;
	int * indices = (int *)alloca( sizeof( int ) * numIndices );
	for ( int i = 1; i < winding.pts.size() - 1; i++ ) {
		int idx = i - 1;
		indices[ 3 * idx + 0 ] = 0;
		indices[ 3 * idx + 1 ] = i;
		indices[ 3 * idx + 2 ] = i + 1;
	}

	// Update attribute values.
	const int stride = sizeof( vert_t );
	shader->SetVertexAttribPointer( "position", 3, GL_FLOAT, 0, stride, verts );
//	shader->SetVertexAttribPointer( "position", 3, GL_FLOAT, 0, stride, verts[ 0 ].pos.ToPtr() );
//	shader->SetVertexAttribPointer( "st", 2, GL_FLOAT, 0, stride, verts[ 0 ].st.ToPtr() );
//	shader->SetVertexAttribPointer( "normal", 4, GL_UNSIGNED_BYTE, 0, stride, verts[ 0 ].norm );
// 	shader->SetVertexAttribPointer( "tangent", 4, GL_UNSIGNED_BYTE, 0, stride, verts[ 0 ].tang );
// 	shader->SetVertexAttribPointer( "color", 4, GL_UNSIGNED_BYTE, 0, stride, verts[ 0 ].buff );

	// Draw
	glDrawElements( GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, indices );
}

void DrawNavMeshEdges( Shader * shader ) {
	vert_t vert;
	vert.st = Vec2( 0, 0 );
	Vec3ToByte4_n11( vert.norm, Vec3( 0, 0, 1 ) );
	Vec3ToByte4_n11( vert.tang, Vec3( 1, 0, 0 ) );
	Vec3ToByte4_n11( vert.buff, Vec3( 0.5f, 0.5f, 1.0f ) );


	vert_t * verts = (vert_t *)alloca( sizeof( vert_t ) * s_navmeshEdges.size() * 2 );
	for ( int i = 0; i < s_navmeshEdges.size(); i++ ) {
		const navEdge_t & edge = s_navmeshEdges[ i ];

		vert.pos = edge.a + Vec3( 0, 0, 0.2f );
		verts[ 2 * i + 0 ] = vert;

		vert.pos = edge.b + Vec3( 0, 0, 0.2f );
		verts[ 2 * i + 1 ] = vert;
	}
	
	const int numIndices = s_navmeshEdges.size() * 2;
	int * indices = (int *)alloca( sizeof( int ) * numIndices );
	for ( int i = 0; i < s_navmeshEdges.size(); i++ ) {
		indices[ 2 * i + 0 ] = 2 * i + 0;
		indices[ 2 * i + 1 ] = 2 * i + 1;
	}

	// Update attribute values.
	const int stride = sizeof( vert_t );
	shader->SetVertexAttribPointer( "position", 3, GL_FLOAT, 0, stride, verts );
//	shader->SetVertexAttribPointer( "position", 3, GL_FLOAT, 0, stride, verts[ 0 ].pos.ToPtr() );
//	shader->SetVertexAttribPointer( "st", 2, GL_FLOAT, 0, stride, verts[ 0 ].st.ToPtr() );
//	shader->SetVertexAttribPointer( "normal", 4, GL_UNSIGNED_BYTE, 0, stride, verts[ 0 ].norm );
// 	shader->SetVertexAttribPointer( "tangent", 4, GL_UNSIGNED_BYTE, 0, stride, verts[ 0 ].tang );
// 	shader->SetVertexAttribPointer( "color", 4, GL_UNSIGNED_BYTE, 0, stride, verts[ 0 ].buff );

	glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	glLineWidth( 5.0f );

	// Draw
	glDrawElements( GL_LINES, numIndices, GL_UNSIGNED_INT, indices );

	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
}

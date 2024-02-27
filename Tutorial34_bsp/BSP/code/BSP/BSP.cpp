//
//  BSP.cpp
//
#include "BSP/BSP.h"
#include "Math/MatrixOps.h"
#include "Math/Matrix.h"
#include "Math/Bounds.h"
#include "Graphics/Graphics.h"
#include "Graphics/TextureManager.h"
#include "Graphics/ShaderManager.h"
#include "Graphics/Targa.h"
#include "Miscellaneous/Fileio.h"
#include <math.h>
#include <algorithm>
#include <stdlib.h>
#include <assert.h>
#include <vector>

#define SIDE_BACK -1
#define SIDE_ON 0
#define SIDE_FRONT 1
#define SIDE_SPLIT 2

/*
================================================================
 
BSP
 
================================================================
*/

/*
================================
PlaneFromWinding
================================
*/
plane_t PlaneFromWinding( const winding_t & winding ) {
	assert( winding.pts.size() >= 3 );

	Vec3d a = winding.pts[ 0 ];
	Vec3d b = winding.pts[ 1 ];
	Vec3d c = winding.pts[ 2 ];
	Vec3d ao = Vec3d( 0 ) - a;

	plane_t plane;
	plane.pts[ 0 ] = a;
	plane.pts[ 1 ] = b;
	plane.pts[ 2 ] = c;

	plane.normal = CalculateNormal( a, b, c );	
	return plane;
}

/*
================================
GetPointSide
================================
*/
int GetPointSide( Vec3d p, const plane_t * split ) {
	Vec3d planeToPoint = p - split->pts[ 0 ];
	float dist = split->normal.Dot( planeToPoint );

	if ( dist > 0 ) {
		return SIDE_FRONT;
	}
	if ( dist < 0 ) {
		return SIDE_BACK;
	}
	return SIDE_ON;
}

/*
================================
GetWindingSide
================================
*/
int GetWindingSide( const winding_t & winding, const plane_t * split ) {
	int countPos = 0;
	int countNeg = 0;
	for ( int i = 0; i < winding.pts.size(); i++ ) {
		Vec3d p = winding.pts[ i ];
		Vec3d planeToPoint = p - split->pts[ 0 ];
		float dist = split->normal.Dot( planeToPoint );

		if ( dist > 0.001f ) {
			countPos++;
		}
		if ( dist < -0.001f ) {
			countNeg++;
		}
	}
	
	if ( countPos > 0 && 0 == countNeg ) {
		return SIDE_FRONT;
	}
	if ( countNeg > 0 && 0 == countPos ) {
		return SIDE_BACK;
	}
	if ( countPos > 0 && countNeg > 0 ) {
		return SIDE_SPLIT;
	}
	return SIDE_ON;
}

/*
================================
SplitWinding
================================
*/
bool SplitWinding( const winding_t & winding, const plane_t * split, winding_t & front, winding_t & back ) {
	front.pts.clear();
	back.pts.clear();

	bool didSplit = false;
	for ( int idx0 = 0; idx0 < winding.pts.size() ; idx0++ ) {
		int idx1 = ( idx0 + 1 ) % winding.pts.size();
		Vec3d p0 = winding.pts[ idx0 ];
		Vec3d p1 = winding.pts[ idx1 ];

		// vector from plane's surface to point
		Vec3d pp0 = p0 - split->pts[ 0 ];
		Vec3d pp1 = p1 - split->pts[ 0 ];

		float dist0 = split->normal.Dot( pp0 );
		float dist1 = split->normal.Dot( pp1 );

		// Keep points that are on the slicing plane
		if ( fabsf( dist0 ) < 0.001f ) {
			front.pts.push_back( p0 );
			back.pts.push_back( p0 );
			continue;
		}

		// Keep points that are front of the slicing plane
		if ( dist0 > 0.0f ) {
			front.pts.push_back( p0 );
		}

		// Keep points that are behind the slicing plane
		if ( dist0 < 0.0f ) {
			back.pts.push_back( p0 );
		}

		// If the second point is on the plane, then there's no need to split this segment
		if ( fabsf( dist1 ) < 0.001f ) {
			continue;
		}

		// If both points are on the same side of the plane, then there's no need to split this segment
		if ( dist0 * dist1 > 0.0f ) {
			continue;
		}

		// Split this segment
		float t = dist0 / ( dist0 - dist1 );
		Vec3d mid = p0 + ( p1 - p0 ) * t;
		front.pts.push_back( mid );
		back.pts.push_back( mid );
		didSplit = true;
	}

	if ( front.pts.size() < 3 ) {
		front.pts.clear();
	}

	if ( back.pts.size() < 3 ) {
		back.pts.clear();
	}

	return didSplit;
}

/*
================================
CalculateHeuristicValue
================================
*/
int CalculateHeuristicValue( const std::vector< winding_t > & windings, int idx ) {
	plane_t plane = PlaneFromWinding( windings[ idx ] );

	int numFront = 0;
	int numBack = 0;
	int numCoplanar = 0;
	int numStraddling = 0;
	for ( int i = 0; i < windings.size(); i++ ) {
		if ( i == idx ) {
			continue;
		}

		int side = GetWindingSide( windings[ i ], &plane );
		if ( SIDE_SPLIT == side ) {
			numStraddling++;
		}
		if ( SIDE_BACK == side ) {
			numBack++;
		}
		if ( SIDE_FRONT == side ) {
			numFront++;
		}
		if ( SIDE_ON == side ) {
			numCoplanar++;
		}
	}

	// Give preference to axis aligned splitting planes
	int axial = 0;
	if ( fabsf( plane.normal.x ) > 0.999f && fabsf( plane.normal.y ) > 0.999f && fabsf( plane.normal.z ) > 0.999f ) {
		axial = 5;
	}

	// We will use Abrash's heuristic from Quake, which was to choose a plane that split the fewest brushes
	// Actually, we're going to use a heuristic from Quake2, which is described in brushbsp.c of Radiant
	int value =  5 * numCoplanar - 5 * numStraddling - abs( numFront - numBack ) + axial;
	return value;
}

/*
================================
BSPNode_r
================================
*/
bsp_t * BSPNode_r( std::vector< winding_t > & windings, int depth ) {
	if ( 0 == windings.size() ) {
		return NULL;
	}

	// Find the winding to use for splitting
	int idx = -1;
	int bestValue = -999999;
	for ( int i = 0; i < windings.size(); i++ ) {
		if ( !windings[ i ].useForSplitting ) {
			continue;
		}

		int value = CalculateHeuristicValue( windings, i );
		if ( value > bestValue ) {
			bestValue = value;
			idx = i;
		}
	}

	bsp_t * node = new bsp_t;
	node->back = NULL;
	node->front = NULL;

	// If no suitable splitting winding was found, then this is a leaf node.
	// Store the geometry and return the childless node.
	if ( -1 == idx ) {
		node->windings = windings;
		return node;
	}
	
	std::vector< winding_t > front;
	std::vector< winding_t > back;
	front.reserve( windings.size() );
	back.reserve( windings.size() );

	// idx is the winding that we will use for this node
	node->plane = PlaneFromWinding( windings[ idx ] );
	winding_t & splitter = windings[ idx ];
	splitter.useForSplitting = false;
	front.push_back( splitter );

	
	for ( int i = 0; i < windings.size(); i++ ) {
		if ( i == idx ) {
			continue;
		}
		winding_t & winding = windings[ i ];

		winding_t windingPos;
		winding_t windingNeg;

		bool didAdd = false;
		int side = GetWindingSide( winding, &node->plane );
		if ( SIDE_ON == side ) {
			winding.useForSplitting = false;
			front.push_back( winding );
			didAdd = true;
			continue;
		}

		if ( SIDE_FRONT == side ) {
			front.push_back( winding );
			didAdd = true;
			continue;
		}

		if ( SIDE_BACK == side ) {
			back.push_back( winding );
			didAdd = true;
			continue;
		}

		if ( SIDE_SPLIT == side ) {
			winding_t frontWinding;
			winding_t backWinding;
			SplitWinding( winding, &node->plane, frontWinding, backWinding );
			if ( frontWinding.pts.size() >= 3 ) {
				frontWinding.useForSplitting = true;
				front.push_back( frontWinding );
				didAdd = true;
			}
			if ( backWinding.pts.size() >= 3 ) {
				backWinding.useForSplitting = true;
				back.push_back( backWinding );
				didAdd = true;
			}
		}

		if ( !didAdd ) {
			printf( "Failed to add winding to bsp\n" );
			assert( 0 );
		}
	}
	windings.empty();	// clear the old windings... they will not be used and are just consuming memory

	if ( 0 == front.size() ) {
		front.empty();
	}
	if ( 0 == back.size() ) {
		back.empty();
	}

	printf( "Depth/Front/Back: %i %i %i\n", depth, front.size(), back.size() );
	node->front = BSPNode_r( front, depth + 1 );
	node->back = BSPNode_r( back, depth + 1 );
	return node;
}

/*
================================
BuildBSP
================================
*/
bsp_t * BuildBSP( const brush_t * brushes, int numBrushes ) {
	if ( numBrushes <= 0 ) {
		return NULL;
	}

	// Copy the windings into one container
	std::vector< winding_t > windings;
	for ( int i = 0; i < numBrushes; i++ ) {
		const brush_t & brush = brushes[ i ];
		
		for ( int j = 0; j < brush.numPlanes; j++ ) {
			winding_t winding = brush.windings[ j ];
			winding.useForSplitting = true;

			if ( winding.pts.size() >= 3 ) {
				windings.push_back( winding );
			}
		}
	}

	bsp_t * root = BSPNode_r( windings, 0 );
	return root;
}

/*
================================
DeleteBSP_r
================================
*/
void DeleteBSP_r( bsp_t * node ) {
	if ( NULL == node ) {
		return;
	}

	DeleteBSP_r( node->front );
	DeleteBSP_r( node->back );
	delete node;
}

/*
================================
WindingToVerts
================================
*/
void WindingToVerts( const winding_t & winding, std::vector< vert_t > & verts, std::vector< int > & indices ) {
	if ( winding.pts.size() < 3 ) {
		// This should never happen
		return;
	}

	const int numVerts = verts.size();
	const Vec3d norm = CalculateNormal( winding.pts[ 0 ], winding.pts[ 1 ], winding.pts[ 2 ] );
	for ( int v = 0; v < winding.pts.size(); v++ ) {
		vert_t vert;
		memset( &vert, 0, sizeof( vert ) );
		vert.pos = winding.pts[ v ];

		vert.st = Vec2d( vert.pos.x, vert.pos.y );
		float x2 = norm.x * norm.x;
		float y2 = norm.y * norm.y;
		float z2 = norm.z * norm.z;
		if ( x2 > y2 && x2 > z2 ) {
			vert.st = Vec2d( vert.pos.y, vert.pos.z );
		}
		if ( y2 > x2 && y2 > z2 ) {
			vert.st = Vec2d( vert.pos.x, vert.pos.z );
		}
		vert.st *= 0.01f;
		Vec3dToByte4_n11( vert.norm, norm );

		verts.push_back( vert );
	}

	// Triangle fan indices for the winding
	for ( int v = 2; v < winding.pts.size(); v++ ) {
		indices.push_back( numVerts + 0 );
		indices.push_back( numVerts + v - 1 );
		indices.push_back( numVerts + v );
	}
}

/*
================================
BuildBSPModel_r
================================
*/
void BuildBSPModel_r( bsp_t * node, std::vector< vert_t > & verts, std::vector< int > & indices, int & totalWindings ) {
	if ( NULL == node ) {
		return;
	}

	// Now convert all the windings to raw vertices and indices
	Bounds bounds;
	bounds.Clear();

	const std::vector< winding_t > & windings = node->windings;
	Vec3d norm = node->plane.normal;

	for ( int w = 0; w < windings.size(); w++ ) {
		const winding_t & winding = windings[ w ];
		WindingToVerts( winding, verts, indices );
	}

	BuildBSPModel_r( node->front, verts, indices, totalWindings );
	BuildBSPModel_r( node->back, verts, indices, totalWindings );
}

/*
================================
DrawBSP_r
================================
*/
void DrawBSP_r( bsp_t * node, Shader * shader ) {
	if ( NULL == node ) {
		return;
	}

	DrawBSP_r( node->front, shader );
	DrawBSP_r( node->back, shader );

	if ( 0 == node->windings.size() ) {
		return;
	}
	assert( NULL == node->front );
	assert( NULL == node->back );

	std::vector< vert_t > verts;
	std::vector< int > indices;

	for ( int w = 0; w < node->windings.size(); w++ ) {
		const winding_t & winding = node->windings[ w ];
		WindingToVerts( winding, verts, indices );
	}
	for ( int i = 0; i < verts.size(); i++ ) {
		Vec3dToByte4_01( verts[ i ].buff, Vec3d( 1, 1, 1 ) );
	}

	glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

	// Update attribute values.
	const int stride = sizeof( vert_t );
	shader->SetVertexAttribPointer( "position", 3, GL_FLOAT, 0, stride, verts[ 0 ].pos.ToPtr() );
//	shader->SetVertexAttribPointer( "st", 2, GL_FLOAT, 0, stride, verts[ 0 ].st.ToPtr() );
	shader->SetVertexAttribPointer( "normal", 4, GL_UNSIGNED_BYTE, 0, stride, verts[ 0 ].norm );
// 	shader->SetVertexAttribPointer( "tangent", 4, GL_UNSIGNED_BYTE, 0, stride, verts[ 0 ].tang );
 	shader->SetVertexAttribPointer( "color", 4, GL_UNSIGNED_BYTE, 0, stride, verts[ 0 ].buff );

	// Draw
	const int num = indices.size();
	glDrawElements( GL_TRIANGLES, num, GL_UNSIGNED_INT, indices.data() );


	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
}

/*
================================
DrawSplitPlaneBSP_r
================================
*/
void DrawSplitPlaneBSP_r( bsp_t * node, Shader * shader, Vec3d camPos, int depth ) {
	if ( NULL == node ) {
		return;
	}

	int side = GetPointSide( camPos, &node->plane );
	if ( SIDE_FRONT == side ) {
		DrawSplitPlaneBSP_r( node->front, shader, camPos, depth + 1 );
	} else {
		DrawSplitPlaneBSP_r( node->back, shader, camPos, depth + 1 );
	}

	if ( depth > 3 ) {
		return;
	}

	Vec3d color = Vec3d( 1, 0, 0 );
	int colorID = depth % 5;
	if ( 0 == colorID ) {
		color = Vec3d( 1, 0, 0 );
	}
	if ( 1 == colorID ) {
		color = Vec3d( 0, 1, 0 );
	}
	if ( 2 == colorID ) {
		color = Vec3d( 0, 0, 1 );
	}
	if ( 3 == colorID ) {
		color = Vec3d( 1, 1, 0 );
	}
	if ( 4 == colorID ) {
		color = Vec3d( 1, 0, 1 );		
	}
	float scale = 0.004f;
	color *= scale;

	winding_t winding = BuildOversizedWinding( &node->plane );

	std::vector< vert_t > verts;
	std::vector< int > indices;
	WindingToVerts( winding, verts, indices );

	for ( int i = 0; i < verts.size(); i++ ) {
		Vec3dToByte4_01( verts[ i ].buff, color );
	}

	// Enable additive blending
	glEnable( GL_BLEND );
	glBlendFunc( GL_ONE, GL_ONE );

	// Update attribute values.
	const int stride = sizeof( vert_t );
	shader->SetVertexAttribPointer( "position", 3, GL_FLOAT, 0, stride, verts[ 0 ].pos.ToPtr() );
// 	shader->SetVertexAttribPointer( "st", 2, GL_FLOAT, 0, stride, verts[ 0 ].st.ToPtr() );
 	shader->SetVertexAttribPointer( "normal", 4, GL_UNSIGNED_BYTE, 0, stride, verts[ 0 ].norm );
// 	shader->SetVertexAttribPointer( "tangent", 4, GL_UNSIGNED_BYTE, 0, stride, verts[ 0 ].tang );
	shader->SetVertexAttribPointer( "color", 4, GL_UNSIGNED_BYTE, 0, stride, verts[ 0 ].buff );

	// Draw
	const int num = indices.size();
	glDrawElements( GL_TRIANGLES, num, GL_UNSIGNED_INT, indices.data() );

	glDisable( GL_BLEND );
}
/*
 *  Mesh.cpp
 *
 */
#include "Mesh.h"
#include "Fileio.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "Graphics.h"

/*
 ================================
 Mesh::Mesh
 ================================
 */
Mesh::Mesh() :
mPositions( 4096 ),
mNormals( 4096 ),
mST( 4096 ),
mVerts( 4096 )
{}

/*
 ================================
 Mesh::Load
 ================================
 */
bool Mesh::Load( const char * name ) {
	char fileName[ 2048 ];
	RelativePathToFullPath( name, fileName );	
	
	FILE * fp = fopen( fileName, "rb" );
	if ( !fp ) {
		fprintf( stderr, "Error: couldn't open \"%s\"!\n", fileName );
		return false;
    }
	
	char buff[ 512 ] = { 0 };

	Vec3d point;
	Vec3d norm;
	Vec2d st;

	int i[ 4 ];
	int j[ 4 ];
	int k[ 4 ];

	while ( !feof( fp ) ) {
		// Read whole line
		fgets( buff, sizeof( buff ), fp );
		
		if ( sscanf( buff, "v %f %f %f", &point.x, &point.y, &point.z ) == 3 ) {
			// Add this point to the list of positions
			mPositions.Append( point );
		} else if ( sscanf( buff, "vn %f %f %f", &norm.x, &norm.y, &norm.z ) == 3 ) {
			// Add this norm to the list of normals
			norm.Normalize();
			mNormals.Append( norm );
		} else if ( sscanf( buff, "vt %f %f", &st.x, &st.y ) == 2 ) {
			// Add this texture coordinate to the list of texture coordinates
			mST.Append( st );
		} else if ( sscanf( buff, "f %i/%i/%i %i/%i/%i %i/%i/%i %i/%i/%i", &i[ 0 ], &j[ 0 ], &k[ 0 ], &i[ 1 ], &j[ 1 ], &k[ 1 ], &i[ 2 ], &j[ 2 ], &k[ 2 ], &i[ 3 ], &j[ 3 ], &k[ 3 ] ) == 12 ) {
			// Convert this quad to two triangles and append
			vert_t vert[ 4 ];
			for ( int it = 0; it < 4; ++it ) {
				vert[ it ].pos	= mPositions[ i[ it ] - 1 ];
				vert[ it ].norm	= mNormals[ k[ it ] - 1 ];
				vert[ it ].st	= mST[ j[ it ] - 1 ];
			}
			mVerts.Append( vert[ 0 ] );
			mVerts.Append( vert[ 1 ] );
			mVerts.Append( vert[ 2 ] );

			mVerts.Append( vert[ 2 ] );
			mVerts.Append( vert[ 3 ] );
			mVerts.Append( vert[ 0 ] );
		} else if ( sscanf( buff, "f %i/%i/%i %i/%i/%i %i/%i/%i", &i[ 0 ], &j[ 0 ], &k[ 0 ], &i[ 1 ], &j[ 1 ], &k[ 1 ], &i[ 2 ], &j[ 2 ], &k[ 2 ] ) == 9 ) {
			// Add this triangle to the list of verts
			vert_t vert[ 3 ];
			for ( int it = 0; it < 3; ++it ) {
				vert[ it ].pos	= mPositions[ i[ it ] - 1 ];
				vert[ it ].norm	= mNormals[ k[ it ] - 1 ];
				vert[ it ].st	= mST[ j[ it ] - 1 ];
			}
			mVerts.Append( vert[ 0 ] );
			mVerts.Append( vert[ 1 ] );
			mVerts.Append( vert[ 2 ] );
		} else if ( sscanf( buff, "f %i/%i %i/%i %i/%i %i/%i", &i[ 0 ], &j[ 0 ], &i[ 1 ], &j[ 1 ], &i[ 2 ], &j[ 2 ], &i[ 3 ], &j[ 3 ] ) == 8 ) {
			// Convert this quad to two triangles and append
			vert_t vert[ 4 ];
			for ( int it = 0; it < 4; ++it ) {
				vert[ it ].pos	= mPositions[ i[ it ] - 1 ];
				vert[ it ].st	= mST[ j[ it ] - 1 ];
			}

			// Calculate normal from face geometry
			Vec3d a = vert[ 0 ].pos - vert[ 1 ].pos;
			Vec3d b = vert[ 2 ].pos - vert[ 1 ].pos;
			Vec3d norm = b.Cross( a );
			norm.Normalize();
			for ( int it = 0; it < 4; ++it ) {
				vert[ it ].norm	= norm;
			}

			mVerts.Append( vert[ 0 ] );
			mVerts.Append( vert[ 1 ] );
			mVerts.Append( vert[ 2 ] );

			mVerts.Append( vert[ 2 ] );
			mVerts.Append( vert[ 3 ] );
			mVerts.Append( vert[ 0 ] );
		} else if ( sscanf( buff, "f %i/%i %i/%i %i/%i", &i[ 0 ], &j[ 0 ], &i[ 1 ], &j[ 1 ], &i[ 2 ], &j[ 2 ] ) == 6 ) {
			// Add this triangle to the list of verts
			vert_t vert[ 3 ];
			for ( int it = 0; it < 3; ++it ) {
				vert[ it ].pos	= mPositions[ i[ it ] - 1 ];
				vert[ it ].st	= mST[ j[ it ] - 1 ];
			}

			// Calculate normal from face geometry
			Vec3d a = vert[ 0 ].pos - vert[ 1 ].pos;
			Vec3d b = vert[ 2 ].pos - vert[ 1 ].pos;
			Vec3d norm = b.Cross( a );
			norm.Normalize();
			for ( int it = 0; it < 3; ++it ) {
				vert[ it ].norm	= norm;
			}

			mVerts.Append( vert[ 0 ] );
			mVerts.Append( vert[ 1 ] );
			mVerts.Append( vert[ 2 ] );
		}
	}

	for ( int i = 0; i < mVerts.Num(); i++ ) {
		Vec2d & st = mVerts[ i ].st;
		st.y = 1.0f - st.y;
		//st.x = 1.0f - st.x;
	}

	CalculateTangents();
	MakeVBO();

	fclose( fp );
	return true;
}

/*
 ================================
 Mesh::CalculateTangents
 ================================
 */
void Mesh::CalculateTangents() {
	for ( int i = 0; i < mVerts.Num(); i += 3 ) {
		vert_t & v0 = mVerts[ i + 0 ];
		vert_t & v1 = mVerts[ i + 1 ];
		vert_t & v2 = mVerts[ i + 2 ];

		// grab the vertices to the triangle
		Vec3d vA = v0.pos;
		Vec3d vB = v1.pos;
		Vec3d vC = v2.pos;
    
		Vec3d vAB = vB - vA;
		Vec3d vAC = vC - vA;
    
		// get the ST mapping values for the triangle
		Vec2d stA = v0.st;
		Vec2d stB = v1.st;
		Vec2d stC = v2.st;
		Vec2d deltaSTab = stB - stA;
		Vec2d deltaSTac = stC - stA;
		deltaSTab.Normalize();
		deltaSTac.Normalize();
    
		// calculate tangents
		Vec3d tangent;
		{
			Vec3d v1 = vAC;
			Vec3d v2 = vAB;
			Vec2d st1 = deltaSTac;
			Vec2d st2 = deltaSTab;
			float coef = 1.0f / ( st1.x * st2.y - st2.x * st1.y );
			tangent.x = coef * ( ( v1.x * st2.y ) + ( v2.x * -st1.y ) );
			tangent.y = coef * ( ( v1.y * st2.y ) + ( v2.y * -st1.y ) );
			tangent.z = coef * ( ( v1.z * st2.y ) + ( v2.z * -st1.y ) );
		}
		tangent.Normalize();
    
		// store the tangents
		v0.tang = tangent;
		v1.tang = tangent;
		v2.tang = tangent;
	}
}


/*
 ======================================
 Mesh::MakeVBO
 ======================================
 */
void Mesh::MakeVBO() {
	assert( !mVBO.IsValid() );

    mVBO.Generate( GL_ARRAY_BUFFER, mVerts.Num() * sizeof( vert_t ), mVerts.ToPtr(), GL_STATIC_DRAW );
    
    //
    //  Generate the VAO and fill
    //
    {
        // AOS - array of structure
        mVAO.Generate();
        mVAO.Bind();
        
        mVBO.Bind();
        
        const int stride = sizeof( vert_t );
        unsigned long offset = 0;
        
        // Enable and Pass this vbo data into the array
        glEnableVertexAttribArray( 0 );
        glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset );
        offset += sizeof( Vec3d );
        
        glEnableVertexAttribArray( 1 );
        glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset );
        offset += sizeof( Vec3d );
        
        glEnableVertexAttribArray( 2 );
        glVertexAttribPointer( 2, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset );
        offset += sizeof( Vec3d );
        
        glEnableVertexAttribArray( 3 );
        glVertexAttribPointer( 3, 2, GL_FLOAT, GL_FALSE, stride, (const void *)offset );
        
        myglGetError();
        
        mVAO.UnBind();
        mVBO.UnBind();
    }
}

/*
 ======================================
 Mesh::Draw
 ======================================
 */
void Mesh::Draw() const {
    // Bind the VAO
    mVAO.Bind();
    
    // Draw
    glDrawArrays( GL_TRIANGLES, 0, mVerts.Num() );

	// Unbind the VAO
	mVAO.UnBind();
}






Vec3d CalculateNormal( float nx, float px, float ny, float py ) {
	float dx = nx - px;
	float dy = ny - py;
// 	dx *= 2.0f;
// 	dy *= 2.0f;
	float dz = sqrtf( 1.0f - dx * dx - dy * dy );

	Vec3d norm = Vec3d( dx, dy, dz );
	norm.Normalize();
	return norm;
}

Vec3d CalculateNormal( int x, int y, int dimX, int dimY, const Vec4d * samples ) {
#if 1
	Vec4d sample = samples[ x + y * dimX ];
	float z = sqrtf( 1.0f - sample.y * sample.y - sample.z * sample.z );
	Vec3d norm = Vec3d( sample.y, sample.z, z );
	return norm;
#else
	float nx;
	float px;
	float ny;
	float py;

	if ( x == 0 ) {
		int idx = x + y * dimX;
		nx = samples[ idx ].x;
		px = samples[ idx + 1 ].x;
	} else if ( x >= dimX - 1 ) {
		int idx = x - 1 + y * dimX;
		nx = samples[ idx ].x;
		px = samples[ idx + 1 ].x;
	} else {
		int idx = x + y * dimX;
		nx = samples[ idx - 1 ].x;
		px = samples[ idx + 1 ].x;
	}

	if ( y == 0 ) {
		int idx = x + y * dimX;
		nx = samples[ idx ].x;
		px = samples[ idx + dimX ].x;
	} else if ( y >= dimY - 1 ) {
		int idx = x + ( y - 1 ) * dimX;
		nx = samples[ idx ].x;
		px = samples[ idx + dimX ].x;
	} else {
		int idx = x + y * dimX;
		nx = samples[ idx - dimX ].x;
		px = samples[ idx + dimX ].x;
	}

	return CalculateNormal( nx, px, ny, py );
#endif
}
#if 0
void Mesh::GenerateMeshFromHeightmap( const Vec4d * samples, const int dimX, const int dimY ) {
	mVerts.Clear();

	int numTiles = dimX * dimY;
	int numVerts = 6 * numTiles;
	vert_t * verts = (vert_t *)malloc( numVerts * sizeof( vert_t ) );
	assert( verts );
	if ( NULL == verts ) {
		return;
	}
	int v = 0;

	Vec3d normal = Vec3d( 0, 0, 1 );
	Vec3d pos = Vec3d( 0, 0, 0 );

	int vertOffset = 0;

	float minHeight = 100000.0f;
	float maxHeight = -100000.0f;
	float avgHeight = 0;
	float avgCounter = 0;

	for ( int y = 0; y < dimY - 1; y++ ) {
		for ( int x = 0; x < dimX - 1; x++ ) {
			int x0 = x + 0;
			int x1 = x + 1;
			int y0 = y + 0;
			int y1 = y + 1;

			int idxA = x0 + y0 * dimX;
			int idxB = x0 + y1 * dimX;
			int idxC = x1 + y1 * dimX;
			int idxD = x1 + y0 * dimX;

			Vec4d sampleA = samples[ idxA ];
			Vec4d sampleB = samples[ idxB ];
			Vec4d sampleC = samples[ idxC ];
			Vec4d sampleD = samples[ idxD ];

			// TODO: scale and recenter the x/y
			Vec3d posA = Vec3d( x0, y0, sampleA.x );
			Vec3d posB = Vec3d( x0, y1, sampleB.x );
			Vec3d posC = Vec3d( x1, y1, sampleC.x );
			Vec3d posD = Vec3d( x1, y0, sampleD.x );

			if ( sampleA.x < minHeight ) {
				minHeight = sampleA.x;
			}
			if ( sampleB.x < minHeight ) {
				minHeight = sampleB.x;
			}
			if ( sampleC.x < minHeight ) {
				minHeight = sampleC.x;
			}
			if ( sampleD.x < minHeight ) {
				minHeight = sampleD.x;
			}

			if ( sampleA.x > maxHeight ) {
				maxHeight = sampleA.x;
			}
			if ( sampleB.x > maxHeight ) {
				maxHeight = sampleB.x;
			}
			if ( sampleC.x > maxHeight ) {
				maxHeight = sampleC.x;
			}
			if ( sampleD.x > maxHeight ) {
				maxHeight = sampleD.x;
			}
			avgHeight += sampleA.x * 0.25f;
			avgHeight += sampleB.x * 0.25f;
			avgHeight += sampleC.x * 0.25f;
			avgHeight += sampleD.x * 0.25f;
			avgCounter += 1.0f;

// 			if ( y == 0 && x == 0 ) {
// 				Vec3d posA = Vec3d( 0, 0, 0 ) * 10.0f;
// 				Vec3d posB = Vec3d( 1, 0, 0 ) * 10.0f;
// 				Vec3d posC = Vec3d( 1, 1, 1 ) * 10.0f;
// 				Vec3d posD = Vec3d( 0, 1, 1 ) * 10.0f;
// 			}

			// TODO: Actually do this
// 			Vec3d normA = Vec3d( 0, 0, 1 );
// 			Vec3d normB = Vec3d( 0, 0, 1 );
// 			Vec3d normC = Vec3d( 0, 0, 1 );
// 			Vec3d normD = Vec3d( 0, 0, 1 );

			Vec3d normA = CalculateNormal( x0, y0, dimX, dimY, samples );
			Vec3d normB = CalculateNormal( x0, y1, dimX, dimY, samples );
			Vec3d normC = CalculateNormal( x1, y1, dimX, dimY, samples );
			Vec3d normD = CalculateNormal( x1, y0, dimX, dimY, samples );

			Vec3d colorA = CalculateColor( posA, normA, sampleA );
			Vec3d colorB = CalculateColor( posB, normB, sampleB );
			Vec3d colorC = CalculateColor( posC, normC, sampleC );
			Vec3d colorD = CalculateColor( posD, normD, sampleD );

			vert_t vertA;
			vertA.pos	= posA;
			vertA.norm	= normA;
			vertA.tang	= colorA;
			vertA.st	= Vec2d( x0, y0 ) * ( 1.0f / dimX );

			vert_t vertB;
			vertB.pos	= posB;
			vertB.norm	= normB;
			vertB.tang	= colorB;
			vertB.st	= Vec2d( x0, y1 ) * ( 1.0f / dimX );

			vert_t vertC;
			vertC.pos	= posC;
			vertC.norm	= normC;
			vertC.tang	= colorC;
			vertC.st	= Vec2d( x1, y1 ) * ( 1.0f / dimX );

			vert_t vertD;
			vertD.pos	= posD;
			vertD.norm	= normD;
			vertD.tang	= colorD;
			vertD.st	= Vec2d( x1, y0 ) * ( 1.0f / dimX );

			// Tri0 = A B D
			// Tri1 = C D B
			verts[ vertOffset + 0 ] = vertA;
			verts[ vertOffset + 1 ] = vertD;
			verts[ vertOffset + 2 ] = vertB;

			verts[ vertOffset + 3 ] = vertD;
			verts[ vertOffset + 4 ] = vertC;
			verts[ vertOffset + 5 ] = vertB;

			vertOffset += 6;
		}
	}
	float centerHeight = maxHeight - minHeight;
	avgHeight /= avgCounter;

	for ( int i = 0; i < numVerts; i++ ) {
		vert_t & vert = verts[ i ];
// 		vert.pos.x -= dimX >> 1;
// 		vert.pos.y -= dimY >> 1;
		vert.pos.x /= dimX;
		vert.pos.y /= dimY;
		vert.pos.x -= 0.5f;
		vert.pos.y -= 0.5f;

		vert.pos.z -= centerHeight;
		//vert.pos.z -= avgHeight;

// 		vert.pos *= 5000.0f;
// 		vert.pos.z -= 2000.0f;

		vert.pos *= 500.0f;
		vert.pos.z -= 200.0f;

// 		vert.pos.z *= 1000.0f;
// 		vert.pos.z -= 100.0f;
	}

	mVerts.Resize( numVerts );
	for ( int i = 0; i < numVerts; i++ ) {
		vert_t & vert = verts[ i ];
		mVerts.Append( vert );
	}

	free( verts );
	MakeVBO();
}
#endif
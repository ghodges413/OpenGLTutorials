//
//	Mesh.cpp
//
#include "Graphics/Mesh.h"
#include "Miscellaneous/Fileio.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "Graphics/Graphics.h"

/*
 ================================
 Mesh::Mesh
 ================================
 */
Mesh::Mesh() :
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

	Vec3 point;
	Vec3 norm;
	Vec2 st;

	Array< Vec3 > mPositions( 4096 );
	Array< Vec3 > mNormals( 4096 );
	Array< Vec2 > mST( 4096 );

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
				Vec3ToByte4_n11( vert[ it ].norm, mNormals[ k[ it ] - 1 ] );
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
				Vec3ToByte4_n11( vert[ it ].norm, mNormals[ k[ it ] - 1 ] );
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
			Vec3 a = vert[ 0 ].pos - vert[ 1 ].pos;
			Vec3 b = vert[ 2 ].pos - vert[ 1 ].pos;
			Vec3 norm = b.Cross( a );
			norm.Normalize();
			for ( int it = 0; it < 4; ++it ) {
				Vec3ToByte4_n11( vert[ it ].norm, norm );
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
			Vec3 a = vert[ 0 ].pos - vert[ 1 ].pos;
			Vec3 b = vert[ 2 ].pos - vert[ 1 ].pos;
			Vec3 norm = b.Cross( a );
			norm.Normalize();
			for ( int it = 0; it < 3; ++it ) {
				vert[ it ].norm[ 0 ] = FloatToByte_n11( norm.x );
				vert[ it ].norm[ 1 ] = FloatToByte_n11( norm.y );
				vert[ it ].norm[ 2 ] = FloatToByte_n11( norm.z );
				vert[ it ].norm[ 3 ] = 0;
			}

			mVerts.Append( vert[ 0 ] );
			mVerts.Append( vert[ 1 ] );
			mVerts.Append( vert[ 2 ] );
		}
	}

	for ( int i = 0; i < mVerts.Num(); i++ ) {
		Vec2 & st = mVerts[ i ].st;
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
		Vec3 vA = v0.pos;
		Vec3 vB = v1.pos;
		Vec3 vC = v2.pos;
    
		Vec3 vAB = vB - vA;
		Vec3 vAC = vC - vA;
    
		// get the ST mapping values for the triangle
		Vec2 stA = v0.st;
		Vec2 stB = v1.st;
		Vec2 stC = v2.st;
		Vec2 deltaSTab = stB - stA;
		Vec2 deltaSTac = stC - stA;
		deltaSTab.Normalize();
		deltaSTac.Normalize();
    
		// calculate tangents
		Vec3 tangent;
		{
			Vec3 v1 = vAC;
			Vec3 v2 = vAB;
			Vec2 st1 = deltaSTac;
			Vec2 st2 = deltaSTab;
			float coef = 1.0f / ( st1.x * st2.y - st2.x * st1.y );
			tangent.x = coef * ( ( v1.x * st2.y ) + ( v2.x * -st1.y ) );
			tangent.y = coef * ( ( v1.y * st2.y ) + ( v2.y * -st1.y ) );
			tangent.z = coef * ( ( v1.z * st2.y ) + ( v2.z * -st1.y ) );
		}
		tangent.Normalize();
    
		// store the tangents
// 		v0.tang = tangent;
// 		v1.tang = tangent;
// 		v2.tang = tangent;
		Vec3ToByte4_n11( v0.tang, tangent );
		Vec3ToByte4_n11( v1.tang, tangent );
		Vec3ToByte4_n11( v2.tang, tangent );
	}
}

/*
======================================
Mesh::LoadFromData
======================================
*/
bool Mesh::LoadFromData( const vert_t * verts, int numVerts, const unsigned short * indices, int numIndices ) {
	mVerts.Clear();
	for ( int i = 0; i < numIndices; i++ ) {
		unsigned short idx = indices[ i ];
		mVerts.Append( verts[ idx ] );
	}
	MakeVBO();
	return true;
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





#if 0
Vec3 CalculateNormal( float nx, float px, float ny, float py ) {
	float dx = nx - px;
	float dy = ny - py;
// 	dx *= 2.0f;
// 	dy *= 2.0f;
	float dz = sqrtf( 1.0f - dx * dx - dy * dy );

	Vec3 norm = Vec3( dx, dy, dz );
	norm.Normalize();
	return norm;
}

Vec3 CalculateNormal( int x, int y, int dimX, int dimY, const Vec4 * samples ) {
#if 1
	Vec4 sample = samples[ x + y * dimX ];
	float z = sqrtf( 1.0f - sample.y * sample.y - sample.z * sample.z );
	Vec3 norm = Vec3( sample.y, sample.z, z );
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
#endif
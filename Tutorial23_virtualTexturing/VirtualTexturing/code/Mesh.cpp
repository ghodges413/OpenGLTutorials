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







void Mesh::GenerateTerrainMesh() {
	mVerts.Clear();

	int width = 100;
	int numTiles = width * width;
	int numVerts = 6 * numTiles;
	vert_t * verts = (vert_t *)malloc( numVerts * sizeof( vert_t ) );
	int v = 0;

	for ( int y = 0; y < width - 1; y++ ) {
		for ( int x = 0; x < width - 1; x++ ) {
			float s = 100;
			float x0 = s * x + 0;
			float x1 = s * x + s;
			float y0 = s * y + 0;
			float y1 = s * y + s;
			Vec3d p0 = Vec3d( x0, y0, 0 );
			Vec3d p1 = Vec3d( x0, y1, 0 );
			Vec3d p2 = Vec3d( x1, y1, 0 );
			Vec3d p3 = Vec3d( x1, y0, 0 );
			Vec2d st0 = Vec2d( x + 0, y + 0 ) * ( 1.0f / float( width ) );
			Vec2d st1 = Vec2d( x + 0, y + 1 ) * ( 1.0f / float( width ) );
			Vec2d st2 = Vec2d( x + 1, y + 1 ) * ( 1.0f / float( width ) );
			Vec2d st3 = Vec2d( x + 1, y + 0 ) * ( 1.0f / float( width ) );


			vert_t vert;
			vert.norm = Vec3d( 0, 0, 1 );
			vert.tang = Vec3d( 1, 0, 0 );

			// Triangle A
			vert.pos = p0;
			vert.st = st0;
			verts[ v ] = vert;
			v++;

			

			vert.pos = p3;
			vert.st = st3;
			verts[ v ] = vert;
			v++;

			vert.pos = p1;
			vert.st = st1;
			verts[ v ] = vert;
			v++;

			// Triangle B
			vert.pos = p1;
			vert.st = st1;
			verts[ v ] = vert;
			v++;

			

			vert.pos = p3;
			vert.st = st3;
			verts[ v ] = vert;
			v++;

			vert.pos = p2;
			vert.st = st2;
			verts[ v ] = vert;
			v++;
		}
	}

	for ( int i = 0; i < v; i++ ) {
		vert_t & vert = verts[ i ];
		float x = vert.pos.x;
		float y = vert.pos.y;
		float z = cosf( x * 0.15f ) * sinf( y * 0.05f ) * 20.0f;
		//float z = cosf( x * 0.015f ) * sinf( y * 0.005f ) * cosf( x * y * 0.001f ) * 20.0f;
		vert.pos.z = z - 100.0f;
		vert.pos.x -= 100.0f * 100.0f * 0.5f;
		vert.pos.y -= 100.0f * 100.0f * 0.5f;
	}

	mVerts.Resize( v );
	for ( int i = 0; i < v; i++ ) {
		vert_t & vert = verts[ i ];
		mVerts.Append( vert );
	}

	free( verts);
	MakeVBO();
}
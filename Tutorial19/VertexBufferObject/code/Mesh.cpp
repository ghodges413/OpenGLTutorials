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
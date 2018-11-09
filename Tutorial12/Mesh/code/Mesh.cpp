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


#ifdef WINDOWS
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <sys/file.h>
#include <sys/mman.h>
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

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
				vert[ it ].norm	= mNormals[ j[ it ] - 1 ];
				vert[ it ].st	= mST[ k[ it ] - 1 ];
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
				vert[ it ].norm	= mNormals[ j[ it ] - 1 ];
				vert[ it ].st	= mST[ k[ it ] - 1 ];
			}
			mVerts.Append( vert[ 0 ] );
			mVerts.Append( vert[ 1 ] );
			mVerts.Append( vert[ 2 ] );
		} else if ( sscanf( buff, "f %i/%i %i/%i %i/%i %i/%i", &i[ 0 ], &k[ 0 ], &i[ 1 ], &k[ 1 ], &i[ 2 ], &k[ 2 ], &i[ 3 ], &k[ 3 ] ) == 8 ) {
			// Convert this quad to two triangles and append
			vert_t vert[ 4 ];
			for ( int it = 0; it < 4; ++it ) {
				vert[ it ].pos	= mPositions[ i[ it ] - 1 ];
				vert[ it ].st	= mST[ k[ it ] - 1 ];
			}

			// Calculate normal from face geometry
			Vec3d a = vert[ 0 ].pos - vert[ 1 ].pos;
			Vec3d b = vert[ 2 ].pos - vert[ 1 ].pos;
			Vec3d norm = a.Cross( b );
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
		} else if ( sscanf( buff, "f %i/%i %i/%i %i/%i", &i[ 0 ], &k[ 0 ], &i[ 1 ], &k[ 1 ], &i[ 2 ], &k[ 2 ] ) == 6 ) {
			// Add this triangle to the list of verts
			vert_t vert[ 3 ];
			for ( int it = 0; it < 3; ++it ) {
				vert[ it ].pos	= mPositions[ i[ it ] - 1 ];
				vert[ it ].st	= mST[ k[ it ] - 1 ];
			}

			// Calculate normal from face geometry
			Vec3d a = vert[ 0 ].pos - vert[ 1 ].pos;
			Vec3d b = vert[ 2 ].pos - vert[ 1 ].pos;
			Vec3d norm = a.Cross( b );
			norm.Normalize();
			for ( int it = 0; it < 3; ++it ) {
				vert[ it ].norm	= norm;
			}

			mVerts.Append( vert[ 0 ] );
			mVerts.Append( vert[ 1 ] );
			mVerts.Append( vert[ 2 ] );
		}
	}

	fclose( fp );
	return true;
}
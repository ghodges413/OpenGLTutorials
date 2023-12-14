//
//  TerrainFile.cpp
//
#include "TerrainFile.h"
#include "../Graphics.h"
#include "../Shader.h"
#include "../Fileio.h"
#include <math.h>
#include <algorithm>
#include <stdlib.h>
#include <assert.h>


/*
================================
WriteHeightmapBinaryFile
================================
*/
void WriteHeightmapBinaryFile( Vec4d * heightmap, int width, int tileX, int tileY ) {
	char strBuffer[ 1024 ];
	sprintf( strBuffer, "generated/terrain%i%i.heightmap", tileX, tileY );

	if ( !OpenFileWriteStream( strBuffer ) ) {
		return;
	}

	const int nx = width;
	const int ny = width;

	WriteFileStream( &width, sizeof( int ) );
	for ( int j = ny - 1; j >= 0; j-- ) {
		for ( int i = 0; i < nx; i++ ) {
            int idx = i + width * j;
            //Vec4d h2 = heightmap[ idx ];
			//sprintf( strBuffer, "%f %f %f %f\n", h2.x, h2.y, h2.z, h2.w );
			WriteFileStream( heightmap + idx, sizeof( Vec4d ) );
		}
	}

	CloseFileWriteStream();
}

/*
================================
WriteHeightmapFile
================================
*/
void WriteHeightmapFile( Vec4d * heightmap, int width, int tileX, int tileY ) {
	WriteHeightmapBinaryFile( heightmap, width, tileX, tileY );

	char strBuffer[ 1024 ];
	sprintf( strBuffer, "generated/terrain%i%i.pfm", tileX, tileY );
	if ( !OpenFileWriteStream( strBuffer ) ) {
		return;
	}

	const int nx = width;
	const int ny = width;
	
	WriteFileStream( strBuffer );
	for ( int j = ny - 1; j >= 0; j-- ) {
		for ( int i = 0; i < nx; i++ ) {
            int idx = i + width * j;
            Vec4d h2 = heightmap[ idx ];
			sprintf( strBuffer, "%f %f %f %f\n", h2.x, h2.y, h2.z, h2.w );
			WriteFileStream( strBuffer );
		}
	}

	CloseFileWriteStream();
}




/*
================================
WriteHeightmapBinaryFile
================================
*/
void WriteHeightmapBinaryFile( Vec4d * heightmap, int width ) {
	if ( !OpenFileWriteStream( "generated/terrain.heightmap" ) ) {
		return;
	}

    char strBuffer[ 1024 ];

	const int nx = width;
	const int ny = width;

	WriteFileStream( &width, sizeof( int ) );
	for ( int j = ny - 1; j >= 0; j-- ) {
		for ( int i = 0; i < nx; i++ ) {
            int idx = i + width * j;
            //Vec4d h2 = heightmap[ idx ];
			//sprintf( strBuffer, "%f %f %f %f\n", h2.x, h2.y, h2.z, h2.w );
			WriteFileStream( heightmap + idx, sizeof( Vec4d ) );
		}
	}

	CloseFileWriteStream();
}

/*
================================
WriteHeightmapFile
================================
*/
void WriteHeightmapFile( Vec4d * heightmap, int width ) {
	WriteHeightmapBinaryFile( heightmap, width );

	if ( !OpenFileWriteStream( "generated/terrain.pfm" ) ) {
		return;
	}

    char strBuffer[ 1024 ];

	const int nx = width;
	const int ny = width;

	sprintf( strBuffer, "%i %i\n", nx, ny );
	WriteFileStream( strBuffer );
	for ( int j = ny - 1; j >= 0; j-- ) {
		for ( int i = 0; i < nx; i++ ) {
            int idx = i + width * j;
            Vec4d h2 = heightmap[ idx ];
			sprintf( strBuffer, "%f %f %f %f\n", h2.x, h2.y, h2.z, h2.w );
			WriteFileStream( strBuffer );
		}
	}

	CloseFileWriteStream();
}








/*
================================
ReadHeightmapBinaryFile
================================
*/
Vec4d * ReadHeightmapBinaryFile( int width, int tileX, int tileY ) {
	char strBuffer[ 1024 ];
	sprintf( strBuffer, "generated/terrain%i%i.heightmap", tileX, tileY );

	unsigned int size = 0;
	unsigned char * data = NULL;
	if ( !GetFileData( strBuffer, &data, size ) ) {
		return NULL;
	}

	if ( NULL == data ) {
		return NULL;
	}

	if ( size < width * width * sizeof( Vec4d ) ) {
		free( data );
		return NULL;
	}

	int readWidth = 0;
	memcpy( &readWidth, data, sizeof( int ) );
	if ( readWidth != width ) {
		free( data );
		return NULL;
	}

	unsigned char * data_itr = data + sizeof( int );
	Vec4d * heightmap = (Vec4d *)malloc( sizeof( Vec4d ) * width * width );
	for ( int i = 0; i < width * width; i++ ) {
		heightmap[ i ] = *((Vec4d *)data_itr);
		data_itr += sizeof( Vec4d );
	}

	free( data );
	return heightmap;
}

/*
================================
ReadHeightmapFile
================================
*/
Vec4d * ReadHeightmapFile( const int width, int tileX, int tileY ) {
	// Attempt to read from the binary file first
	Vec4d * heightmap = ReadHeightmapBinaryFile( width, tileX, tileY );
	if ( heightmap != NULL ) {
		return heightmap;
	}

	char strBuffer[ 1024 ];
	sprintf( strBuffer, "generated/terrain%i%i.pfm", tileX, tileY );

	char fullPath[ 2048 ];
	RelativePathToFullPath( strBuffer, fullPath );
	printf( "Reading Heightmap File: %s\n", fullPath );
	FILE * fp = NULL;
	char buff[ 512 ] = { 0 };
	int version = 0;
	int currentMesh = 0;
	
	int numJoints = 0;
	int numMeshes = 0;
	
	fp = fopen( fullPath, "rb" );
	if ( !fp ) {
		fprintf( stderr, "Error: couldn't open \"%s\"!\n", fullPath );
		return NULL;
    }

	int dimX = 0;
	int dimY = 0;

	Vec4d * data = NULL;

	int idx = 0;
	Vec4d tmp;
	
	while ( !feof( fp ) ) {
		// Read whole line
		fgets( buff, sizeof( buff ), fp );
		Vec4d tmp;
		
		if ( sscanf( buff, "%i %i", &dimX, &dimY ) == 2 ) {
			if ( dimX != dimY || dimX != width ) {
				fclose( fp );
				return NULL;
			}

			data = (Vec4d *)malloc( sizeof( Vec4d ) * ( dimX + 1 ) * ( dimY + 1 ) );
			if ( NULL == data ) {
				fclose( fp );
				return NULL;
			}
		} else if ( sscanf( buff, "%f %f %f %f", &tmp.x, &tmp.y, &tmp.z, &tmp.w ) == 4 ) {		// numJoints
			assert( data );
			if ( data ) {
				data[ idx ] = tmp;
				idx++;
			}
		}

		if ( ( idx % ( 4096 * 512 ) ) == 0 ) {
			printf( "reading: %i\n", idx );
		}
	}

	fclose( fp );
	return data;
}





/*
================================
ReadHeightmapBinaryFile
================================
*/
Vec4d * ReadHeightmapBinaryFile( int width ) {
	unsigned int size = 0;
	unsigned char * data = NULL;
	if ( !GetFileData( "generated/terrain.heightmap", &data, size ) ) {
		return NULL;
	}

	if ( NULL == data ) {
		return NULL;
	}

	if ( size < width * width * sizeof( Vec4d ) ) {
		free( data );
		return NULL;
	}

	int readWidth = 0;
	memcpy( &readWidth, data, sizeof( int ) );
	if ( readWidth != width ) {
		free( data );
		return NULL;
	}

	unsigned char * data_itr = data + sizeof( int );
	Vec4d * heightmap = (Vec4d *)malloc( sizeof( Vec4d ) * width * width );
	for ( int i = 0; i < width * width; i++ ) {
		heightmap[ i ] = *((Vec4d *)data_itr);
		data_itr += sizeof( Vec4d );
	}

	free( data );
	return heightmap;
}

/*
================================
ReadHeightmapFile
================================
*/
Vec4d * ReadHeightmapFile( const int width ) {
	// Attempt to read from the binary file first
	Vec4d * heightmap = ReadHeightmapBinaryFile( width );
	if ( heightmap != NULL ) {
		return heightmap;
	}

	char fullPath[ 2048 ];
	RelativePathToFullPath( "generated/terrain.pfm", fullPath );
	printf( "Reading Heightmap File: %s\n", fullPath );
	FILE * fp = NULL;
	char buff[ 512 ] = { 0 };
	int version = 0;
	int currentMesh = 0;
	
	int numJoints = 0;
	int numMeshes = 0;
	
	fp = fopen( fullPath, "rb" );
	if ( !fp ) {
		fprintf( stderr, "Error: couldn't open \"%s\"!\n", fullPath );
		return NULL;
    }

	int dimX = 0;
	int dimY = 0;

	Vec4d * data = NULL;

	int idx = 0;
	Vec4d tmp;
	
	while ( !feof( fp ) ) {
		// Read whole line
		fgets( buff, sizeof( buff ), fp );
		Vec4d tmp;
		
		if ( sscanf( buff, "%i %i", &dimX, &dimY ) == 2 ) {
			if ( dimX != dimY || dimX != width ) {
				fclose( fp );
				return NULL;
			}

			data = (Vec4d *)malloc( sizeof( Vec4d ) * ( dimX + 1 ) * ( dimY + 1 ) );
			if ( NULL == data ) {
				fclose( fp );
				return NULL;
			}
		} else if ( sscanf( buff, "%f %f %f %f", &tmp.x, &tmp.y, &tmp.z, &tmp.w ) == 4 ) {		// numJoints
			assert( data );
			if ( data ) {
				data[ idx ] = tmp;
				idx++;
			}
		}

		if ( ( idx % ( 4096 * 512 ) ) == 0 ) {
			printf( "reading: %i\n", idx );
		}
	}

	fclose( fp );
	return data;
}

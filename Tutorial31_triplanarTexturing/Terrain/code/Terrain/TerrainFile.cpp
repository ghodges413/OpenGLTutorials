//
//  TerrainFile.cpp
//
#include "Terrain/TerrainFile.h"
#include "Graphics/Graphics.h"
#include "Graphics/Shader.h"
#include "Miscellaneous/Fileio.h"
#include "Terrain/Heightmap.h"
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

	WriteTerrainFile( tileX, tileY );
}

/*
================================
WriteHeightmapFile
================================
*/
void WriteHeightmapFile( Vec4d * heightmap, int width, int tileX, int tileY ) {
	WriteHeightmapBinaryFile( heightmap, width, tileX, tileY );

#if 0
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
#endif
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




/*
================================================================

TerrainTileFile

================================================================
*/

/*
================================
WriteTerrainFile
================================
*/
void WriteTerrainFile( int tileX, int tileY ) {
	Vec4d * data = ReadHeightmapFile( TILE_SIZE, tileX, tileY );
	if ( NULL == data ) {
		CreateTerrainHeightmap( TILE_SIZE, tileX, tileY );
		data = ReadHeightmapFile( TILE_SIZE, tileX, tileY );
		assert( data );
		if ( NULL == data ) {
			return;
		}
	}

	Bounds bounds;

	// Convert the heightmap data to terrainverts
	unsigned long size = sizeof( vert_t ) * TERRAIN_ISLAND_SIZE * TERRAIN_ISLAND_SIZE;
	vert_t * verts = (vert_t *)malloc( size );
	if ( verts == NULL ) {
		printf( "wtf!?\n" );
	}

	char strBuffer[ 1024 ];
	sprintf( strBuffer, "generated/terrainFile%i%i.terra", tileX, tileY );
	if ( !OpenFileWriteStream( strBuffer ) ) {
		return;
	}

	//
	//	Write Heightmap data to file first
	//
	{
		float * heightmap = (float*)malloc( sizeof( float ) * TILE_SIZE * TILE_SIZE );
		for ( int y = 0; y < TILE_SIZE; y++ ) {
			for ( int x = 0; x < TILE_SIZE; x++ ) {
				int idx = x + y * TILE_SIZE;

				Vec3d pos = HeightmapToPos( data[ idx ], x, y, tileX, tileY );
				heightmap[ idx ] = pos.z;
			}
		}

		WriteFileStream( heightmap, sizeof( float ) * TILE_SIZE * TILE_SIZE );
		free( heightmap );
	}

	//
	//	Write island bounds data to file
	//
	{
		int numWritten = 0;

		extern int GetMaxDepth();
		const int maxDepth = GetMaxDepth();
		for ( int depth = 0; depth <= maxDepth; depth++ ) {
			const int terrainIslandsWide = ( 1 << depth );

			for ( int subTileY = 0; subTileY < terrainIslandsWide; subTileY++ ) {
				for ( int subTileX = 0; subTileX < terrainIslandsWide; subTileX++ ) {

					const int widthData = ( TILE_SIZE - 1 ) >> depth;
					const int skip = widthData / ( TERRAIN_ISLAND_SIZE - 1 );

					const int offsetX = subTileX * widthData;
					const int offsetY = subTileY * widthData;

					bounds.Clear();

					for ( int y = 0; y < TERRAIN_ISLAND_SIZE; y++ ) {
						for ( int x = 0; x < TERRAIN_ISLAND_SIZE; x++ ) {
							int x2 = x * skip + offsetX;
							int y2 = y * skip + offsetY;
							if ( x2 >= TILE_SIZE ) {
								x2 = TILE_SIZE - 1;
							}
							if ( y2 >= TILE_SIZE ) {
								y2 = TILE_SIZE - 1;
							}
 							int idx = x + y * TERRAIN_ISLAND_SIZE;
							int idx2 = x2 + y2 * TILE_SIZE;

							Vec3d pos = HeightmapToPos( data[ idx2 ], x2, y2, tileX, tileY );
							bounds.AddPoint( pos );
						}
					}

					// Write the bounds to file
					WriteFileStream( &bounds, sizeof( bounds ) );
					numWritten++;
				}
			}
		}
	}

	//
	//	Write island verts to file
	//
	int numWritten = 0;
	{
		extern int GetMaxDepth();
		const int maxDepth = GetMaxDepth();
		for ( int depth = 0; depth <= maxDepth; depth++ ) {
			const int terrainIslandsWide = ( 1 << depth );

			for ( int subTileY = 0; subTileY < terrainIslandsWide; subTileY++ ) {
				for ( int subTileX = 0; subTileX < terrainIslandsWide; subTileX++ ) {

					const int widthData = ( TILE_SIZE - 1 ) >> depth;
					const int skip = widthData / ( TERRAIN_ISLAND_SIZE - 1 );

					const int offsetX = subTileX * widthData;
					const int offsetY = subTileY * widthData;

					for ( int y = 0; y < TERRAIN_ISLAND_SIZE; y++ ) {
						for ( int x = 0; x < TERRAIN_ISLAND_SIZE; x++ ) {
							int x2 = x * skip + offsetX;
							int y2 = y * skip + offsetY;
							if ( x2 >= TILE_SIZE ) {
								x2 = TILE_SIZE - 1;
							}
							if ( y2 >= TILE_SIZE ) {
								y2 = TILE_SIZE - 1;
							}
 							int idx = x + y * TERRAIN_ISLAND_SIZE;
							int idx2 = x2 + y2 * TILE_SIZE;

							vert_t vert = HeightmapToVert( data[ idx2 ], x2, y2, tileX, tileY );

							verts[ idx ] = vert;
						}
					}

					// Re-calculate the normals to smooth it out (otherwise the sampling on the course lods will look bad)
					for ( int y = 1; y < TERRAIN_ISLAND_SIZE - 1; y++ ) {
						for ( int x = 1; x < TERRAIN_ISLAND_SIZE - 1; x++ ) {
							int idx_x0 = ( x - 1 ) + ( y - 0 ) * TERRAIN_ISLAND_SIZE;
							int idx_x1 = ( x + 1 ) + ( y + 0 ) * TERRAIN_ISLAND_SIZE;

							// The y-axis has to be flipped because of the weirdness with the heightmap generation.
							// The heightmap is in a left handed coordinate system.  But the geo is in a right handed one.
							int idx_y0 = ( x - 0 ) + ( y + 1 ) * TERRAIN_ISLAND_SIZE;
							int idx_y1 = ( x + 0 ) + ( y - 1 ) * TERRAIN_ISLAND_SIZE;

							Vec3d x0 = verts[ idx_x0 ].pos;
							Vec3d x1 = verts[ idx_x1 ].pos;
							Vec3d y0 = verts[ idx_y0 ].pos;
							Vec3d y1 = verts[ idx_y1 ].pos;

							Vec3d dx = x1 - x0;
							Vec3d dy = y1 - y0;

							Vec3d norm = dx.Cross( dy );
							norm.Normalize();

							int idx = x + y * TERRAIN_ISLAND_SIZE;
							Vec3dToByte4_n11( verts[ idx ].norm, norm );
						}
					}

					// Write the island to file
					WriteFileStream( verts, sizeof( vert_t ) * TERRAIN_ISLAND_SIZE * TERRAIN_ISLAND_SIZE );
					numWritten++;
				}
			}
		}
	}

	CloseFileWriteStream();

	free( verts );
	free( data );
	printf( "Terrain contains %i islands\n", numWritten );
	printf( "Done building terrain\n" );
}


/*
================================
TerrainTileFile::WriteFile
================================
*/
bool TerrainTileFile::WriteFile( TerrainTile * tile ) {
	if ( NULL == tile ) {
		return false;
	}

	return false;
}

/*
================================
TerrainTileFile::ReadHeightmap
================================
*/
bool TerrainTileFile::ReadHeightmap( float * heightmap ) {
	if ( NULL == m_file ) {
		return false;
	}

	unsigned int heightmapSize = sizeof( float ) * TILE_SIZE * TILE_SIZE;

	fseek( m_file, 0, SEEK_SET );

	unsigned int bytesRead = (unsigned int)fread( heightmap, sizeof( unsigned char ), heightmapSize, m_file );
    fflush( m_file );

	if ( bytesRead != heightmapSize ) {
		return false;
	}

	return true;
}

/*
================================
TerrainTileFile::ReadBounds
================================
*/
bool TerrainTileFile::ReadBounds( Bounds ** buffer ) {
	if ( NULL == m_file ) {
		return false;
	}

	unsigned int heightmapOffset = sizeof( float ) * TILE_SIZE * TILE_SIZE;

	extern int GetTerrainIslandsPerTile();
	unsigned int numIslandsPerTile = (unsigned int)GetTerrainIslandsPerTile();
	unsigned int boundsBufferSize = sizeof( Bounds ) * numIslandsPerTile;

	*buffer = (Bounds*)malloc( boundsBufferSize );
	if ( NULL == *buffer ) {
		return false;
	}

	fseek( m_file, heightmapOffset, SEEK_SET );

	unsigned int bytesRead = (unsigned int)fread( *buffer, sizeof( unsigned char ), boundsBufferSize, m_file );
    fflush( m_file );

	if ( bytesRead != boundsBufferSize ) {
		return false;
	}

	return true;
}

/*
================================
TerrainTileFile::ReadTerrainIsland
================================
*/
bool TerrainTileFile::ReadTerrainIsland( int x, int y, int depth, void * buffer ) {
	if ( NULL == m_file ) {
		return false;
	}

	unsigned int heightmapOffset = sizeof( float ) * TILE_SIZE * TILE_SIZE;

	extern int GetTerrainIslandsPerTile();
	unsigned int numIslandsPerTile = (unsigned int)GetTerrainIslandsPerTile();
	unsigned int boundsOffset = sizeof( Bounds ) * numIslandsPerTile;

	unsigned int islandSize = sizeof( vert_t ) * TERRAIN_ISLAND_SIZE * TERRAIN_ISLAND_SIZE;

	extern int GetIslandID( int depth, int x, int y );
	int islandId = GetIslandID( depth, x, y );

	unsigned int offset = islandSize * (unsigned int)islandId + heightmapOffset + boundsOffset;

	fseek( m_file, offset, SEEK_SET );

	unsigned int bytesRead = (unsigned int)fread( buffer, sizeof( unsigned char ), islandSize, m_file );
    fflush( m_file );

	if ( bytesRead != islandSize ) {
		return false;
	}

	return true;
}


/*
================================
TerrainTileFile::OpenFile
================================
*/
bool TerrainTileFile::OpenFile( int tileX, int tileY ) {
	char strBuffer[ 1024 ];
	sprintf( strBuffer, "generated/terrainFile%i%i.terra", tileX, tileY );

	// open file for reading
	printf( "opening file: %s\n", strBuffer );
	m_file = fopen( strBuffer, "rb" );
	
	// handle any errors
	if ( m_file == NULL ) {
		printf( "ERROR: open file failed: %s\n", strBuffer );
		return false;
	}
	
	// get file size
	fseek( m_file, 0, SEEK_END );
	fflush( m_file );
	m_size = ftell( m_file );
	fflush( m_file );
	rewind( m_file );
	fflush( m_file );

	return true;
}
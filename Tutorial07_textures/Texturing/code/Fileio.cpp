/*
 *  Fileio.cpp
 *
 */

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

static char gApplicationDirectory[ FILENAME_MAX ];	// needs to be CWD
static bool gWasInitialized = false;

/*
 =================================
 Initialize
 Grabs the application's directory
 =================================
 */
static void Initialize() {
	if ( gWasInitialized ) {
		return;
	}
	gWasInitialized = true;
	
	bool result = GetCurrentDir( gApplicationDirectory, sizeof( gApplicationDirectory ) );
	assert( result );
	if ( result ) {
		printf( "ApplicationDirectory: %s\n", gApplicationDirectory );
	} else {
		printf( "ERROR: Unable to get current working directory!\n");
	}
}

/*
 =================================
 GetFileData
 Opens the file and stores it in data
 =================================
 */
bool GetFileData( const char * fileName, unsigned char ** data, unsigned int & size ) {
	Initialize();

	FILE * file = NULL;

	char newFileName[ 2048 ];
	sprintf( newFileName, "%s/../%s", gApplicationDirectory, fileName );
	
	// open file for reading
	printf( "opening file: %s\n", newFileName );
	file = fopen( newFileName, "rb" );
	
	// handle any errors
	if ( file == NULL ) {
		printf("ERROR: open file failed: %s\n", newFileName );
		return false;
	}
	
	// get file size
	fseek( file, 0, SEEK_END );
	fflush( file );
	size = ftell( file );
	fflush( file );
	rewind( file );
	fflush( file );
	
	// output file size
	printf( "file size: %u\n", size );
	
	// create the data buffer
	*data = (unsigned char*)malloc( ( size + 1 ) * sizeof( unsigned char ) );
    
	// handle any errors
	if ( *data == NULL ) {
		printf( "ERROR: Could not allocate memory!\n" );
		fclose( file );
		return false;
	}

	// zero out the memory
	memset( *data, 0, ( size + 1 ) * sizeof( unsigned char ) );
	
	// read the data
	unsigned int bytesRead = fread( *data, sizeof( unsigned char ), size, file );
    fflush( file );
	printf( "total bytes read: %u\n", bytesRead );
    
    assert( bytesRead == size );
	
	// handle any errors
	if ( bytesRead != size ) {
		printf( "ERROR: reading file went wrong\n" );
		fclose( file );
		if ( *data != NULL ) {
			free( *data );
		}
		return false;
	}
	
	fclose( file );
	printf( "Read file was success\n");
	return true;	
}

/*
 *  Fileio.h
 *
 */

#pragma once

// bool GetFileData( const char * fileName, unsigned char ** data, unsigned int & size );
// bool RelativePathToFullPath( const char * relativePath, char fullPath[ 2048 ] );


void RelativePathToFullPath( const char * relativePathName, char * fullPath );
bool GetFileData( const char * fileName, unsigned char ** data, unsigned int & size );
bool WriteFileData( const char * fileName, const void * data, const unsigned int size );

bool OpenFileWriteStream( const char * fileName );
bool WriteFileStream( const void * data, const unsigned int size );
bool WriteFileStream( const char * data );
void CloseFileWriteStream();
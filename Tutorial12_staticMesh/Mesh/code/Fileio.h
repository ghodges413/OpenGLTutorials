/*
 *  Fileio.h
 *
 */

#pragma once

bool GetFileData( const char * fileName, unsigned char ** data, unsigned int & size );
bool RelativePathToFullPath( const char * relativePath, char fullPath[ 2048 ] );
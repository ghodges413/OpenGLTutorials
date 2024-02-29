//
//	Graphics.cpp
//
#include "Graphics/Graphics.h"

#include <stdio.h>
#include <assert.h>

/*
 ===================================
 myglClearErrors
 ===================================
 */
void myglClearErrors() {
	for ( int i = 0; i < 10; i++ ) {
		glGetError();
	}
}

/*
 ===================================
 myglGetError
 ===================================
 */
void myglGetError() {
	// check for up to 10 errors pending
	for ( int i = 0; i < 10; i++ ) {
		const int err = glGetError();
		if ( err == GL_NO_ERROR ) {
			return;
		}
		switch( err ) {
			case GL_INVALID_ENUM:
				printf( "GL_INVALID_ENUM\n" );
				break;
			case GL_INVALID_VALUE:
				printf( "GL_INVALID_VALUE\n" );
				break;
			case GL_INVALID_OPERATION:
				printf( "GL_INVALID_OPERATION\n" );
				break;
			case GL_OUT_OF_MEMORY:
				printf( "GL_OUT_OF_MEMORY\n" );
				break;
			case GL_INVALID_FRAMEBUFFER_OPERATION:
				printf( "GL_INVALID_FRAMEBUFFER_OPERATION\n" );
				break;
			default:
				printf( "unknown GL error: 0x%x\n", err );
				break;
		}
		assert( false );
	}
}

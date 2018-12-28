/*
 *  RenderSurface.cpp
 *
 */
#include "RenderSurface.h"
#include "Graphics.h"

#include <stdio.h>
#include <assert.h>

/*
 ================================
 CheckFrameBufferStatus
 ================================
 */
static bool CheckFrameBufferStatus() {
	GLuint status = glCheckFramebufferStatusEXT( GL_FRAMEBUFFER_EXT );
	if ( GL_FRAMEBUFFER_COMPLETE_EXT != status ) {
		// didn't work
		printf( "ERROR: surface failed to build 0x%x\n", status );
        switch ( status ) {
            case GL_FRAMEBUFFER_COMPLETE_EXT:						printf( "GL_FRAMEBUFFER_COMPLETE_EXT\n" );                      break;
            case 0x8CDB:											printf( "GL_FRAMEBUFFER_EXT_INCOMPLETE_DRAW_BUFFER_EXT\n" );    break;
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:			printf( "GL_FRAMEBUFFER_EXT_INCOMPLETE_ATTACHMENT\n" );         break;
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:	printf( "GL_FRAMEBUFFER_EXT_INCOMPLETE_MISSING_ATTACHMENT\n" ); break;
            case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:			printf( "GL_FRAMEBUFFER_EXT_INCOMPLETE_DIMENSIONS\n" );         break;
            case GL_FRAMEBUFFER_UNSUPPORTED_EXT:					printf( "GL_FRAMEBUFFER_EXT_UNSUPPORTED\n" );                   break;	
            default:												printf( "Unknown issue (%x).\n", status );							break;	
        }
		assert( false );
		return false;
	}

	return true;
}

/*
 ================================
 RenderSurface::RenderSurface
 ================================
 */
RenderSurface::RenderSurface() :
mFBO( 0 ),
mColorTexture( 0 ),
mDepthTexture( 0 ),
mWidth( 0 ),
mHeight( 0 ) {
}

/*
 ================================
 RenderSurface::~RenderSurface
 ================================
 */
RenderSurface::~RenderSurface() {
	if ( mColorTexture ) {
		glDeleteTextures( 1, &mColorTexture );
		mColorTexture = 0;
	}
	if ( mDepthTexture ) {
		glDeleteTextures( 1, &mDepthTexture );
		mDepthTexture = 0;
	}
	if ( mFBO ) {
		glDeleteFramebuffersEXT( 1, &mFBO );
		mFBO = 0;
	}
}

/*
 ================================
 RenderSurface::CreateSurface
 ================================
 */
bool RenderSurface::CreateSurface( const int flags, const int width, const int height ) {
	assert( 0 == mFBO );
	assert( 0 == mColorTexture );
	assert( 0 == mDepthTexture );

	//
	// create FBO
	//
    glGenFramebuffersEXT( 1, &mFBO );
    glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, mFBO );
    
	//
    //  Color texture
    //
    if ( flags & RS_COLOR_BUFFER ) {
        // Diffuse
        glGenTextures( 1, &mColorTexture );
        glBindTexture( GL_TEXTURE_2D, mColorTexture );

		const int internalFormat	= GL_RGBA8;
		const int format			= GL_RGBA;
		const int data_type			= GL_FLOAT;
		
		// NULL means reserve texture memory, but texels are undefined
        glTexImage2D( GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, data_type, NULL );

        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

        glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT + 0, GL_TEXTURE_2D, mColorTexture, 0 );
    }
    
	//
	// Depth texture
	//
	if ( flags & RS_DEPTH_BUFFER ) {
        // generate depth texture
		glGenTextures( 1, &mDepthTexture );
		glBindTexture( GL_TEXTURE_2D, mDepthTexture );
		
		// Using GL_DEPTH_COMPONENT instead of GL_DEPTH_COMPONENT32/24/16 allows the gpu to decide which bit depth
		const int internalFormat	= GL_DEPTH_COMPONENT;
		const int format			= GL_DEPTH_COMPONENT;
		const int data_type			= GL_UNSIGNED_INT;

        // NULL means reserve texture memory, but texels are undefined
		glTexImage2D( GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, data_type, NULL );

        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL );

        glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, mDepthTexture, 0 );
    }

	// Error handling
	const bool isValid = CheckFrameBufferStatus();
	if ( !isValid ) {
		return false;
	}
	    
    // clear bindings
	glBindTexture( GL_TEXTURE_2D, 0 );
	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
	
	mWidth = width;
	mHeight = height;
	printf( "surface built correctly width: %i  height:%i\n", mWidth, mHeight );
	return true;
}

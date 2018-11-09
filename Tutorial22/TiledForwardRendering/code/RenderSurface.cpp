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

		const int internalFormat	= GL_RGBA8;//GL_RGBA32F
		const int format			= GL_RGBA;
		const int data_type			= GL_UNSIGNED_BYTE;//GL_FLOAT;
		
		// NULL means reserve texture memory, but texels are undefined
        glTexImage2D( GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, data_type, NULL );

        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

        glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT + 0, GL_TEXTURE_2D, mColorTexture, 0 );
		myglGetError();
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
		myglGetError();
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


/*
 ================================================================
 
    GLSurface
 
 ================================================================
 */

/*
 ================================
 GLSurface::GetIndexFromBufferID
 ================================
 */
int GLSurface::GetIndexFromBufferID( const int buffID ) const {
    for ( int i = 0; i < mIDPairMap.Num(); ++i ) {
        if ( buffID == mIDPairMap[ i ].mFirst ) {
            return mIDPairMap[ i ].mSecond;
        }
    }
    return -1;
}

/*
 ================================
 GLSurface::GetBuffer
 ================================
 */
unsigned int GLSurface::GetBuffer( const int buffID ) const {
    const int idx = GetIndexFromBufferID( buffID );
    assert( idx >= 0 );
    return mBufferPairs[ idx ].mFirst;
}

/*
 ================================
 GLSurface::GetTexture
 ================================
 */
unsigned int GLSurface::GetTexture( const int buffID ) const {
    const int idx = GetIndexFromBufferID( buffID );
    assert( idx >= 0 );
    return mBufferPairs[ idx ].mSecond;
}

/*
 ================================
 GLSurface::HasBuffer
 ================================
 */
bool GLSurface::HasBuffer( const int buffID ) const {
    if ( GetIndexFromBufferID( buffID ) >= 0 ) {
        return true;
    }
    return false;
}

/*
 ================================
 GLSurface::SetDrawBuffers
 ================================
 */
void GLSurface::SetDrawBuffers() {
    glDrawBuffers( mBufferColorAttachments.Num(), mBufferColorAttachments.ToPtr() );
}

/*
 ================================
 GLSurface::Cleanup
 ================================
 */
void GLSurface::Cleanup() {
	//Delete resources
	for ( int i = 0; i < mBufferPairs.Num(); ++i ) {
		uintPair_t & pair = mBufferPairs[ i ];
		if ( pair.mFirst ) {
			glDeleteRenderbuffersEXT( 1, &pair.mFirst );
			pair.mFirst = 0;
		}
		if ( pair.mSecond ) {
			glDeleteTextures( 1, &pair.mSecond );
			pair.mSecond = 0;
		}
	}

	if ( mFBO ) {
		glDeleteFramebuffersEXT( 1, &mFBO );
		mFBO = 0;
	}
}

/*
 ================================================================
 
    GLSurface2d
 
 ================================================================
 */

/*
 ================================
 GLSurface2d::GLSurface2d
 ================================
 */
GLSurface2d::GLSurface2d() {
}

/*
 ================================
 GLSurface2d::~GLSurface2d
 ================================
 */
GLSurface2d::~GLSurface2d() {
	Cleanup();
}

/*
 ================================
 GLSurface2d::CreateSurface
 ================================
 */
bool GLSurface2d::CreateSurface( const int flags, const int * const dimensions ) {
	if ( NULL == dimensions ) {
		return false;
	}

	const int width = dimensions[ 0 ];
	const int height = dimensions[ 1 ];

    printf( "Building GLSurface buffer with   width: %i  height: %i\n", width, height );
    assert( width > 0 );
    assert( height > 0 );
    
    GLint maxNumColorAttachments = 0;
    glGetIntegerv( GL_MAX_COLOR_ATTACHMENTS_EXT, &maxNumColorAttachments );
    printf( "Maximum number of color buffer attachments:  %i\n", maxNumColorAttachments );
    
    //
    //  Generate buffers
    //
    
    glGenFramebuffersEXT( 1, &mFBO );
    glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, mFBO );
    myglGetError();
    
    unsigned int bufferID   = 0;
    unsigned int textureID  = 0;
    
    //
    //  Set and bind buffers
    //
    if ( flags & RS_COLOR_BUFFER ) {
        const int count = mBufferPairs.Num();
        glGenRenderbuffersEXT( 1, &bufferID );
        glBindRenderbufferEXT( GL_RENDERBUFFER_EXT, bufferID );
        glRenderbufferStorageEXT( GL_RENDERBUFFER_EXT, GL_RGB8, width, height );
        glFramebufferRenderbufferEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT + count, GL_RENDERBUFFER_EXT, bufferID );

        // Diffuse
        glGenTextures( 1, &textureID );
        glBindTexture( GL_TEXTURE_2D, textureID );
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
        glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT + count, GL_TEXTURE_2D, textureID, 0 );
        
        mIDPairMap.Append( intPair_t( RS_COLOR_BUFFER, mBufferPairs.Num() ) );
        mBufferPairs.Append( uintPair_t( bufferID, textureID ) );
        mBufferColorAttachments.Append( GL_COLOR_ATTACHMENT0_EXT + count );
        myglGetError();
    }
    if ( flags & RS_POSITION_BUFFER ) {
        const int count = mBufferPairs.Num();
        glGenRenderbuffersEXT( 1, &bufferID );
        glBindRenderbufferEXT( GL_RENDERBUFFER_EXT, bufferID );
        glRenderbufferStorageEXT( GL_RENDERBUFFER_EXT, GL_RGB32F_ARB, width, height );
        glFramebufferRenderbufferEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT + count, GL_RENDERBUFFER_EXT, bufferID );
        
        // Position
        glGenTextures( 1, &textureID );
        glBindTexture( GL_TEXTURE_2D, textureID );
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB32F_ARB, width, height, 0, GL_RGB, GL_FLOAT, NULL );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
        glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT + count, GL_TEXTURE_2D, textureID, 0 );
        
        mIDPairMap.Append( intPair_t( RS_POSITION_BUFFER, mBufferPairs.Num() ) );
        mBufferPairs.Append( uintPair_t( bufferID, textureID ) );
        mBufferColorAttachments.Append( GL_COLOR_ATTACHMENT0_EXT + count );
        myglGetError();
    }
    if ( flags & RS_NORMAL_BUFFER ) {
        const int count = mBufferPairs.Num();
        glGenRenderbuffersEXT( 1, &bufferID );
        glBindRenderbufferEXT( GL_RENDERBUFFER_EXT, bufferID );
        glRenderbufferStorageEXT( GL_RENDERBUFFER_EXT, GL_RGB16F_ARB, width, height );
        glFramebufferRenderbufferEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT + count, GL_RENDERBUFFER_EXT, bufferID );
        
        // Normal
        glGenTextures( 1, &textureID );
        glBindTexture( GL_TEXTURE_2D, textureID );
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB16F_ARB, width, height, 0, GL_RGB, GL_FLOAT, NULL );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
        glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT + count, GL_TEXTURE_2D, textureID, 0 );
        
        mIDPairMap.Append( intPair_t( RS_NORMAL_BUFFER, mBufferPairs.Num() ) );
        mBufferPairs.Append( uintPair_t( bufferID, textureID ) );
        mBufferColorAttachments.Append( GL_COLOR_ATTACHMENT0_EXT + count );
        myglGetError();
    }
    if ( flags & RS_SPECULAR_BUFFER ) {
        const int count = mBufferPairs.Num();
        glGenRenderbuffersEXT( 1, &bufferID );
        glBindRenderbufferEXT( GL_RENDERBUFFER_EXT, bufferID );
        glRenderbufferStorageEXT( GL_RENDERBUFFER_EXT, GL_R8, width, height );
        glFramebufferRenderbufferEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT + count, GL_RENDERBUFFER_EXT, bufferID );
        
        // Specular
        glGenTextures( 1, &textureID );
        glBindTexture( GL_TEXTURE_2D, textureID );
        glTexImage2D( GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, NULL );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
        glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT + count, GL_TEXTURE_2D, textureID, 0 );
        
        mIDPairMap.Append( intPair_t( RS_SPECULAR_BUFFER, mBufferPairs.Num() ) );
        mBufferPairs.Append( uintPair_t( bufferID, textureID ) );
        mBufferColorAttachments.Append( GL_COLOR_ATTACHMENT0_EXT + count );
        myglGetError();
    }
    if ( flags & RS_TANGENT_BUFFER ) {
        const int count = mBufferPairs.Num();
        glGenRenderbuffersEXT( 1, &bufferID );
        glBindRenderbufferEXT( GL_RENDERBUFFER_EXT, bufferID );
        glRenderbufferStorageEXT( GL_RENDERBUFFER_EXT, GL_RGB16F_ARB, width, height );
        glFramebufferRenderbufferEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT + count, GL_RENDERBUFFER_EXT, bufferID );
        
        // Tangent
        glGenTextures( 1, &textureID );
        glBindTexture( GL_TEXTURE_2D, textureID );
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB16F_ARB, width, height, 0, GL_RGB, GL_FLOAT, NULL );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
        glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT + count, GL_TEXTURE_2D, textureID, 0 );
        
        mIDPairMap.Append( intPair_t( RS_TANGENT_BUFFER, mBufferPairs.Num() ) );
        mBufferPairs.Append( uintPair_t( bufferID, textureID ) );
        mBufferColorAttachments.Append( GL_COLOR_ATTACHMENT0_EXT + count );
        myglGetError();
    }
    if ( flags & RS_DEPTH_BUFFER ) {
        bufferID = 0;
        // The render buffer generation was commented out so that copying the depth buffer worked on OSX.
//        glGenRenderbuffersEXT( 1, &bufferID );
//        glBindRenderbufferEXT( GL_RENDERBUFFER_EXT, bufferID );
//		glRenderbufferStorageEXT( GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, width, height );
//        glFramebufferRenderbufferEXT( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, bufferID );
        
        // generate depth texture
		glGenTextures( 1, &textureID );
		glBindTexture( GL_TEXTURE_2D, textureID );
        
		// NULL means reserve texture memory, but texels are undefined
		// Using GL_DEPTH_COMPONENT instead of GL_DEPTH_COMPONENT32/24/16 allows the gpu to decide which bit depth
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);
		
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL );
        glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, textureID, 0 );

        mIDPairMap.Append( intPair_t( RS_DEPTH_BUFFER, mBufferPairs.Num() ) );
        mBufferPairs.Append( uintPair_t( bufferID, textureID ) );
        myglGetError();
    }
    if ( flags & RS_STENCIL_BUFFER ) {
        //
        //	I've read that this is bad... that it's a major performance hit to create a stencil only buffer
        //  It's apparently best to use the packed Depth24Stencil8.  But I have no clue how true this is.
		//	Upon further reading, the hardware performs the depth stencil/depth check together.
		//	A big TODO: check for the stencil/depth buffer creation at the same time.  Prepare
		//	the buffer accordingly and use the depth24/stencil8 format.
        //
        glGenRenderbuffersEXT( 1, &bufferID );
        glBindRenderbufferEXT( GL_RENDERBUFFER_EXT, bufferID );
        glRenderbufferStorageEXT( GL_RENDERBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, width, height );
        glFramebufferRenderbufferEXT( GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, bufferID );
        
        mIDPairMap.Append( intPair_t( RS_STENCIL_BUFFER, mBufferPairs.Num() ) );
        mBufferPairs.Append( uintPair_t( bufferID, 0 ) );
        myglGetError();
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
	printf( "surface built correctly numBuffers: %i  width: %i  height: %i\n", mBufferPairs.Num(), mWidth, mHeight );
	myglGetError();
    return true;
}
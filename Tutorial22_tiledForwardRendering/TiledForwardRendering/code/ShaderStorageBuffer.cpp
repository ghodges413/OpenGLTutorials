/*
 *  ShaderStorageBuffer.cpp
 *
 */
#include "ShaderStorageBuffer.h"
#include "Graphics.h"
#include <assert.h>

/*
 ======================================
 ShaderStorageBuffer::ShaderStorageBuffer
 ======================================
 */
ShaderStorageBuffer::ShaderStorageBuffer() {
    mBufferObject = 0;
	mStructureSize = 0;
	mStructureCount = 0;
}

/*
 ======================================
 ShaderStorageBuffer::~ShaderStorageBuffer
 ======================================
 */
ShaderStorageBuffer::~ShaderStorageBuffer() {
    if ( mBufferObject > 0 ) {
        glDeleteBuffers( sNumBuffers, &mBufferObject );
        mBufferObject = 0;
    }
}

/*
 ======================================
 ShaderStorageBuffer::Generate
 ======================================
 */
bool ShaderStorageBuffer::Generate( const unsigned int structSize, const unsigned int count, const int flags ) {
#ifdef WINDOWS
    assert( 0 == mBufferObject );
    if ( mBufferObject > 0 ) {
        return false;
    }

	mStructureSize = structSize;
	mStructureCount = count;
	const unsigned int totalMemoryUsage = mStructureSize * mStructureCount * sNumBuffers;
	    
    glGenBuffers( 1, &mBufferObject );
	glBindBuffer( GL_SHADER_STORAGE_BUFFER, mBufferObject );
	glBufferStorage( GL_SHADER_STORAGE_BUFFER, totalMemoryUsage, NULL, flags );
#endif
	myglGetError();
	return true;
}

/*
 ======================================
 ShaderStorageBuffer::Generate
 ======================================
 */
bool ShaderStorageBuffer::Generate( const unsigned int structSize, const unsigned int count ) {
#ifdef WINDOWS
    assert( 0 == mBufferObject );
    if ( mBufferObject > 0 ) {
        return false;
    }

	mStructureSize = structSize;
	mStructureCount = count;
	const unsigned int totalMemoryUsage = mStructureSize * mStructureCount * sNumBuffers;
    
    // Usage values for buffer data:
	// Draw - The application sends data to GL
    // GL_STATIC_DRAW   = Upload data once and never update again
    // GL_DYNAMIC_DRAW  = Update data many times
    // GL_STREAM_DRAW   = Update a couple of times and discard

	// Read - GL sends data to the application
	// GL_STATIC_READ   = Upload data once and never update again
    // GL_DYNAMIC_READ  = Update data many times
    // GL_STREAM_READ   = Update a couple of times and discard

	// Copy - The applicationa sends data to GL and GL sends data to the application
	// GL_STATIC_COPY   = Upload data once and never update again
    // GL_DYNAMIC_COPY  = Update data many times
    // GL_STREAM_COPY   = Update a couple of times and discard
    
    glGenBuffers( 1, &mBufferObject );
	glBindBuffer( GL_SHADER_STORAGE_BUFFER, mBufferObject );
	glBufferData( GL_SHADER_STORAGE_BUFFER, totalMemoryUsage, NULL, GL_STATIC_DRAW );

	GLint bufMask = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT;	// the invalidate makes a big difference when re-writing

	void * data_ptr = MapBuffer( bufMask );
	if ( NULL == data_ptr ) {
		return false;
	}
	memset( data_ptr, 0, totalMemoryUsage );
	
	UnMapBuffer();    
	myglGetError();
	return true;
#else
    return false;
#endif
}

/*
 ======================================
 ShaderStorageBuffer::MapBuffer
 ======================================
 */
void * ShaderStorageBuffer::MapBuffer( const int bufferMask ) {
#ifdef WINDOWS
	assert( mBufferObject );
    if ( 0 == mBufferObject ) {
        return NULL;
    }

	const unsigned int totalMemoryUsage = mStructureSize * mStructureCount * sNumBuffers;

	void * data_ptr = glMapBufferRange( GL_SHADER_STORAGE_BUFFER, 0, totalMemoryUsage, bufferMask );
	myglGetError();
	return data_ptr;
#else
    return NULL;
#endif
}

#ifdef WINDOWS
/*
 ======================================
 ShaderStorageBuffer::UnMapBuffer
 ======================================
 */
void ShaderStorageBuffer::UnMapBuffer() {
	assert( mBufferObject );
    if ( 0 == mBufferObject ) {
        return;
    }

	glUnmapBuffer( GL_SHADER_STORAGE_BUFFER );
	myglGetError();
}

/*
 ======================================
 ShaderStorageBuffer::Bind
 ======================================
 */
void ShaderStorageBuffer::Bind( const unsigned int index ) {
	assert( mBufferObject );
    if ( 0 == mBufferObject ) {
        return;
    }

	glBindBufferBase( GL_SHADER_STORAGE_BUFFER, index, mBufferObject );
	myglGetError();
}

/*
 ======================================
 ShaderStorageBuffer::Bind
 ======================================
 */
void ShaderStorageBuffer::Bind() {
	assert( mBufferObject );
    if ( 0 == mBufferObject ) {
        return;
    }

	glBindBuffer( GL_SHADER_STORAGE_BUFFER, mBufferObject );
	myglGetError();
}
#endif


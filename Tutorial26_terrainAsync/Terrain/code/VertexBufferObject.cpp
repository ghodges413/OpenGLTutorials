/*
 *  VertexBufferObject.cpp
 *
 */
#include "VertexBufferObject.h"
#include "Graphics.h"
#include <assert.h>

/*
 ======================================
 VertexBufferObject::VertexBufferObject
 ======================================
 */
VertexBufferObject::VertexBufferObject() {
    mVBO = 0;
}

/*
 ======================================
 VertexBufferObject::~VertexBufferObject
 ======================================
 */
VertexBufferObject::~VertexBufferObject() {
    if ( mVBO > 0 ) {
        glDeleteBuffers( 1, &mVBO );
        myglGetError();
        mVBO = 0;
    }
}

/*
 ======================================
 VertexBufferObject::Cleanup
 ======================================
 */
void VertexBufferObject::Cleanup() {
	if ( mVBO > 0 ) {
        glDeleteBuffers( 1, &mVBO );
        myglGetError();
        mVBO = 0;
    }
}

/*
 ======================================
 VertexBufferObject::Generate
 ======================================
 */
void VertexBufferObject::Generate( const unsigned int type, const unsigned long dataSize, const void * const data_ptr, const unsigned int hint ) {
    assert( 0 == mVBO );
    if ( mVBO > 0 ) {
        return;
    }
    assert( GL_ARRAY_BUFFER == type || GL_ELEMENT_ARRAY_BUFFER == type );
    assert( GL_DYNAMIC_DRAW == hint || GL_STREAM_DRAW == hint || GL_STATIC_DRAW == hint );
    assert( dataSize > 0 );
    
    // GL_STATIC_DRAW   = Upload data once and never update again
    // GL_DYNAMIC_DRAW  = Update data many times
    // GL_STREAM_DRAW   = Update a couple of times and discard
    
    glGenBuffers( 1, &mVBO );
    myglGetError();
    
    mType = type;
    mHint = hint;
    mDataSize = dataSize;
    
    Bind();
    glBufferData( mType, mDataSize, data_ptr, mHint );
    UnBind();
}

/*
 ======================================
 VertexBufferObject::Update
 ======================================
 */
void VertexBufferObject::Update( const void * const data_ptr ) const {
    assert( GL_DYNAMIC_DRAW == mHint || GL_STREAM_DRAW == mHint );
    assert( IsValid() );
    
    Bind();
    
    glBufferData( mType, mDataSize, data_ptr, mHint );
    myglGetError();
    
    UnBind();
}

/*
 ======================================
 VertexBufferObject::Bind
 ======================================
 */
void VertexBufferObject::Bind() const {
    assert( mVBO > 0 );
    glBindBuffer( mType, mVBO );
    myglGetError();
}

/*
 ======================================
 VertexBufferObject::UnBind
 ======================================
 */
void VertexBufferObject::UnBind() const {
    glBindBuffer( mType, 0 );
    myglGetError();
}


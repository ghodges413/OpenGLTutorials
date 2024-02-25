//
//  VertexArrayObject
//
#include "Graphics/VertexArrayObject.h"
#include "Graphics/Graphics.h"
#include <assert.h>

unsigned int VertexArrayObject::sBoundVAO = 0;

/*
 ======================================
 VertexArrayObject::VertexArrayObject
 ======================================
 */
VertexArrayObject::VertexArrayObject() {
    mVAO = 0;
}

/*
 ======================================
 VertexArrayObject::~VertexArrayObject
 ======================================
 */
VertexArrayObject::~VertexArrayObject() {
    if ( mVAO > 0 ) {
        glDeleteVertexArrays( 1, &mVAO );
        myglGetError();
        mVAO = 0;
    }
}

/*
 ======================================
 VertexArrayObject::Cleanup
 ======================================
 */
void VertexArrayObject::Cleanup() {
	if ( mVAO > 0 ) {
        glDeleteVertexArrays( 1, &mVAO );
        myglGetError();
        mVAO = 0;
    }
}

/*
 ======================================
 VertexArrayObject::Generate
 ======================================
 */
void VertexArrayObject::Generate() {
    assert( 0 == mVAO );
    if ( 0 == mVAO ) {
        glGenVertexArrays( 1, &mVAO );
        myglGetError();
    }
}

/*
 ======================================
 VertexArrayObject::Bind
 ======================================
 */
void VertexArrayObject::Bind() const {
    assert( mVAO > 0 );

	// If this VAO is already bound, skip binding
	if ( sBoundVAO == mVAO ) {
		return;
	}

	// Bind this vao and update the static bound
    glBindVertexArray( mVAO );
	sBoundVAO = mVAO;

    myglGetError();
}

/*
 ======================================
 VertexArrayObject::UnBind
 ======================================
 */
void VertexArrayObject::UnBind() const {
    glBindVertexArray( 0 );
	sBoundVAO = 0;

    myglGetError();
}


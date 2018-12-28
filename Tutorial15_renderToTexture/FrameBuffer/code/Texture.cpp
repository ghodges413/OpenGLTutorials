/*
 *  Texture.cpp
 *
 */
#include "Texture.h"
#include "Graphics.h"

/*
 ===============================
 Texture::Texture
 ===============================
 */
Texture::Texture() :
mName( 0 ),
mWidth( 0 ),
mHeight( 0 ) {
}

/*
 ===============================
 Texture::~Texture
 ===============================
 */
Texture::~Texture() {
	if ( mName > 0 ) {
		glDeleteTextures( 1, &mName );
		mName = 0;
	}
}

/*
 ===============================
 Texture::InitWithData
 ===============================
 */
void Texture::InitWithData( const void * data, const int width, const int height ) {
	// Store the width and height of the texture
	mWidth = width;
	mHeight = height;

	// Generate the texture
	glGenTextures( 1, &mName );

	// Bind this texture so that we can modify and set it up
	glBindTexture( GL_TEXTURE_2D, mName );
	
	// Set the texture wrapping to clamp to edge
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

	// Setup the filtering between texels
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	const int internalFormat	= GL_RGBA8;
	const int format			= GL_RGBA;
	const int type				= GL_UNSIGNED_BYTE;

	// Upload the texture data to the GPU
	glTexImage2D( GL_TEXTURE_2D, 0, internalFormat, mWidth, mHeight, 0, format, type, data );

	// Reset the bound texture to nothing
	glBindTexture( GL_TEXTURE_2D, 0 );
}

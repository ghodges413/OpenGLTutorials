//
//  Texture.cpp
//
#include "Graphics/Texture.h"
#include "Graphics/Graphics.h"

/*
 ===============================
 Texture::Texture
 ===============================
 */
Texture::Texture() :
m_name( 0 ) {
}

/*
 ===============================
 Texture::~Texture
 ===============================
 */
Texture::~Texture() {
	if ( m_name > 0 ) {
		glDeleteTextures( 1, &m_name );
		m_name = 0;
	}
}

/*
 ===============================
 Texture::InitWithData
 ===============================
 */
void Texture::InitWithData( const void * data, const int width, const int height ) {
#if 1
	m_opts.wrapS = WM_CLAMP;
	m_opts.wrapR = WM_CLAMP;
	m_opts.wrapT = WM_CLAMP;
	m_opts.wrapS = WM_REPEAT;
	m_opts.wrapR = WM_REPEAT;
	m_opts.wrapT = WM_REPEAT;
	m_opts.minFilter = FM_LINEAR;
	m_opts.magFilter = FM_LINEAR;
	m_opts.minFilter = FM_LINEAR_MIPMAP_NEAREST;
	m_opts.magFilter = FM_LINEAR;
	m_opts.dimX = width;
	m_opts.dimY = height;
	m_opts.dimZ = 0;
	m_opts.type = TT_TEXTURE_2D;
	m_opts.format = FMT_RGBA8;
	InitializeFormat( data );
#else
	// Store the width and height of the texture
	m_width = width;
	m_height = height;

	// Generate the texture
	glGenTextures( 1, &m_name );

	// Bind this texture so that we can modify and set it up
	glBindTexture( GL_TEXTURE_2D, m_name );
	
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
	glTexImage2D( GL_TEXTURE_2D, 0, internalFormat, m_width, m_height, 0, format, type, data );

	// Reset the bound texture to nothing
	glBindTexture( GL_TEXTURE_2D, 0 );
#endif
}

/*
 ===============================
 Texture::Init
 ===============================
 */
void Texture::Init( TextureOpts_t opts, const void * data ) {
	m_opts = opts;
	InitializeFormat( data );
}

/*
 ===============================
 Texture::InitializeFormat
 ===============================
 */
void Texture::InitializeFormat( const void * data ) {
	if ( 0 == m_name ) {
		glGenTextures( 1, &m_name );
	}

	//
	// Setup the texture type
	//

	GLenum target = GL_TEXTURE_2D;
	switch ( m_opts.type ) {
		case TT_TEXTURE_1D: { target = GL_TEXTURE_1D; } break;
		case TT_TEXTURE_1D_ARRAY: { target = GL_TEXTURE_1D_ARRAY; } break;
		default:
		case TT_TEXTURE_2D: { target = GL_TEXTURE_2D; } break;
		case TT_TEXTURE_2D_ARRAY: { target = GL_TEXTURE_2D_ARRAY; } break;
		case TT_TEXTURE_3D: { target = GL_TEXTURE_3D; } break;
//		case TT_TEXTURE_CUBEMAP: { target = GL_TEXTURE_CUBE_MAP; } break;
//		case TT_TEXTURE_CUBEMAP_ARRAY: { target = GL_TEXTURE_CUBE_MAP_ARRAY; } break;
	};
	glBindTexture( target, m_name );

	//
	// Setup the wrapping mode
	//
	{
		GLint wrapS = ( WM_CLAMP == m_opts.wrapS ) ? GL_CLAMP_TO_EDGE : GL_REPEAT;
		glTexParameteri( target, GL_TEXTURE_WRAP_S, wrapS );

		if ( m_opts.type >= TT_TEXTURE_2D ) {
			GLint wrapT = ( WM_CLAMP == m_opts.wrapT ) ? GL_CLAMP_TO_EDGE : GL_REPEAT;
			glTexParameteri( target, GL_TEXTURE_WRAP_T, wrapT );
		}

		if ( m_opts.type == TT_TEXTURE_3D ) {
			GLint wrapR = ( WM_CLAMP == m_opts.wrapR ) ? GL_CLAMP_TO_EDGE : GL_REPEAT;
			glTexParameteri( target, GL_TEXTURE_WRAP_R, wrapR );
		}
	}

	//
	//	Setup the magnification/minification filters
	//
	{
		int minFilter;
		switch ( m_opts.minFilter ) {
			default:
			case FM_NEAREST: { minFilter = GL_NEAREST; } break;
			case FM_LINEAR: { minFilter = GL_LINEAR; } break;
			case FM_NEAREST_MIPMAP_NEAREST: { minFilter = GL_NEAREST_MIPMAP_NEAREST; } break;
			case FM_LINEAR_MIPMAP_NEAREST: { minFilter = GL_LINEAR_MIPMAP_NEAREST; } break;
			case FM_NEAREST_MIPMAP_LINEAR: { minFilter = GL_NEAREST_MIPMAP_LINEAR; } break;
			case FM_LINEAR_MIPMAP_LINEAR: { minFilter = GL_LINEAR_MIPMAP_LINEAR; } break;
		}
		int magFilter;
		switch ( m_opts.magFilter ) {
			default:
			case FM_NEAREST: { magFilter = GL_NEAREST; } break;
			case FM_LINEAR: { magFilter = GL_LINEAR; } break;
			case FM_NEAREST_MIPMAP_NEAREST: { magFilter = GL_NEAREST_MIPMAP_NEAREST; } break;
			case FM_LINEAR_MIPMAP_NEAREST: { magFilter = GL_LINEAR_MIPMAP_NEAREST; } break;
			case FM_NEAREST_MIPMAP_LINEAR: { magFilter = GL_NEAREST_MIPMAP_LINEAR; } break;
			case FM_LINEAR_MIPMAP_LINEAR: { magFilter = GL_LINEAR_MIPMAP_LINEAR; } break;
		}
		glTexParameterf( target, GL_TEXTURE_MIN_FILTER, minFilter );
		glTexParameterf( target, GL_TEXTURE_MAG_FILTER, magFilter );
	}

	// Check for mipmapping
	const bool doMipMapping = ( m_opts.minFilter > FM_LINEAR || m_opts.magFilter > FM_LINEAR );

	// TODO: probably check that this is supported?
	// Add Anisotropic Filtering
	if ( doMipMapping && GL_TEXTURE_2D == target ) {
		int maxA = 0;
		glGetIntegerv( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxA );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxA );
	}

	
	int internalFormat	= GL_RGBA8;
	int format			= GL_RGBA;
	int data_type		= GL_UNSIGNED_BYTE;
	switch ( m_opts.format ) {
		default:
		case FMT_RGBA8: {
			internalFormat = GL_RGBA8;
			format = GL_RGBA;
			data_type = GL_UNSIGNED_BYTE;
		} break;
		case FMT_RGB8: {
			internalFormat = GL_RGB8;
			format = GL_RGB;
			data_type = GL_UNSIGNED_BYTE;
		} break;
		case FMT_RGBA32F: {
			internalFormat = GL_RGBA32F;
			format = GL_RGBA;
			data_type = GL_FLOAT;
		} break;
		case FMT_RGB32F: {
			internalFormat = GL_RGB32F;
			format = GL_RGB;
			data_type = GL_FLOAT;
		} break;
		case FMT_RG32F: {
			internalFormat = GL_RG32F;
			format = GL_RG;
			data_type = GL_FLOAT;
		} break;
		case FMT_R32F: {
			internalFormat = GL_R32F;
			format = GL_RED;
			data_type = GL_FLOAT;
		} break;
		case FMT_RGBA16F: {
			internalFormat = GL_RGBA16F;
			format = GL_RGBA;
			data_type = GL_FLOAT;
		} break;
		case FMT_RGB16F: {
			internalFormat = GL_RGB16F;
			format = GL_RGB;
			data_type = GL_FLOAT;
		} break;
		case FMT_RG16F: {
			internalFormat = GL_RG16F;
			format = GL_RG;
			data_type = GL_FLOAT;
		} break;
		case FMT_R16F: {
			internalFormat = GL_R16F;
			format = GL_RED;
			data_type = GL_FLOAT;
		} break;
	};

	switch ( m_opts.type ) {
		case TT_TEXTURE_1D: { glTexImage1D( target, 0, internalFormat, m_opts.dimX, 0, format, data_type, data ); } break;
		default:
		case TT_TEXTURE_2D: { glTexImage2D( target, 0, internalFormat, m_opts.dimX, m_opts.dimY, 0, format, data_type, data ); } break;
		case TT_TEXTURE_2D_ARRAY:
		case TT_TEXTURE_3D: { glTexImage3D( target, 0, internalFormat, m_opts.dimX, m_opts.dimY, m_opts.dimZ, 0, format, data_type, data ); } break;
	}

	// TODO: Create your own mip-mapping generation code
	if ( doMipMapping ) {
		glGenerateMipmap( target );
	}

	// Unbind the texture
	glBindTexture( target, 0 );
	myglGetError();
}
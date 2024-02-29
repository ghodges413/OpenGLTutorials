//
//	TextureManager.cpp
//
#include "Graphics/TextureManager.h"
#include <stdlib.h>

TextureManager * g_textureManager = NULL;

/*
 ===============================
 TextureManager::TextureManager
 ===============================
 */
TextureManager::TextureManager() {
}

/*
 ===============================
 TextureManager::~TextureManager
 ===============================
 */
TextureManager::~TextureManager() {
	TextureContainer_t::iterator it = mTextures.begin();
	while ( it != mTextures.end() ) {
        printf( "removing texture: %s\n", it->first.cstr() );
        assert( it->second );
		delete it->second;
		it->second = NULL;
		
		++it;
	}
	mTextures.clear();
}

/*
 ===============================
 TextureManager::GetTexture
 ===============================
 */
Texture * TextureManager::GetTexture( const char * name, const TextureOpts_t & opts, const void * data ) {
	// attempt to find the texture in our container
	Texture * texture = GetCachedTexture( name );
	if ( texture ) {
		return texture;
	}
	
	// Create a texture using the incoming format and data
	texture = new Texture();
	texture->m_opts = opts;
	texture->InitializeFormat( data );	

	// store and return
	mTextures[ name ] = texture;
	return texture;
}

/*
 ===============================
 TextureManager::GetCachedTexture
 ===============================
 */
Texture * TextureManager::GetCachedTexture( const char * name ) {
	TextureContainer_t::iterator it = mTextures.find( name );
	if ( it != mTextures.end() ) {
		return it->second;
	}
	return NULL;
}
   


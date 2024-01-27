//
//	TextureManager.h
//
//	This is the start to a smarter way of managing texture data.
//
#pragma once
#include "Graphics/Texture.h"
#include "Miscellaneous/String.h"
#include <map>

/*
 ===============================
 TextureManager
 ===============================
 */
class TextureManager {
public:
	TextureManager();
	~TextureManager();
	
	Texture * GetCachedTexture( const char * name );
	Texture * GetTexture( const char * name, const TextureOpts_t & opts, const void * data );
	
private:
	typedef std::map< String, Texture * > TextureContainer_t;
	TextureContainer_t	mTextures;
};

extern TextureManager * g_textureManager;


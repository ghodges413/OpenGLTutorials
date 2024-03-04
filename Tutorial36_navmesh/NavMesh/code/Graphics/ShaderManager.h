//
//  ShaderManager.h
//
#pragma once
#include <map>
#include "Miscellaneous/String.h"
#include "Graphics/Shader.h"


/*
 ===============================
 hbShaderManager
 ===============================
 */
class hbShaderManager {
public:
	hbShaderManager();
	~hbShaderManager();
	
	Shader * GetShader( const char * const name );
    Shader * GetAndUseShader( const char * const name );

	bool RemoveShader( String name );
	
private:
	typedef std::map< String, Shader * > Shaders_t;
	
	Shaders_t	mShaders;
};

extern hbShaderManager * g_shaderManager;
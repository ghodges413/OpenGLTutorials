//
//	ShaderManager.cpp
//
#include "Graphics/ShaderManager.h"


hbShaderManager * g_shaderManager = NULL;

/*
 ===============================
 hbShaderManager::hbShaderManager
 ===============================
 */
hbShaderManager::hbShaderManager() {
}

/*
 ===============================
 hbShaderManager::~hbShaderManager
 ===============================
 */
hbShaderManager::~hbShaderManager() {
	Shaders_t::iterator it = mShaders.begin();
	while ( it != mShaders.end() ) {
		delete it->second;
		it->second = NULL;
		
		++it;
	}
	mShaders.clear();
}

/*
 ===============================
 hbShaderManager::GetShader
 ===============================
 */
Shader * hbShaderManager::GetShader( const char * const name ) {
    assert( name );
	String nameStr = name;
	nameStr.ToLower();
    
	Shaders_t::iterator it = mShaders.find( nameStr.cstr() );
	if ( it != mShaders.end() ) {
		// shader found!  return it!
        assert( NULL != it->second );
		return it->second;
	}

	// didn't find the shader... try loading it
	char computeShader[ 2048 ];
	char vertexShader[ 2048 ];
	char tessellationControlShader[ 2048 ];
	char tessellationEvaluationShader[ 2048 ];
	char geometryShader[ 2048 ];
	char fragmenShader[ 2048 ];
	sprintf( computeShader, "data/Shaders/%s.comp", nameStr.cstr() );
	sprintf( vertexShader, "data/Shaders/%s.vert", nameStr.cstr() );
	sprintf( tessellationControlShader, "data/Shaders/%s.cont", nameStr.cstr() );
	sprintf( tessellationEvaluationShader, "data/Shaders/%s.tess", nameStr.cstr() );
	sprintf( geometryShader, "data/Shaders/%s.geom", nameStr.cstr() );
    sprintf( fragmenShader, "data/Shaders/%s.frag", nameStr.cstr() );
	
    Shader * shader = new Shader();
	const bool result = shader->LoadFromFile( fragmenShader, vertexShader, tessellationControlShader, tessellationEvaluationShader, geometryShader, computeShader );
    assert( result );
	if ( false == result ) {
        // shader not found
        delete shader;
        return NULL;
	}
	printf( "Shader %s wasn't found.  Compiled Successfully\n", nameStr.cstr() );

	//shader->mShaderName = nameStr;
	
	// store shader and return
	mShaders[ nameStr ] = shader;
	return shader;
}

/*
 ===============================
 hbShaderManager::GetAndUseShader
 ===============================
 */
Shader * hbShaderManager::GetAndUseShader( const char * const name ) {
    assert( name );
    
    Shader * shader = GetShader( name );
    assert( shader );
    
    shader->UseProgram();
    return shader;
}

/*
 ===============================
 hbShaderManager::RemoveShader
 ===============================
 */
bool hbShaderManager::RemoveShader( String name ) {
	name.ToLower();

	// Find this element
	Shaders_t::iterator it = mShaders.find( name );
	if ( it == mShaders.end() ) {
		// Shader not found... it does not exist
		printf( "Shader %s couldn't be found to remove\n", name.cstr() );
		return false;
	}
	printf( "Shader %s found removing now\n", name.cstr() );

	// shader found! delete it!
    delete it->second;
	it->second = NULL;

	// Remove the shader from the map
	mShaders.erase( it );
	return true;
}




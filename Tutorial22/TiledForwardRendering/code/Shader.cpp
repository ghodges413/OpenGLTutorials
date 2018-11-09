/*
 *  Shader.cpp
 *
 */
#include "Shader.h"
#include <stdio.h>
#include "Fileio.h"
#include <assert.h>

static GLuint gCurrentProgram = 0;

/*
 ================================
 Shader::Shader
 ================================
 */
Shader::Shader() :
mShaderProgram( 0 ),
mHasCompute( false ) {
}

/*
 ================================
 Shader::~Shader
 ================================
 */
Shader::~Shader() {
	if ( mShaderProgram ) {
		glDeleteProgram( mShaderProgram );
		mShaderProgram = 0;
	}
}

/*
 ================================
 Shader::PrintLog
 ================================
 */
void Shader::PrintLog( const GLuint shader, const char * logname ) const {
    GLint logLength = 0;
    glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &logLength );

    if ( logLength > 0 ) {
        GLchar *log = (GLchar *)malloc( logLength );
        glGetShaderInfoLog( shader, logLength, &logLength, log );

        printf( "Shader %s log:\n%s\n", logname, log );
        free( log );
    }
}

/*
 ================================
 Shader::CompileShaderFromFile
 ================================
 */
bool Shader::CompileShaderFromFile( GLuint * shader, const GLenum type, const char * file ) {
	if ( NULL == file ) {
		return false;
	}

	printf( "Compiling Shader:  %s\n", file );
	GLint status;
	GLchar * source = NULL;
    
    // copy source from file
	unsigned int size( 0 );
	GetFileData( file, (unsigned char**)&source, size );
    if ( !source ) {
        printf("Failed to load vertex shader\n");
        return false;
    }
	
    // make sure we didn't grab bad data
	const int length = strlen( source );
    assert( length == size );
	if ( length > size ) {
		printf( "shader length was longer than its size... removing extra data\n" );
		source[ size ] = '\0';
	}
	const GLchar * source2 = (const GLchar *)source;
    
    // compile code
    *shader = glCreateShader( type );
    glShaderSource( *shader, 1, &source2, NULL );
    glCompileShader( *shader );
	PrintLog( *shader, "compile" );
    
    // check that everything is okay
    glGetShaderiv( *shader, GL_COMPILE_STATUS, &status );
    
    bool result = true;
    if ( status == 0 ) {
		printf( "SHADER COMPILE FAILED: Text sent to shader:\n\n%s\n\n", source );
        // free the source code once we're done with it
        glDeleteShader( *shader );
        result = false;
    }
    
    // free the source code once we're done with it
    free( source );
    
    return result;
}

/*
 ================================
 Shader::CompileShaderFromCSTR
 ================================
 */
bool Shader::CompileShaderFromCSTR( GLuint * shader, const GLenum type, const char * source ) {
	GLint status;
	const GLchar * source2 = (const GLchar *)source;
    
    // compile shader code
    *shader = glCreateShader( type );
    glShaderSource( *shader, 1, &source2, NULL );
    glCompileShader( *shader );
	PrintLog( *shader, "compile" );
    
    // check that everything is okay
    glGetShaderiv( *shader, GL_COMPILE_STATUS, &status );
    if ( status == 0 ) {
		printf( "SHADER COMPILE FAILED: Text sent to shader:\n\n%s\n\n", source );
        glDeleteShader( *shader );
        return false;
    }
    
    return true;
}

/*
 ================================
 Shader::LinkProgram
 ================================
 */
bool Shader::LinkProgram( GLuint prog ) {
    glLinkProgram( prog );

	GLint status = 0;
    glGetProgramiv( prog, GL_LINK_STATUS, &status );

    if ( status == 0 ) {
        static const int max_chars = 2048;
        int log_length = 0;
        GLchar log[max_chars];// = (GLchar *)malloc( logLength );
        glGetProgramInfoLog( prog, max_chars, &log_length, log );
        printf( "Program linking log:\n%s\n", log );
        return false;
	}
    
    return true;
}

/*
 ================================
 Shader::ValidateProgram
 ================================
 */
bool Shader::ValidateProgram( GLuint prog ) const {
    GLint status;
    
    glValidateProgram( prog );
	PrintLog( prog, "validate" );
	
    glGetProgramiv( prog, GL_VALIDATE_STATUS, &status );
    if ( status == 0 ) {
        return false;
	}
    
    return true;
}

/*
 ================================
 Shader::LoadFromFile
 ================================
 */
bool Shader::LoadFromFile( const char * fragmentShader, const char * vertexShader, const char * tessellationControlShader, const char * tessellationEvaluationShader, const char * geometryShader, const char * computeShader ) {
	GLuint compShader( 0 );
	GLuint vertShader( 0 );
	GLuint tctlShader( 0 );
	GLuint tevlShader( 0 );
	GLuint geomShader( 0 );
	GLuint fragShader( 0 );
    
    // Create shader program.
    mShaderProgram = glCreateProgram();
	myglGetError();

	//
	//	Compile Shaders
	//
#ifdef WINDOWS
    mHasCompute = CompileShaderFromFile( &compShader, GL_COMPUTE_SHADER, computeShader );
    // Standard rendering shaders are not allowed to be mixed with compute shaders (at least so I've read).
    // So if a compute shader is found, then do not attempt to load/link rendering shaders.
#else
    mHasCompute = false;
#endif
    
	if ( !mHasCompute ) {
		//
		//	Look for rendering shaders
		//

		// Create and compile vertex shader.
		if ( false == CompileShaderFromFile( &vertShader, GL_VERTEX_SHADER, vertexShader ) ) {
			printf( "Failed to compile vertex shader\n" );
			assert( false );
			return false;
		}
		
		// Create and compile tessellation control shader.  Tessellation shaders are optional.
		CompileShaderFromFile( &tctlShader, GL_TESS_CONTROL_SHADER, tessellationControlShader );

		// Create and compile tessellation evaluation shader.  Tessellation shaders are optional.
		CompileShaderFromFile( &tevlShader, GL_TESS_EVALUATION_SHADER, tessellationEvaluationShader );
		
		// Create and compile geometry shader.  Geometry shaders are optional.
		CompileShaderFromFile( &geomShader, GL_GEOMETRY_SHADER, geometryShader );

		// Create and compile fragment shader.  Fragment shaders are optional.
		CompileShaderFromFile( &fragShader, GL_FRAGMENT_SHADER, fragmentShader );
	}

	//
	//	Attach Shaders
	//

	// Attach vertex shader to program.
	if ( compShader ) {
		glAttachShader( mShaderProgram, compShader );
		myglGetError();
	}
	    
    // Attach vertex shader to program.
	if ( vertShader ) {
		glAttachShader( mShaderProgram, vertShader );
		myglGetError();
	}

	// Attach tessellation control shader to program.
	if ( tctlShader ) {
		glAttachShader( mShaderProgram, tctlShader );
		myglGetError();
	}

	// Attach tessellation evaluation shader to program.
	if ( tevlShader ) {
		glAttachShader( mShaderProgram, tevlShader );
		myglGetError();
	}

	// Attach geometry shader to program.
	if ( geomShader ) {
		glAttachShader( mShaderProgram, geomShader );
		myglGetError();
	}

    // Attach fragment shader to program.
	if ( fragShader ) {
		glAttachShader( mShaderProgram, fragShader );
		myglGetError();
	}

	//
    // Link program.
	//
	const bool didLink = LinkProgram( mShaderProgram );
	if ( !didLink ) {
        printf( "Failed to link program: %d", mShaderProgram );
    } else {
		printf( "Shader compile complete\n" );
	}

	//
    // Release shaders.  (we don't need them anymore)
	//
	if ( compShader ) {
		glDeleteShader( compShader );
	}
    if ( vertShader ) {
        glDeleteShader( vertShader );
	}
	if ( tctlShader ) {
		glDeleteShader( tctlShader );
	}
	if ( tevlShader ) {
		glDeleteShader( tevlShader );
	}
	if ( geomShader ) {
		glDeleteShader( geomShader );
	}
    if ( fragShader ) {
        glDeleteShader( fragShader );
	}
	myglGetError();
	
    return didLink;
#if 0
	myglGetError();
    GLuint vertShader;
	GLuint fragShader;
    
    // Create shader program.
    mShaderProgram = glCreateProgram();
	myglGetError();

    // Create and compile vertex shader.
	if ( false == CompileShaderFromFile( &vertShader, GL_VERTEX_SHADER, vertexShader ) ) {
        printf( "Failed to compile vertex shader\n" );
        assert( false );
        return false;
    }
    myglGetError();

    // Create and compile fragment shader.
	if ( false == CompileShaderFromFile( &fragShader, GL_FRAGMENT_SHADER, fragmentShader ) ) {
        printf( "Failed to compile fragment shader\n" );
        assert( false );
        return false;
    }
	myglGetError();
    
    // Attach vertex shader to program.
    glAttachShader( mShaderProgram, vertShader );
	myglGetError();

    // Attach fragment shader to program.
    glAttachShader( mShaderProgram, fragShader );
	myglGetError();

    // Link program.
	if ( !LinkProgram( mShaderProgram ) ) {
        printf( "Failed to link program: %d", mShaderProgram );
        
        if ( vertShader ) {
            glDeleteShader( vertShader );
            vertShader = 0;
        }
        if ( fragShader ) {
            glDeleteShader( fragShader );
            fragShader = 0;
        }
        if ( mShaderProgram ) {
            glDeleteProgram( mShaderProgram );
            mShaderProgram = 0;
        }
        
        return false;
    }
    myglGetError();
    // Release vertex and fragment shaders.
    if ( vertShader ) {
        glDeleteShader( vertShader );
	}
    if ( fragShader ) {
        glDeleteShader( fragShader );
	}
	myglGetError();
	printf( "Shader compile complete\n" );
    return true;
#endif
}

/*
 ================================
 Shader::DispatchCompute
 ================================
 */
void Shader::DispatchCompute( const unsigned int numGroupsX, const unsigned int numGroupsY, const unsigned int numGroupsZ ) {
	assert( IsCurrentProgram() );
	assert( mHasCompute );
#ifdef WINDOWS
	glDispatchCompute( numGroupsX, numGroupsY, numGroupsZ );
#endif
}

/*
 ================================
 Shader::GetUniform
 ================================
 */
GLuint Shader::GetUniform( const char * name ) {
	// didn't find this uniform... find it the slow way and store
	const int val = glGetUniformLocation( mShaderProgram, name );
	assert( val != -1 );
	return val;
}

/*
 ================================
 Shader::GetAttribute
 ================================
 */
int Shader::GetAttribute( const char * name ) {
	// didn't find this attribute... find it the slow way and store
	const int val = glGetAttribLocation( mShaderProgram, name );
	assert( val != -1 );
	return val;
}

/*
 ================================
 Shader::UseProgram
 ================================
 */
void Shader::UseProgram() {
    if ( gCurrentProgram != mShaderProgram ) {
        glUseProgram( mShaderProgram );
        gCurrentProgram = mShaderProgram;
    }
}

/*
 ================================
 Shader::IsCurrentProgram
 ================================
 */
bool Shader::IsCurrentProgram() const {
    return ( mShaderProgram == gCurrentProgram );
}

/*
 ================================
 Shader::SetUniform1i
 ================================
 */
void Shader::SetUniform1i( const char * uniform, const int count, const int * values ) {
    assert( IsCurrentProgram() );
    assert( uniform );
    assert( values );
	myglGetError();
    
    const int uniformID   = GetUniform( uniform );
    glUniform1iv( uniformID, count, values );
}

/*
 ================================
 Shader::SetUniform2i
 ================================
 */
void Shader::SetUniform2i( const char * uniform, const int count, const int * values ) {
    assert( IsCurrentProgram() );
    assert( uniform );
    assert( values );
	myglGetError();
    
    const int uniformID   = GetUniform( uniform );
    glUniform2iv( uniformID, count, values );
}

/*
 ================================
 Shader::SetUniform3i
 ================================
 */
void Shader::SetUniform3i( const char * uniform, const int count, const int * values ) {
    assert( IsCurrentProgram() );
    assert( uniform );
    assert( values );
	myglGetError();
    
    const int uniformID   = GetUniform( uniform );
    glUniform3iv( uniformID, count, values );
}

/*
 ================================
 Shader::SetUniform4i
 ================================
 */
void Shader::SetUniform4i( const char * uniform, const int count, const int * values ) {
    assert( IsCurrentProgram() );
    assert( uniform );
    assert( values );
	myglGetError();
    
    const int uniformID   = GetUniform( uniform );
    glUniform4iv( uniformID, count, values );
}

/*
 ================================
 Shader::SetUniform1f
 ================================
 */
void Shader::SetUniform1f( const char * uniform, const int count, const float * values ) {
    assert( IsCurrentProgram() );
    assert( uniform );
    assert( values );
	myglGetError();
    
    const int uniformID   = GetUniform( uniform );
    glUniform1fv( uniformID, count, values );
}

/*
 ================================
 Shader::SetUniform2f
 ================================
 */
void Shader::SetUniform2f( const char * uniform, const int count, const float * values ) {
    assert( IsCurrentProgram() );
    assert( uniform );
    assert( values );
	myglGetError();
    
    const int uniformID   = GetUniform( uniform );
    glUniform2fv( uniformID, count, values );
}

/*
 ================================
 Shader::SetUniform3f
 ================================
 */
void Shader::SetUniform3f( const char * uniform, const int count, const float * values ) {
    assert( IsCurrentProgram() );
    assert( uniform );
    assert( values );
	myglGetError();
    
    const int uniformID   = GetUniform( uniform );
    glUniform3fv( uniformID, count, values );
}

/*
 ================================
 Shader::SetUniform4f
 ================================
 */
void Shader::SetUniform4f( const char * uniform, const int count, const float * values ) {
    assert( IsCurrentProgram() );
    assert( uniform );
    assert( values );
	myglGetError();
    
    const int uniformID   = GetUniform( uniform );
    glUniform4fv( uniformID, count, values );
}

/*
 ================================
 Shader::SetUniformMatrix2f
 ================================
 */
void Shader::SetUniformMatrix2f( const char * uniform, const int count, const bool transpose, const float * values ) {
    assert( IsCurrentProgram() );
    assert( uniform );
    assert( values );
	myglGetError();
    
    const int uniformID = GetUniform( uniform );
    glUniformMatrix2fv( uniformID, count, transpose, values );
}

/*
 ================================
 Shader::SetUniformMatrix3f
 ================================
 */
void Shader::SetUniformMatrix3f( const char * uniform, const int count, const bool transpose, const float * values ) {
    assert( IsCurrentProgram() );
    assert( uniform );
    assert( values );
	myglGetError();
    
    const int uniformID = GetUniform( uniform );
    glUniformMatrix3fv( uniformID, count, transpose, values );
}

/*
 ================================
 Shader::SetUniformMatrix4f
 ================================
 */
void Shader::SetUniformMatrix4f( const char * uniform, const int count, const bool transpose, const float * values ) {
    assert( IsCurrentProgram() );
    assert( uniform );
    assert( values );
	myglGetError();
    
    const int uniformID = GetUniform( uniform );
    glUniformMatrix4fv( uniformID, count, transpose, values );
}

/*
 ================================
 Shader::SetAndBindUniformTexture
 ================================
 */
void Shader::SetAndBindUniformTexture(    const char * uniform,
                                            const int textureSlot,
                                            const GLenum textureTarget,
                                            const GLuint textureID ) {
    assert( IsCurrentProgram() );
    assert( textureSlot >= 0 );
    assert( uniform );
    myglGetError();

    const int uniformID = GetUniform( uniform );
    glActiveTexture( GL_TEXTURE0 + textureSlot);
    glUniform1i( uniformID, textureSlot );
    glBindTexture( textureTarget, textureID );
}

/*
 ================================
 Shader::SetVertexAttribPointer
 ================================
 */
void Shader::SetVertexAttribPointer(  const char * attribute,
                                        GLint size,
                                        GLenum type,
                                        GLboolean normalized,
                                        GLsizei stride,
                                        const GLvoid * pointer ) {
    assert( IsCurrentProgram() );
    assert( attribute );
//    assert( pointer ); // It's okay if the pointer is null valued, it means we're using vbo's
    myglGetError();

    // Enable and set
    const int attributeID   = GetAttribute( attribute );
    glEnableVertexAttribArray( attributeID );
    glVertexAttribPointer( attributeID, size, type, normalized, stride, pointer );
	myglGetError();
}

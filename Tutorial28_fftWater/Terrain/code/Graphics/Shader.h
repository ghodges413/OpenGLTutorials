//
//  Shader.h
//
#pragma once
#include "Graphics/Graphics.h"
#include "Miscellaneous/String.h"

/*
 ==============================
 Shader
 ==============================
 */
class Shader {
public:
	Shader();
	~Shader();
	
	bool LoadFromFile( const char * fragmentShader, const char * vertexShader, const char * tessellationControlShader, const char * tessellationEvaluationShader, const char * geometryShader, const char * computeShader );

	void DispatchCompute( const unsigned int numGroupsX, const unsigned int numGroupsY, const unsigned int numGroupsZ );

	GLuint GetShaderProgram() const { return mShaderProgram; }
	bool Validate() { return ValidateProgram( mShaderProgram ); }
	
	GLuint GetUniform( const char * name );
	int GetAttribute( const char * name );
    
    void UseProgram();
    bool IsCurrentProgram() const;
	void SetUniform1i( const char * uniform, const int count, const int * values );
    void SetUniform2i( const char * uniform, const int count, const int * values );
    void SetUniform3i( const char * uniform, const int count, const int * values );
    void SetUniform4i( const char * uniform, const int count, const int * values );
    void SetUniform1f( const char * uniform, const int count, const float * values );
    void SetUniform2f( const char * uniform, const int count, const float * values );
    void SetUniform3f( const char * uniform, const int count, const float * values );
    void SetUniform4f( const char * uniform, const int count, const float * values );
    void SetUniformMatrix2f( const char * uniform, const int count, const bool transpose, const float * values );
    void SetUniformMatrix3f( const char * uniform, const int count, const bool transpose, const float * values );
    void SetUniformMatrix4f( const char * uniform, const int count, const bool transpose, const float * values );
    void SetAndBindUniformTexture(  const char * uniform,
                                    const int textureSlot,
                                    const GLenum textureTarget,
                                    const GLuint textureID );
    void SetVertexAttribPointer( const char * attribute, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid * pointer );
	
private:
    void PrintLog( const GLuint shader, const char * logname ) const;
	bool CompileShaderFromFile( GLuint * shader, const GLenum type, const char * file );
    bool CompileShaderFromCSTR( GLuint * shader, const GLenum type, const char * source );
	bool LinkProgram( GLuint prog );
	bool ValidateProgram( GLuint prog ) const;
    bool GetFileContent( const char * file, String & str );
    String AddIncludes( const String & source, const String & fileName );
	
private:
	GLuint mShaderProgram;
	bool mHasCompute;
};

//
//  winmain.cpp
//
#include <string>
#include <stdio.h>

#ifdef WINDOWS
#include <GL/glew.h>
#include <GL/glut.h>
#else
#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#include <OpenGL/glu.h>
#endif

#include "Vector.h"
#include "Shader.h"
#include "Targa.h"
#include "Texture.h"
#include "MatrixOps.h"
#include "Mesh.h"

// Global storage of the window size
const int gScreenWidth  = 1200;
const int gScreenHeight = 720;

// The texture that resides on the GPU
Texture gTextureDiffuse;
Texture gTextureNormals;

// The shader program that runs on the GPU
Shader gShader;

// A class for holding very simple static meshes
Mesh gMesh;

/*
 ================================
 DrawFrame

 This is the function that is repeatedly called.
 And it is where all the magic happens.
 ================================
 */
void DrawFrame( void ) {
	// Bind the primary frame buffer for rendering
    glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );

	// Enabling color writing to the frame buffer
	glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );

	// Enable writing to the depth buffer
	glDepthMask( GL_TRUE );

	// Set the depth test function to less than or equal
	glDepthFunc( GL_LEQUAL );

	// Enable testing against the depth buffer
	glEnable( GL_DEPTH_TEST );
	//glDisable( GL_DEPTH_TEST ); // uncomment to disable the depth test
	
	// Set the OpenGL viewport to be the entire size of the window
    const int width = gScreenWidth;
    const int height = gScreenHeight;
	glViewport( 0, 0, width, height );

	// Clear the frame buffer color
	// Inputs are Red Green Blue Alpha (RGBA)
	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );

	// Clear previous frame values
	glClear( GL_COLOR_BUFFER_BIT );
	glClear( GL_DEPTH_BUFFER_BIT );

	// This will turn on back face culling.  Uncomment to turn on.
	glCullFace( GL_BACK );
	glEnable( GL_CULL_FACE );

	static float angle = 0;
	angle += 0.01f;

	// This sets the rotation matrix.
	float matRotate[ 16 ] = { 0 };
	myRotateY( angle, matRotate );

	// This sets the translation matrix
	float matTranslate[ 16 ] = { 0 };
	myTranslate( 0, 0, -10, matTranslate );

	// This sets the perspective projection matrix
	float matProj[ 16 ] = { 0 };
	const float fieldOfViewDegrees = 45;
	const float aspectRatio = static_cast< float >( gScreenHeight ) / static_cast< float >( gScreenWidth );
	const float nearDepth = 1;
	const float farDepth = 1000;
	myPerspective( fieldOfViewDegrees, aspectRatio, nearDepth, farDepth, matProj );

	// Concatenate the rotation and translation matrix.
	// This will make it so the cube is rotated and then translated.
	// Reversing the order of the multiply will translate the cube and then rotate it.
	float matModelWorld[ 16 ] = { 0 };
	myMatrixMultiply( matRotate, matTranslate, matModelWorld );
	//myMatrixMultiply( matTranslate, matRotate, matModelView );

	//
	//	Draw a single cube
	//

	// Set the shader program that'll be used to render the cube
	gShader.UseProgram();

	Vec3d rayToLight = Vec3d( 1, 0, 0 );
	rayToLight.Normalize();
	const Vec3d lightColor = Vec3d( 1, 1, 1 );

	// Send the light direction to the shader
	gShader.SetUniform3f( "rayToLight", 1, rayToLight.ToPtr() );

	// Send the light color to the shader
	gShader.SetUniform3f( "lightColor", 1, lightColor.ToPtr() );

	// Bind the texture to be rendered
	gShader.SetAndBindUniformTexture( "s_textureDiffuse", 0, GL_TEXTURE_2D, gTextureDiffuse.GetName() );
	gShader.SetAndBindUniformTexture( "s_textureNormals", 1, GL_TEXTURE_2D, gTextureNormals.GetName() );

	// Send the transformation matrix to the shader
	gShader.SetUniformMatrix4f( "matModelWorld", 1, false, matModelWorld );

	// Send the projection matrix to the shader
	gShader.SetUniformMatrix4f( "matProj", 1, false, matProj );
	
	// Update attribute values.
	const int stride = sizeof( vert_t );
	gShader.SetVertexAttribPointer( "position", 3, GL_FLOAT, 0, stride, gMesh.mVerts[ 0 ].pos.ToPtr() );
	gShader.SetVertexAttribPointer( "normal", 3, GL_FLOAT, 0, stride, gMesh.mVerts[ 0 ].norm.ToPtr() );
	gShader.SetVertexAttribPointer( "tangent", 3, GL_FLOAT, 0, stride, gMesh.mVerts[ 0 ].tang.ToPtr() );
	gShader.SetVertexAttribPointer( "st", 2, GL_FLOAT, 0, stride, gMesh.mVerts[ 0 ].st.ToPtr() );

	// Draw
	glDrawArrays( GL_TRIANGLES, 0, gMesh.mVerts.Num() );

	//
	//	Draw another cube
	//

	// Create a new rotation and translation matrix for this cube
	myRotateY( -angle, matRotate );
	myTranslate( 3.0f, 0, -20, matTranslate );

	// Concatenate the rotation and translation matrix.
	myMatrixMultiply( matRotate, matTranslate, matModelWorld );

	// Send the transformation matrix to the shader
	gShader.SetUniformMatrix4f( "matModelWorld", 1, false, matModelWorld );

	// We're going to draw the same model.
	// So we don't have to update the vertex attribute pointers in the shader.
	// The shader still has that data.

	// Draw
	glDrawArrays( GL_TRIANGLES, 0, gMesh.mVerts.Num() );

	//
	//	Finish and swap buffers
	//
    
	// Tell OpenGL to finish all the previous OpenGL commands before continuing
	glFinish();

	// Swap the back buffer to the front buffer (display all the OpenGL commands that just completed).
	glutSwapBuffers();
}

/*
 ================================
 main
 ================================
 */
int main( int argc, char ** argv ) {
	// Initialize GLUT
	glutInit( &argc, argv );

	// Tell GLUT to create a single display with Red Green Blue (RGB) color and a depth buffer.
	glutInitDisplayMode( GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH );

	// Tell GLUT the size of the desired window
	glutInitWindowSize( gScreenWidth, gScreenHeight );

	// Set the intial window position
	glutInitWindowPosition( 50, 50 );

	// Create the window
	glutCreateWindow( "OpenGL program" );

	// Initialize GLEW (this will set the proper bindings for all of our OpenGL calls).
	// GLEW is a third party library that connects our application with OpenGL drivers
	// that are installed on the current system.
	GLenum err = glewInit();

	// Check for any errors that may have occured during the initialization of GLEW
	if ( GLEW_OK != err ) {
		// Problem: glewInit failed, something is seriously wrong.
		fprintf( stderr, "Error: %s\n", glewGetErrorString( err ) );

		// Quit the program because it will not run without GLEW
		return 0;
	}

	// Print out the installed version of OpenGL on this system
	printf( "GL_VERSION:  %s\n", (const char *)glGetString( GL_VERSION ) );

	// Load the shader program that will be used for rendering
	gShader.LoadFromFile( "data/Shaders/lit.fsh", "data/Shaders/lit.vsh" );

	// Load the targa data from file
	Targa targaDiffuse;
	targaDiffuse.Load( "data/Images/StoneBrick_d.tga", true );

	// Create a texture on the GPU and initialize it with the targa data
	gTextureDiffuse.InitWithData( targaDiffuse.DataPtr(), targaDiffuse.GetWidth(), targaDiffuse.GetHeight() );

	// Load the targa data from file
	Targa targaNormals;
	targaNormals.Load( "data/Images/StoneBrick_n.tga", true );

	// Create a texture on the GPU and initialize it with the targa data
	gTextureNormals.InitWithData( targaNormals.DataPtr(), targaNormals.GetWidth(), targaNormals.GetHeight() );

	// Load a mesh for drawing
	//gMesh.Load( "data/Meshes/companion_cube.obj" );
	gMesh.Load( "data/Meshes/torus01.obj" );
	//gMesh.Load( "data/Meshes/sphere.obj" );

	//
	// Set GLUT function pointers here
	//

	// Setting the idle function to point to the DrawFrame function tells GLUT to call this function in GLUT's infinite loop
	glutIdleFunc( DrawFrame );
	
	//
	//	Perform the infinite loop
	//

	// Do the infinite loop.  This starts glut's inifinite loop.  It will call our draw function over and over
	glutMainLoop();

	// The code reaches here, when the application is quiting.
	return 0;
}

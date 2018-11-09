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
#include "RenderSurface.h"
#include "SpotLight.h"
#include "MD5Model.h"
#include "MD5Anim.h"
#include "Time.h"
#include "PointLight.h"

// Global storage of the window size
const int gScreenWidth  = 1200;
const int gScreenHeight = 720;

// The texture that resides on the GPU
Texture gTextureDiffuse;
Texture gTextureNormals;

// The shader program that runs on the GPU
Shader gShader;
Shader gShaderDeferred;
Shader gShaderSimple;
Shader gShaderPointLight;

Mesh gMeshPlane;
Texture gTexturePlaneDiffuse;
Texture gTexturePlaneNormals;

GLSurface2d gRenderSurface;
Array< PointLight > gPointLights;

// The animated mesh and an animation
MD5Model gModelMD5;
MD5Anim gAnimMD5;

Vec3d gCameraPos;
Vec3d gCameraUp;
Vec3d gCameraLook;
float gMatView[ 16 ];

/*
 ================================
 DrawFrame

 This is the function that is repeatedly called.
 And it is where all the magic happens.
 ================================
 */
void DrawFrame( void ) {
	// Calculate the timing
	static int timeLastFrame	= 0;
	const int time				= GetTimeMicroseconds();
	const float dt_us			= time - timeLastFrame;
	const float dt_ms			= dt_us * 0.001f;
	timeLastFrame				= time;
	static float runTimeSeconds = 0;
	runTimeSeconds += dt_ms * 0.001f;

	// Update the camera view matrix
	const float radius = 400;
	const float angle = runTimeSeconds * 0.3f;
	gCameraPos	= Vec3d( radius * cosf( angle ), radius * sinf( angle ), 100 );
	gCameraUp	= Vec3d( 0, 0, 1 );
	myLookAt( gCameraPos, Vec3d( 0, 0, 100 ), gCameraUp, gMatView );

	// Bind the off screen render surface for rendering
	//glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, gRenderSurface.GetFBO() );
	gRenderSurface.SetDrawBuffers();

	// Enabling color writing to the frame buffer
	glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );

	// Enable writing to the depth buffer
	glDepthMask( GL_TRUE );

	// Set the depth test function to less than or equal
	glDepthFunc( GL_LEQUAL );

	// Enable testing against the depth buffer
	glEnable( GL_DEPTH_TEST );

	glDisable( GL_BLEND );
	
	// Set the OpenGL viewport to be the entire size of the window
	glViewport( 0, 0, gScreenWidth, gScreenHeight );

	// Clear the frame buffer color
	// Inputs are Red Green Blue Alpha (RGBA)
	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );

	// Clear previous frame values
	glClear( GL_COLOR_BUFFER_BIT );
	glClear( GL_DEPTH_BUFFER_BIT );

	// This will turn on back face culling.  Uncomment to turn on.
	glCullFace( GL_BACK );
	glEnable( GL_CULL_FACE );

	// This sets the perspective projection matrix
	float matProj[ 16 ] = { 0 };
	const float fieldOfViewDegrees = 45;
	const float aspectRatio = static_cast< float >( gScreenHeight ) / static_cast< float >( gScreenWidth );
	const float nearDepth = 1;
	const float farDepth = 2000;
	myPerspective( fieldOfViewDegrees, aspectRatio, nearDepth, farDepth, matProj );

	//
	//	Draw a single mesh
	//

	// Set the shader program that'll be used to render the mesh
	gShaderDeferred.UseProgram();
	
	float matIdentity[ 16 ] = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	// Send the transformation matrix to the shader
	gShaderDeferred.SetUniformMatrix4f( "matModelWorld", 1, false, matIdentity );

	// Send the projection matrix to the shader
	gShaderDeferred.SetUniformMatrix4f( "matProj", 1, false, matProj );
	gShaderDeferred.SetUniformMatrix4f( "matView", 1, false, gMatView );

	//
	//	Draw the md5 model
	//
	{
		// Bind the texture to be rendered
		gShaderDeferred.SetAndBindUniformTexture( "s_diffuse", 0, GL_TEXTURE_2D, gTextureDiffuse.GetName() );
		gShaderDeferred.SetAndBindUniformTexture( "s_normal", 1, GL_TEXTURE_2D, gTextureNormals.GetName() );

		const int iframe = (int)( runTimeSeconds * gAnimMD5.FrameRate() );
		const md5Skeleton_t & skeleton = gAnimMD5[ iframe ];
		gModelMD5.PrepareMesh( skeleton );
		gModelMD5.UpdateVBO();
		gModelMD5.Draw();
	}

	//
	//	Draw the plane
	//
	{
		gShaderDeferred.SetAndBindUniformTexture( "s_diffuse", 0, GL_TEXTURE_2D, gTexturePlaneDiffuse.GetName() );
		gShaderDeferred.SetAndBindUniformTexture( "s_normal", 1, GL_TEXTURE_2D, gTexturePlaneNormals.GetName() );
		gMeshPlane.Draw();
	}
		
	//
	//	Draw the off screen texture to the screen
	//

	// Bind the primary frame buffer for rendering
    glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );

	// Enabling color writing to the frame buffer
	glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );

	// Disable testing against the depth buffer
	glDisable( GL_DEPTH_TEST );

	// Clear the frame buffer color
	// Inputs are Red Green Blue Alpha (RGBA)
	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );

	// Clear previous frame values
	glClear( GL_COLOR_BUFFER_BIT );
	glClear( GL_DEPTH_BUFFER_BIT );
	
    //
    //  Copy the geometry buffer's depth buffer to the primary frambuffer's depth
    //
    {
        const int width         = gScreenWidth;
        const int height        = gScreenHeight;
		const unsigned int fbo  = gRenderSurface.GetFBO();
        glBindFramebufferEXT( GL_READ_FRAMEBUFFER_EXT, fbo );
        glBindFramebufferEXT( GL_DRAW_FRAMEBUFFER_EXT, 0 );
        glBlitFramebufferEXT(   0, 0, width, height,
                                0, 0, width, height,
                                GL_DEPTH_BUFFER_BIT,
                                GL_NEAREST );
        glBindFramebufferEXT( GL_READ_FRAMEBUFFER_EXT, 0 );
        glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
        myglGetError();
    }

	//
	//	Debug draw
	//
	{
		// Positions for drawing to the lower left quarter of the screen
		Vec3d positions[ 4 ] = {
			Vec3d( -1, -0.5f, 0 ),
			Vec3d( -1, -1, 0 ),
			Vec3d( -0.5f, -1, 0 ),
			Vec3d( -0.5f, -0.5f, 0 )
		};

		Vec3d positions2[ 4 ] = {
			Vec3d( -1, 0, 0 ),
			Vec3d( -1, -0.5f, 0 ),
			Vec3d( -0.5f, -0.5f, 0 ),
			Vec3d( -0.5f, 0, 0 )
		};

		Vec3d positions3[ 4 ] = {
			Vec3d( -1, 0.5f, 0 ),
			Vec3d( -1, 0, 0 ),
			Vec3d( -0.5f, 0, 0 ),
			Vec3d( -0.5f, 0.5f, 0 )
		};

		Vec2d st[ 4 ] = {
			Vec2d( 0, 1 ),
			Vec2d( 0, 0 ),
			Vec2d( 1, 0 ),
			Vec2d( 1, 1 )
		};

		// Set the shader program that'll be used to render the offscreen texture
		gShaderSimple.UseProgram();

		// Update attribute values.
		gShaderSimple.SetVertexAttribPointer( "st", 2, GL_FLOAT, 0, 0, st );

		// Bind the off screen texture to be rendered
		gShaderSimple.SetVertexAttribPointer( "position", 3, GL_FLOAT, 0, 0, positions );
		gShaderSimple.SetAndBindUniformTexture( "s_texture", 0, GL_TEXTURE_2D, gRenderSurface.GetTexture( RS_NORMAL_BUFFER ) );
		glDrawArrays( GL_QUADS, 0, 4 );

		gShaderSimple.SetVertexAttribPointer( "position", 3, GL_FLOAT, 0, 0, positions2 );
		gShaderSimple.SetAndBindUniformTexture( "s_texture", 0, GL_TEXTURE_2D, gRenderSurface.GetTexture( RS_POSITION_BUFFER ) );
		glDrawArrays( GL_QUADS, 0, 4 );

		gShaderSimple.SetVertexAttribPointer( "position", 3, GL_FLOAT, 0, 0, positions3 );
		gShaderSimple.SetAndBindUniformTexture( "s_texture", 0, GL_TEXTURE_2D, gRenderSurface.GetTexture( RS_COLOR_BUFFER ) );
		glDrawArrays( GL_QUADS, 0, 4 );
	}

	//
	//	Draw lights
	//

	// Turn on additive blending
	glEnable( GL_BLEND );
	glBlendFunc( GL_ONE, GL_ONE );

	// Cull the front faces and render the back faces of our light
	glEnable( GL_CULL_FACE );
	glCullFace( GL_FRONT );

	// Don't write to the depth buffer, but use it, draw anything that's further away
	glEnable( GL_DEPTH_TEST );
	glDepthMask( GL_FALSE );
	glDepthFunc( GL_GEQUAL );

	gShaderPointLight.UseProgram();

	// Send the projection matrix to the shader
	gShaderPointLight.SetUniformMatrix4f( "matProj", 1, false, matProj );
	gShaderPointLight.SetUniformMatrix4f( "matView", 1, false, gMatView );

	gShaderPointLight.SetAndBindUniformTexture( "s_diffuse", 0, GL_TEXTURE_2D, gRenderSurface.GetTexture( RS_COLOR_BUFFER ) );
	gShaderPointLight.SetAndBindUniformTexture( "s_position", 1, GL_TEXTURE_2D, gRenderSurface.GetTexture( RS_POSITION_BUFFER ) );
	gShaderPointLight.SetAndBindUniformTexture( "s_normal", 2, GL_TEXTURE_2D, gRenderSurface.GetTexture( RS_NORMAL_BUFFER ) );
	
	for ( int i = 0; i < gPointLights.Num(); ++i ) {
		const PointLight & pointLight = gPointLights[ i ];

		gShaderPointLight.SetUniform3f( "u_lightPosition", 1, pointLight.mPosition.ToPtr() );
		gShaderPointLight.SetUniform3f( "u_lightColor", 1, pointLight.mColor.ToPtr() );
		gShaderPointLight.SetUniform1f( "u_scale", 1, &pointLight.mScale );
		gShaderPointLight.SetUniform1f( "u_lightIntensity", 1, &pointLight.mIntensity );
		gShaderPointLight.SetUniform1f( "u_maxDist", 1, &pointLight.mMaxDist );

		gPointLights[ i ].Draw();
	}

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
	gShaderDeferred.LoadFromFile( "data/Shaders/deferred.fsh", "data/Shaders/deferred.vsh" );
	gShaderSimple.LoadFromFile( "data/Shaders/simple.fsh", "data/Shaders/simple.vsh" );
	gShaderPointLight.LoadFromFile( "data/Shaders/deferredPointLight.fsh", "data/Shaders/deferredPointLight.vsh" );

	// Load the targa data from file
	Targa targaDiffuse;
	targaDiffuse.Load( "data/Images/hellknight_d.tga", true );

	// Create a texture on the GPU and initialize it with the targa data
	gTextureDiffuse.InitWithData( targaDiffuse.DataPtr(), targaDiffuse.GetWidth(), targaDiffuse.GetHeight() );

	// Load the targa data from file
	Targa targaNormals;
	targaNormals.Load( "data/Images/hellknight_n.tga", true );

	// Create a texture on the GPU and initialize it with the targa data
	gTextureNormals.InitWithData( targaNormals.DataPtr(), targaNormals.GetWidth(), targaNormals.GetHeight() );

	// Load animated mesh for drawing
	gModelMD5.Load( "data/Meshes/hellknight.md5mesh" );
	gAnimMD5.Load( "data/Anims/hellknight_idle2.md5anim" );
	
	// Setup the view matrix
	gCameraPos	= Vec3d( 300, 0, 70 );
	gCameraUp	= Vec3d( 0, 0, 1 );
	gCameraLook	= Vec3d( -1, 0, 0 );
	myLookAt( gCameraPos, gCameraPos + gCameraLook, gCameraUp, gMatView );

	gMeshPlane.Load( "data/Meshes/room01.obj" );

	// Load the targa data from file
	Targa targaDiffusePlane;
	targaDiffusePlane.Load( "data/Images/StoneBrick_d.tga", true );

	// Create a texture on the GPU and initialize it with the targa data
	gTexturePlaneDiffuse.InitWithData( targaDiffusePlane.DataPtr(), targaDiffusePlane.GetWidth(), targaDiffusePlane.GetHeight() );

	// Load the targa data from file
	Targa targaNormalsPlane;
	targaNormalsPlane.Load( "data/Images/StoneBrick_n.tga", true );

	// Create a texture on the GPU and initialize it with the targa data
	gTexturePlaneNormals.InitWithData( targaNormalsPlane.DataPtr(), targaNormalsPlane.GetWidth(), targaNormalsPlane.GetHeight() );

	for ( int i = 0; i < 22; ++i ) {
		PointLight pointLight;
		pointLight.SetIntensity( 3000 );

		Vec3d pos;

		switch ( i ) {
			default:
			case 0: { pos = Vec3d( 50, 0, 50 ); } break;
			case 1: { pos = Vec3d( 150, 150, 50 ); } break;
			case 2: { pos = Vec3d( -150, 150, 50 ); } break;
			case 3: { pos = Vec3d( -150, -150, 50 ); } break;
			case 4: { pos = Vec3d( 150, -150, 50 ); } break;
			case 5: { pos = Vec3d( 250, 0, 50 ); } break;
			case 6: { pos = Vec3d( 0, 250, 50 ); } break;
			case 7: { pos = Vec3d( -250, 0, 50 ); } break;
			case 8: { pos = Vec3d( 0, -250, 50 ); } break;
			case 9: { pos = Vec3d( -50, 0, 150 ); } break;
			case 10: { pos = Vec3d( 450, 0, 50 ); } break;
			case 11: { pos = Vec3d( 0, 450, 50 ); } break;
			case 12: { pos = Vec3d( -450, 0, 50 ); } break;
			case 13: { pos = Vec3d( 0, -450, 50 ); } break;
			case 14: { pos = Vec3d( 150, 150, 150 ); } break;
			case 15: { pos = Vec3d( -150, 150, 150 ); } break;
			case 16: { pos = Vec3d( -150, -150, 150 ); } break;
			case 17: { pos = Vec3d( 150, -150, 150 ); } break;
			case 18: { pos = Vec3d( 450, 450, 150 ); } break;
			case 19: { pos = Vec3d( -450, 450, 150 ); } break;
			case 20: { pos = Vec3d( -450, -450, 150 ); } break;
			case 21: { pos = Vec3d( 450, -450, 150 ); } break;
		};

		pointLight.mPosition = pos;
		gPointLights.Append( pointLight );
	}

	//
    // Build an offscreen g-buffer for deferred shading
    //
    {
        const int w = gScreenWidth;
        const int h = gScreenHeight;
        
        const int flags =   RS_DEPTH_BUFFER |
                            RS_COLOR_BUFFER |
                            RS_POSITION_BUFFER |
                            RS_NORMAL_BUFFER;
		const int dimensions[] = { w, h };
        gRenderSurface.CreateSurface( flags, dimensions );
    }
	
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

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
#include "ShaderStorageBuffer.h"

// Global storage of the window size
const int gScreenWidth  = 1200;
const int gScreenHeight = 720;

// The texture that resides on the GPU
Texture gTextureDiffuse;
Texture gTextureNormals;

// The shader program that runs on the GPU
Shader gShaderDepthOnly;
Shader gShaderBuildLightList;
Shader gShaderTiledDebug;
Shader gShaderTiledForward;

Mesh gMeshPlane;
Texture gTexturePlaneDiffuse;
Texture gTexturePlaneNormals;

RenderSurface gRenderSurface;
Array< PointLight > gPointLights;

// The animated mesh and an animation
MD5Model gModelMD5;
MD5Anim gAnimMD5;

Vec3d gCameraPos;
Vec3d gCameraUp;
Vec3d gCameraLook;
float gMatView[ 16 ];

// The shader storage buffer for the point lights that will be used in the tiled lighting shader
struct storageLight_t {
	Vec4d	mSphere;
	Vec4d	mColor;
};
ShaderStorageBuffer	gBufferPointLights;

const int gThreadWidth = 16;
const int gNumTiles = gScreenWidth * gScreenHeight / ( gThreadWidth * gThreadWidth ); // = 3375

// The shader storage buffer for the per tile light lists
const int gMaxLightsPerTile = 32;
struct lightList_t {
	int mNumLights;
	int mLightIds[ gMaxLightsPerTile ];
};
ShaderStorageBuffer gBufferLightList;

/*
 ================================
 DrawFrame

 This is the function that is repeatedly called.
 And it is where all the magic happens.
 ================================
 */
void DrawFrame( void ) {
	//
	//	Clear the per tile light list
	//
	{
		gBufferLightList.Bind();
		lightList_t * lightList = ( lightList_t * )gBufferLightList.MapBuffer( GL_MAP_WRITE_BIT );
		if ( lightList ) {
			for ( int i = 0; i < gNumTiles; ++i ) {
				lightList[ i ].mNumLights = 0;
				for ( int j = 0; j < gMaxLightsPerTile; ++j ) {
					lightList[ i ].mLightIds[ j ] = -1;
				}
			}
			gBufferLightList.UnMapBuffer();

			glMemoryBarrier( GL_SHADER_STORAGE_BARRIER_BIT );
		}
	}

	// Debug map lights
#if 0
	{
		gBufferPointLights.Bind();
		storageLight_t * bufferMappedPtr = ( storageLight_t * )gBufferPointLights.MapBuffer( GL_MAP_READ_BIT );
		glMemoryBarrier( GL_SHADER_STORAGE_BARRIER_BIT );
		if ( bufferMappedPtr ) {
			for ( int i = 0; i < gPointLights.Num(); ++i ) {
				Vec4d sphere = bufferMappedPtr[ i ].mSphere;
				Vec4d color = bufferMappedPtr[ i ].mColor;
				printf( "Sphere: %f %f %f %f\n", sphere.x, sphere.y, sphere.z, sphere.w );
			}
			gBufferPointLights.UnMapBuffer();

			glMemoryBarrier( GL_SHADER_STORAGE_BARRIER_BIT );
		}
	}
#endif

	// Calculate the timing
	static int timeLastFrame	= 0;
	const int time				= GetTimeMicroseconds();
	const float dt_us			= time - timeLastFrame;
	const float dt_ms			= dt_us * 0.001f;
	timeLastFrame				= time;
	static float runTimeSeconds = 0;
	runTimeSeconds += dt_ms * 0.001f;

	// Update the camera view matrix
	const float radius = 475;
	const float angle = runTimeSeconds * 0.3f;
	gCameraPos	= Vec3d( radius * cosf( angle ), radius * sinf( angle ), 75 );
	gCameraUp	= Vec3d( 0, 0, 1 );
	myLookAt( gCameraPos, Vec3d( 0, 0, 100 ), gCameraUp, gMatView );

	// Bind the off screen render surface for rendering
	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, gRenderSurface.GetFBO() );

	// Enabling color writing to the frame buffer
	glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );

	// Enable writing to the depth buffer
	glDepthMask( GL_TRUE );

	// Set the depth test function to less than or equal
	glDepthFunc( GL_LEQUAL );

	// Enable testing against the depth buffer
	glEnable( GL_DEPTH_TEST );

	// Disable blending
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
	gShaderDepthOnly.UseProgram();
	
	float matIdentity[ 16 ] = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	// Send the transformation matrix to the shader
	gShaderDepthOnly.SetUniformMatrix4f( "matModelWorld", 1, false, matIdentity );

	// Send the projection matrix to the shader
	gShaderDepthOnly.SetUniformMatrix4f( "matProj", 1, false, matProj );
	gShaderDepthOnly.SetUniformMatrix4f( "matView", 1, false, gMatView );

	// Draw the room
	gMeshPlane.Draw();

	//
	//	Update and draw the md5 model
	//
	{
		const int iframe = (int)( runTimeSeconds * gAnimMD5.FrameRate() );
		const md5Skeleton_t & skeleton = gAnimMD5[ iframe ];
		gModelMD5.PrepareMesh( skeleton );
		gModelMD5.UpdateVBO();
		gModelMD5.Draw();
	}
	
	//
	//	Fill our per tile light lists
	//
	{
		gShaderBuildLightList.UseProgram();

		gShaderBuildLightList.SetUniform1i( "screenWidth", 1, &gScreenWidth );
		gShaderBuildLightList.SetUniform1i( "screenHeight", 1, &gScreenHeight );

		const int maxLights = gPointLights.Num();
		gShaderBuildLightList.SetUniform1i( "maxLights", 1, &maxLights );

		float matProjInv[ 16 ] = { 0 };
		myMatrixInverse4x4( matProj, matProjInv );
		gShaderBuildLightList.SetUniformMatrix4f( "matView", 1, false, gMatView );
		gShaderBuildLightList.SetUniformMatrix4f( "matProjInv", 1, false, matProjInv );

		gShaderBuildLightList.SetAndBindUniformTexture( "textureDepth", 0, GL_TEXTURE_2D, gRenderSurface.GetDepthTexture() );

		gBufferPointLights.Bind( 1 );
		gBufferLightList.Bind( 2 );

		gShaderBuildLightList.DispatchCompute( gScreenWidth / gThreadWidth, gScreenHeight / gThreadWidth, 1 );
	}

	//
	//	Debug map and check light list
	//
#if 0
	{
		glMemoryBarrier( GL_SHADER_STORAGE_BARRIER_BIT );

		gBufferLightList.Bind();
		lightList_t * lightList = ( lightList_t * )gBufferLightList.MapBuffer( GL_MAP_READ_BIT );
		if ( lightList ) {
			for ( int i = 0; i < gNumTiles; ++i ) {
				const lightList_t & list = lightList[ i ];
				printf( "Tile: %i  numLights %i\n", i, list.mNumLights;
			}
			gBufferLightList.UnMapBuffer();

			glMemoryBarrier( GL_SHADER_STORAGE_BARRIER_BIT );
		}
	}
#endif

	//
	//	Perform Tiled Lighting
	//
	{
		// Turn off writing to the depth buffer, but keep using it
		glDepthMask( GL_FALSE );

		// Use the tiled forward shader
		gShaderTiledForward.UseProgram();

		// Set the work group size and screen width
		gShaderTiledForward.SetUniform1i( "workGroupSize", 1, &gThreadWidth );
		gShaderTiledForward.SetUniform1i( "screenWidth", 1, &gScreenWidth );

		// Bind the buffer point lights and light list
		gBufferPointLights.Bind( 1 );
		gBufferLightList.Bind( 2 );
		
		// Send the transformation matrix to the shader
		gShaderTiledForward.SetUniformMatrix4f( "matModelWorld", 1, false, matIdentity );

		// Send the projection matrix to the shader
		gShaderTiledForward.SetUniformMatrix4f( "matProj", 1, false, matProj );
		gShaderTiledForward.SetUniformMatrix4f( "matView", 1, false, gMatView );

		// Draw the room's geometry
		gShaderTiledForward.SetAndBindUniformTexture( "s_textureDiffuse", 0, GL_TEXTURE_2D, gTexturePlaneDiffuse.GetName() );
		gShaderTiledForward.SetAndBindUniformTexture( "s_textureNormals", 1, GL_TEXTURE_2D, gTexturePlaneNormals.GetName() );
		gMeshPlane.Draw();

		// Draw the animated character
		gShaderTiledForward.SetAndBindUniformTexture( "s_textureDiffuse", 0, GL_TEXTURE_2D, gTextureDiffuse.GetName() );
		gShaderTiledForward.SetAndBindUniformTexture( "s_textureNormals", 1, GL_TEXTURE_2D, gTextureNormals.GetName() );
		gModelMD5.Draw();
	}

	//
	//	Debug draw
	//
#if 1
	{
		// Turn on additive blending
		glEnable( GL_BLEND );
		glBlendFunc( GL_ONE, GL_ONE );

		// Turn off depth buffer
		glDisable( GL_DEPTH_TEST );
		glDepthMask( GL_FALSE );

		Vec3d positions[ 4 ] = {
			Vec3d( -1.0f, 1.0f, 0 ),
			Vec3d( -1.0f,-1.0f, 0 ),
			Vec3d(  1.0f,-1.0f, 0 ),
			Vec3d(  1.0f, 1.0f, 0 )
		};
		
		gShaderTiledDebug.UseProgram();
		gShaderTiledDebug.SetVertexAttribPointer( "position", 3, GL_FLOAT, 0, 0, positions );
		gShaderTiledDebug.SetUniform1i( "workGroupSize", 1, &gThreadWidth );
		gShaderTiledDebug.SetUniform1i( "screenWidth", 1, &gScreenWidth );
		gBufferLightList.Bind( 2 );
		glDrawArrays( GL_QUADS, 0, 4 );
	}
#endif

	// Bind the primary frame buffer for rendering
    //glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
	
    //
    //  Copy the render buffer's color buffer to the primary frambuffer
    //
    {
        const int width         = gScreenWidth;
        const int height        = gScreenHeight;
		const unsigned int fbo  = gRenderSurface.GetFBO();
        glBindFramebufferEXT( GL_READ_FRAMEBUFFER_EXT, fbo );
        glBindFramebufferEXT( GL_DRAW_FRAMEBUFFER_EXT, 0 );
        glBlitFramebufferEXT(   0, 0, width, height,
                                0, 0, width, height,
                                GL_COLOR_BUFFER_BIT,
                                GL_NEAREST );
        glBindFramebufferEXT( GL_READ_FRAMEBUFFER_EXT, 0 );
        glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
        myglGetError();
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
	glutInitDisplayMode( GLUT_SINGLE | GLUT_RGB );

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

	gRenderSurface.CreateSurface( RS_COLOR_BUFFER | RS_DEPTH_BUFFER, gScreenWidth, gScreenHeight );

	// Load the shader program that will be used for rendering
	gShaderDepthOnly.LoadFromFile( "data/Shaders/depthOnly.fsh", "data/Shaders/depthOnly.vsh", NULL, NULL, NULL, NULL );
	gShaderTiledDebug.LoadFromFile( "data/Shaders/tiledDebug.fsh", "data/Shaders/tiledDebug.vsh", NULL, NULL, NULL, NULL );
	gShaderTiledForward.LoadFromFile( "data/Shaders/tiledForward.fsh", "data/Shaders/tiledForward.vsh", NULL, NULL, NULL, NULL );
	gShaderBuildLightList.LoadFromFile( NULL, NULL, NULL, NULL, NULL, "data/Shaders/tiledBuildLightList.csh" );

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

	for ( int i = 0; i < 30; ++i ) {
		PointLight pointLight;
		pointLight.SetIntensity( 3000 );

		Vec3d pos;

		switch ( i ) {
			default:
			case 0: { pos = Vec3d( 50, 0, 50 ); } break;
			case 1: { pos = Vec3d( -50, 0, 150 ); } break;
			case 2: { pos = Vec3d( 150, 150, 50 ); } break;
			case 3: { pos = Vec3d( -150, 150, 50 ); } break;
			case 4: { pos = Vec3d( -150, -150, 50 ); } break;
			case 5: { pos = Vec3d( 150, -150, 50 ); } break;
			case 6: { pos = Vec3d( 250, 0, 50 ); } break;
			case 7: { pos = Vec3d( 0, 250, 50 ); } break;
			case 8: { pos = Vec3d( -250, 0, 50 ); } break;
			case 9: { pos = Vec3d( 0, -250, 50 ); } break;
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
			case 22: { pos = Vec3d( 275, 275, 150 ); } break;
			case 23: { pos = Vec3d( -275, 275, 150 ); } break;
			case 24: { pos = Vec3d( -275, -275, 150 ); } break;
			case 25: { pos = Vec3d( 275, -275, 150 ); } break;
			case 26: { pos = Vec3d( 275, 275, 50 ); } break;
			case 27: { pos = Vec3d( -275, 275, 50 ); } break;
			case 28: { pos = Vec3d( -275, -275, 50 ); } break;
			case 29: { pos = Vec3d( 275, -275, 50 ); } break;
		};

		pointLight.mPosition = pos;
		gPointLights.Append( pointLight );
	}

	// Store all the lights in an SSBO
	const int flags = GL_MAP_WRITE_BIT | GL_MAP_READ_BIT;
	gBufferPointLights.Generate( sizeof( storageLight_t ), gPointLights.Num(), flags );
	{
		gBufferPointLights.Bind();
		storageLight_t * bufferMappedPtr = ( storageLight_t * )gBufferPointLights.MapBuffer( GL_MAP_WRITE_BIT );
		for ( int i = 0; i < gPointLights.Num(); ++i ) {
			const Vec3d pos			= gPointLights[ i ].mPosition;
			const float radius		= gPointLights[ i ].mMaxDist;
			const Vec3d color		= gPointLights[ i ].mColor;
			const float intensity	= gPointLights[ i ].mIntensity;
			bufferMappedPtr[ i ].mSphere	= Vec4d( pos.x, pos.y, pos.z, radius );
			bufferMappedPtr[ i ].mColor		= Vec4d( color.x, color.y, color.z, intensity );
		}
		glMemoryBarrier( GL_SHADER_STORAGE_BARRIER_BIT );
		gBufferPointLights.UnMapBuffer();
		glMemoryBarrier( GL_SHADER_STORAGE_BARRIER_BIT );
	}
	gBufferLightList.Generate( sizeof( lightList_t ), gNumTiles, flags );
	
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

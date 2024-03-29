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

#include "Math/Vector.h"
#include "Math/MatrixOps.h"
#include "Math/Frustum.h"

#include "Graphics/Mesh.h"
#include "Graphics/Targa.h"
#include "Graphics/Texture.h"
#include "Graphics/Shader.h"
#include "Graphics/RenderSurface.h"
#include "Graphics/ShaderManager.h"
#include "Graphics/TextureManager.h"

#include "Miscellaneous/Time.h"
#include "Miscellaneous/Input.h"
#include "Physics/PhysicsWorld.h"
#include "Physics/Shapes/ShapeBox.h"
#include "Physics/Shapes/ShapeSphere.h"
#include "BSP/Map.h"
#include "Entities/Player.h"
#include "Entities/Projectile.h"
#include "Entities/ClothEntity.h"
#include "Fluids/FluidSim.h"

// Global storage of the window size
// int g_screenWidth  = 1200;
// int g_screenHeight = 720;
int g_screenWidth  = 1920;
int g_screenHeight = 1080;

Player g_player;
Body * g_playerBody = NULL;

ShapeBox * g_shapeProjectile = NULL;
Mesh g_meshProjectile;

ClothEntity * g_clothEntity = NULL;

std::vector< Entity * > g_entities;

Mesh g_modelScreenSpaceFarPlane;
Mesh g_modelScreenSpaceNearPlane;
Mesh g_modelSkyBox;

RenderSurface g_renderSurface;

Vec3 g_cameraPos( 0, 0, 1.7f );
Vec3 g_cameraUp( 0, 0, 1 );
Vec3 g_cameraLook( 1, 0, 0 );
float g_matView[ 16 ];

float g_dtSec = 0;
float g_timeTime = 0;

float g_timeMS = 0;

Texture * g_flagTexture = NULL;

int counter = 0;

/*
================================
Update
================================
*/
void Update() {
	// Calculate the timing
	static int timeLastFrame	= 0;
	const int time				= GetTimeMicroseconds();
	const float dt_us			= time - timeLastFrame;
	const float dt_ms			= dt_us * 0.001f;
	timeLastFrame				= time;
	static float runTimeSeconds = 0;
	runTimeSeconds += dt_ms * 0.001f;
	g_dtSec = dt_ms * 0.001f;
	g_timeTime += g_dtSec;
	g_timeMS += dt_ms;

	if ( 0 == ( counter % 120 ) ) {
		printf( "dt ms: %.2f\n", dt_ms );
	}
	++counter;

	//
	//	Input Updates
	//
	{
		float deltaSun = 0;
		if ( g_keyboard.IsKeyDown( 'o' ) || g_keyboard.IsKeyDown( 'O' ) ) {
			deltaSun += 3;
		}
		if ( g_keyboard.IsKeyDown( 'p' ) || g_keyboard.IsKeyDown( 'P' ) ) {
			deltaSun -= 3;
		}

		float speed = 1.0f;
		if ( g_keyboard.IsKeyDown( 'r' ) ) {
			speed *= 10.0f;
		}

		if ( g_keyboard.IsKeyDown( 't' ) ) {
			ResetFluidSim();
		} else {
			// Step the fluid sim
			FluidSimStep( dt_ms * 0.001f );
		}
	}

	//
	//	Update mouse inputs
	//
	{
		Vec3 ds = g_mouse.GLPosDelta() * -10.0f;
		float dtX = -ds.x;
		float dtY = -ds.y;

		if ( fabsf( dtX ) > 100 ) {
			dtX = 0;
		}
		if ( fabsf( dtY ) > 100 ) {
			dtY = 0;
		}

		float sensitivity = 0.003f;
		g_player.m_cameraPositionTheta += ds.y * sensitivity;
		g_player.m_cameraPositionPhi += ds.x * sensitivity;

		if ( g_player.m_cameraPositionTheta < 0.14f ) {
			g_player.m_cameraPositionTheta = 0.14f;
		}
		if ( g_player.m_cameraPositionTheta > 3.0f ) {
			g_player.m_cameraPositionTheta = 3.0f;
		}

		float cameraTheta = g_player.m_cameraPositionPhi;
		float cameraPhi = g_player.m_cameraPositionTheta;
		g_cameraLook.x = cosf( cameraTheta ) * sinf( cameraPhi );
		g_cameraLook.y = sinf( cameraTheta ) * sinf( cameraPhi );
		g_cameraLook.z = cosf( cameraPhi );
	}

	//
	//	Fire Projectiles
	//
	static float weaponCoolOff = 0.0f;
	weaponCoolOff -= dt_ms * 0.001f;
	if ( g_mouse.IsButtonDown( Mouse::mb_left ) && weaponCoolOff <= 0.0f ) {
		weaponCoolOff = 0.1f;

		Vec3 lookDir = g_cameraLook;

		Body body;
		body.m_position = g_playerBody->m_position + lookDir;
		body.m_position.z += 0.5f;
		body.m_orientation = Quat( 1, 0, 0, 0 );
		body.m_linearVelocity = lookDir * 50.0f;
		body.m_linearVelocity += g_playerBody->m_linearVelocity;
		body.m_angularVelocity.Zero();
		body.m_invMass = 1.0f;
		body.m_elasticity = 0;
		body.m_friction = 0;
		body.m_shape = g_shapeProjectile;
		body.m_enableRotation = false;
		body.m_enableGravity = true;
		body.m_bodyContents = BC_PROJECTILE;
		body.m_collidesWith = BC_ENEMY | BC_GENERIC;

		Projectile * entity = new Projectile;
		body.m_owner = entity;

		entity->m_bodyid = g_physicsWorld->AllocateBody( body );

		entity->m_modelDraw = &g_meshProjectile;
		g_entities.push_back( entity );
	}

	// Update Entities
	for ( int i = 0; i < g_entities.size(); i++ ) {
		Entity * entity = g_entities[ i ];
		if ( NULL == entity ) {
			g_entities.erase( g_entities.begin() + i );
			--i;
			continue;
		}
		entity->Update( dt_ms * 0.001f );

		if ( entity->IsExpired() ) {
			delete g_entities[ i ];
			g_entities[ i ] = NULL;
		}
	}

	g_player.Update( dt_ms * 0.001f );
	g_physicsWorld->StepSimulation( dt_ms * 0.001f );
	if ( g_player.m_bodyid.body->m_position.z < 0.0f ) {
		g_player.m_bodyid.body->m_linearVelocity.z = 0;
		g_player.m_bodyid.body->m_linearVelocity *= 0.99f;
		g_player.m_bodyid.body->m_position.z = 0;
	}


	if ( g_playerBody->m_position.z < -10 ) {
		g_playerBody->m_position = Vec3( 0, 0, 10 );
		g_playerBody->m_linearVelocity = Vec3( 0, 0, 0 );
	}

	//
	//	Collide with Cloth
	//
// 	ShapeSphere sphereShape( 0.5f );
// 	for ( int i = 0; i < g_entities.size(); i++ ) {
// 		Entity * entity = g_entities[ i ];
// 		if ( NULL == entity || entity->GetType() != ET_PROJECTILE ) {
// 			continue;
// 		}
// 
// 		g_clothEntity->Collide( &sphereShape, entity->m_bodyid.body->m_position );
// 	}
// 	sphereShape.m_radius = 1.0f;
// 	g_clothEntity->Collide( &sphereShape, g_playerBody->m_position );
// 	g_clothEntity->Update( dt_ms * 0.001f );
	
	//
	//	Update camera matrices
	//
	{
		// Update the camera view matrix
		const float radius = 1000;
		const float pi = acosf( -1.0f );

		Vec3 right = g_cameraLook.Cross( Vec3( 0, 0, 1 ) );
		g_cameraUp = right.Cross( g_cameraLook );
		g_cameraUp.Normalize();

		g_cameraPos = g_playerBody->m_position;
		g_cameraPos.z += 0.75f;

		Vec3 lookat = g_cameraPos + g_cameraLook;
		myLookAt( g_cameraPos, lookat, g_cameraUp, g_matView );
	}
}

/*
================================
DrawFrame
================================
*/
void DrawFrame( void ) {
	Update();

	const float bias_matrix[ 16 ] = {
		0.5f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.5f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.5f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f
	};

	// This sets the perspective projection matrix
	float matProj[ 16 ] = { 0 };
	const float fieldOfViewDegrees = 120;
	//const float fieldOfViewDegrees = 90;
	const float aspectRatio = static_cast< float >( g_screenHeight ) / static_cast< float >( g_screenWidth );
	const float nearDepth = 0.01f;
	const float farDepth = 1000;
	myPerspective( fieldOfViewDegrees, aspectRatio, nearDepth, farDepth, matProj );

	// Bind the off screen render surface for rendering
	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, g_renderSurface.GetFBO() );

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
	glViewport( 0, 0, g_screenWidth, g_screenHeight );

	// Clear the frame buffer color
	// Inputs are Red Green Blue Alpha (RGBA)
	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );

	// Clear previous frame values
	glClear( GL_COLOR_BUFFER_BIT );
	glClear( GL_DEPTH_BUFFER_BIT );

	// This will turn on back face culling.  Uncomment to turn on.
	glCullFace( GL_BACK );
	glEnable( GL_CULL_FACE );
	glDisable( GL_CULL_FACE );

	//
	//	Draw Skybox
	//
	{
		glDepthMask( GL_FALSE );

		// Set the shader program that'll be used to render the mesh
		Shader * shader = g_shaderManager->GetAndUseShader( "sky" );

		// Send the projection matrix to the shader
		shader->SetUniformMatrix4f( "matProj", 1, false, matProj );
		shader->SetUniformMatrix4f( "matView", 1, false, g_matView );
		g_modelSkyBox.Draw();

		glDepthMask( GL_TRUE );
	}

	//
	//	Draw brushes
	//
	{
		// Set the shader program that'll be used to render the mesh
		Shader * shader = g_shaderManager->GetAndUseShader( "Checkerboard/checkerboard2" );
	
		float matIdentity[ 16 ] = {
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1
		};

		// Send the transformation matrix to the shader
		shader->SetUniformMatrix4f( "matModelWorld", 1, false, matIdentity );

		// Send the projection matrix to the shader
		shader->SetUniformMatrix4f( "matProj", 1, false, matProj );
		shader->SetUniformMatrix4f( "matView", 1, false, g_matView );

		// Draw the brush map
		//DrawMap();
	}

	//
	//	Draw Entities
	//
	for ( int i = 0; i < g_entities.size(); i++ ) {
		Entity * entity = g_entities[ i ];
		if ( NULL == entity || NULL == entity->m_modelDraw || NULL == entity->m_bodyid.body ) {
			continue;
		}

		// Set the shader program that'll be used to render the mesh
		Shader * shader = g_shaderManager->GetAndUseShader( "Checkerboard/checkerboard2" );

		Vec3 pos = entity->m_bodyid.body->m_position;
		Quat orient = entity->m_bodyid.body->m_orientation;
		Mat4 matOrient;
		matOrient.Orient( pos, orient );
		matOrient = matOrient.Transpose();

		// Send the transformation matrix to the shader
		shader->SetUniformMatrix4f( "matModelWorld", 1, false, matOrient.ToPtr() );

		// Send the projection matrix to the shader
		shader->SetUniformMatrix4f( "matProj", 1, false, matProj );
		shader->SetUniformMatrix4f( "matView", 1, false, g_matView );

		entity->m_modelDraw->Draw();
	}

	//
	//	Draw Cloth
	//
// 	{
// 		Entity * entity = g_clothEntity;
// 
// 		// Set the shader program that'll be used to render the mesh
// 		Shader * shader = g_shaderManager->GetAndUseShader( "flag" );
// 
// 		Vec3 pos = Vec3( 0, 0, 0 );//entity->m_bodyid.body->m_position;
// 		Quat orient = entity->m_bodyid.body->m_orientation;
// 		Mat4 matOrient;
// 		matOrient.Orient( pos, orient );
// 		matOrient = matOrient.Transpose();
// 
// 		// Send the transformation matrix to the shader
// 		shader->SetUniformMatrix4f( "matModelWorld", 1, false, matOrient.ToPtr() );
// 
// 		// Send the projection matrix to the shader
// 		shader->SetUniformMatrix4f( "matProj", 1, false, matProj );
// 		shader->SetUniformMatrix4f( "matView", 1, false, g_matView );
// 
// 		shader->SetAndBindUniformTexture( "flagTexture", 0, GL_TEXTURE_2D, g_flagTexture->GetName() );
// 
// 		DrawCloth();
// 	}

	//
	//	Draw Fluid
	//
	for ( int i = 0; i < MAX_PARTICLES; i++ ) {
		// Set the shader program that'll be used to render the mesh
		Shader * shader = g_shaderManager->GetAndUseShader( "fluid" );

		Vec3 pos = g_particles[ i ].pos;
		Quat orient = Quat( 0, 0, 0, 1 );
		Mat4 matOrient;
		matOrient.Orient( pos, orient );
		matOrient = matOrient.Transpose();

		// Send the transformation matrix to the shader
		shader->SetUniformMatrix4f( "matModelWorld", 1, false, matOrient.ToPtr() );

		// Send the projection matrix to the shader
		shader->SetUniformMatrix4f( "matProj", 1, false, matProj );
		shader->SetUniformMatrix4f( "matView", 1, false, g_matView );

		float speed = g_particles[ i ].vel.GetMagnitude();
		shader->SetUniform1f( "speed", 1, &speed );

		//entity->m_modelDraw->Draw();
		g_meshProjectile.Draw();
	}

    //
    //  Copy the render buffer's color buffer to the primary framebuffer
    //
    {
        const int width         = g_screenWidth;
        const int height        = g_screenHeight;
		const unsigned int fbo  = g_renderSurface.GetFBO();
		
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
reshape
================================
*/
void reshape( int w, int h ) {
    // TODO:    implement me
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
	glutInitWindowSize( g_screenWidth, g_screenHeight );

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

	LoadMap();

	g_shaderManager = new hbShaderManager;
	g_textureManager = new TextureManager;

	g_renderSurface.CreateSurface( RS_COLOR_BUFFER | RS_DEPTH_BUFFER, g_screenWidth, g_screenHeight );


	//
	//	Build the near/far planes (for full screen rendering)
	//
	{
		const unsigned short indices[ 6 ] = {
			0, 1, 2,
			0, 2, 3,
		};

		vert_t data[ 4 ];
		memset( data, 0, sizeof( vert_t ) * 4 );

		data[ 0 ].pos = Vec3( -1.0f, -1.0f, 1.0f );
		data[ 0 ].st = Vec2( 0.0f, 0.0f );
		Vec3ToByte4_n11( data[ 0 ].norm, Vec3( 0.0f, 0.0f, -1.0f ) );
		Vec3ToByte4_n11( data[ 0 ].tang, Vec3( 1.0f, 0.0f, 0.0f ) );

		data[ 1 ].pos = Vec3( 1.0f, -1.0f, 1.0f );
		data[ 1 ].st = Vec2( 1.0f, 0.0f );
		Vec3ToByte4_n11( data[ 1 ].norm, Vec3( 0.0f, 0.0f, -1.0f ) );
		Vec3ToByte4_n11( data[ 1 ].tang, Vec3( 1.0f, 0.0f, 0.0f ) );

		data[ 2 ].pos = Vec3( 1.0f, 1.0f, 1.0f );
		data[ 2 ].st = Vec2( 1.0f, 1.0f );
		Vec3ToByte4_n11( data[ 2 ].norm, Vec3( 0.0f, 0.0f, -1.0f ) );
		Vec3ToByte4_n11( data[ 2 ].tang, Vec3( 1.0f, 0.0f, 0.0f ) );

		data[ 3 ].pos = Vec3( -1.0f, 1.0f, 1.0f );
		data[ 3 ].st = Vec2( 0.0f, 1.0f );
		Vec3ToByte4_n11( data[ 3 ].norm, Vec3( 0.0f, 0.0f, -1.0f ) );
		Vec3ToByte4_n11( data[ 3 ].tang, Vec3( 1.0f, 0.0f, 0.0f ) );
		
		g_modelScreenSpaceFarPlane.LoadFromData( data, 4, indices, 6 );

		data[ 0 ].pos.z = -1.0f;
		data[ 1 ].pos.z = -1.0f;
		data[ 2 ].pos.z = -1.0f;
		data[ 3 ].pos.z = -1.0f;

		g_modelScreenSpaceNearPlane.LoadFromData( data, 4, indices, 6 );
	}

	g_physicsWorld = new PhysicsWorld;

	//
	//	Add brushes
	//
	std::vector< Vec3 > pts;
	for ( int i = 0; i < g_brushes.size(); i++ ) {
		const brush_t & brush = g_brushes[ i ];
		pts.clear();

		for ( int w = 0; w < brush.numPlanes; w++ ) {
			const winding_t & winding = brush.windings[ w ];

			for ( int v = 0; v < winding.pts.size(); v++ ) {
				pts.push_back( winding.pts[ v ] );
			}
		}

		if ( 0 == pts.size() ) {
			continue;
		}

		Body body;
		body.m_position = Vec3( 0, 0, 0 );
		body.m_orientation = Quat( 0, 0, 0, 1 );
		body.m_linearVelocity.Zero();
		body.m_angularVelocity.Zero();
		body.m_invMass = 0.0f;
		body.m_elasticity = 1.0f;//0.5f;
		body.m_enableRotation = false;
		body.m_friction = 1.0f;//0.5f;
		body.m_shape = new ShapeConvex( pts.data(), pts.size() );

		body.m_owner = NULL;
		body.m_bodyContents = BC_GENERIC;
		body.m_collidesWith = BC_ALL;
		bodyID_t m_bodyid = g_physicsWorld->AllocateBody( body );
	}

	//
	//	Add Player
	//
	Body body;
	body.m_position = Vec3( -9, 0, 10 );
	body.m_orientation = Quat( 0, 0, 0, 1 );
	body.m_shape = new ShapeCapsule( 0.25f, 0.5f );
	body.m_invMass = 0.5f;
	body.m_elasticity = 0.0f;
	body.m_enableRotation = false;
	body.m_friction = 1.0f;

	body.m_bodyContents = BC_PLAYER;
	body.m_collidesWith = BC_ENEMY | BC_GENERIC;

	bodyID_t bodyid = g_physicsWorld->AllocateBody( body );
	g_playerBody = bodyid.body;
	g_player.m_bodyid = bodyid;

	// Create the projectile shape
	{
		const float w = 0.05f;
		Vec3 pts[ 2 ];
		pts[ 0 ] = Vec3( -w );
		pts[ 1 ] = Vec3( w );
		g_shapeProjectile = new ShapeBox( pts, 2 );

		vert_t verts[ 6 * 4 ];
		unsigned short indices[ 6 * 3 * 2 ];

		for ( int i = 0; i < 4; i++ ) {
			Vec3ToByte4_n11( verts[ 0 + i ].norm, Vec3( 1, 0, 0 ) );
			Vec3ToByte4_n11( verts[ 4 + i ].norm, Vec3(-1, 0, 0 ) );
			Vec3ToByte4_n11( verts[ 8 + i ].norm, Vec3( 0, 1, 0 ) );
			Vec3ToByte4_n11( verts[ 12 + i ].norm, Vec3( 0,-1, 0 ) );
			Vec3ToByte4_n11( verts[ 16 + i ].norm, Vec3( 0, 0, 1 ) );
			Vec3ToByte4_n11( verts[ 20 + i ].norm, Vec3( 0, 0,-1 ) );

			Vec3ToByte4_n11( verts[ 0 + i ].tang, Vec3( 0, 1, 0 ) );
			Vec3ToByte4_n11( verts[ 4 + i ].tang, Vec3( 0,-1, 0 ) );
			Vec3ToByte4_n11( verts[ 8 + i ].tang, Vec3( 1, 0, 0 ) );
			Vec3ToByte4_n11( verts[ 12 + i ].tang, Vec3(-1, 0, 0 ) );
			Vec3ToByte4_n11( verts[ 16 + i ].tang, Vec3( 1, 0, 0 ) );
			Vec3ToByte4_n11( verts[ 20 + i ].tang, Vec3(-1, 0, 0 ) );

			Vec3ToByte4_n11( verts[ 0 + i ].buff, Vec3( 0, 0, 0 ) );
			Vec3ToByte4_n11( verts[ 4 + i ].buff, Vec3( 0, 0, 0 ) );
			Vec3ToByte4_n11( verts[ 8 + i ].buff, Vec3( 0, 0, 0 ) );
			Vec3ToByte4_n11( verts[ 12 + i ].buff, Vec3( 0, 0, 0 ) );
			Vec3ToByte4_n11( verts[ 16 + i ].buff, Vec3( 0, 0, 0 ) );
			Vec3ToByte4_n11( verts[ 20 + i ].buff, Vec3( 0, 0, 0 ) );
		}

		for ( int i = 0; i < 6; i++ ) {
			verts[ 4 * i + 0 ].st = Vec2( 1, 0 );
			verts[ 4 * i + 1 ].st = Vec2( 0, 0 );
			verts[ 4 * i + 2 ].st = Vec2( 0, 1 );
			verts[ 4 * i + 3 ].st = Vec2( 1, 1 );

			indices[ i * 6 + 0 ] = 4 * i + 0;
			indices[ i * 6 + 1 ] = 4 * i + 1;
			indices[ i * 6 + 2 ] = 4 * i + 2;
			
			indices[ i * 6 + 3 ] = 4 * i + 0;
			indices[ i * 6 + 4 ] = 4 * i + 2;
			indices[ i * 6 + 5 ] = 4 * i + 3;
		}

		// +X
		verts[ 0 ].pos = Vec3( w, w, w );
		verts[ 1 ].pos = Vec3( w,-w, w );
		verts[ 2 ].pos = Vec3( w,-w,-w );
		verts[ 3 ].pos = Vec3( w, w,-w );
		
		// -X
		verts[ 4 ].pos = Vec3(-w, w, w );
		verts[ 5 ].pos = Vec3(-w, w,-w );
		verts[ 6 ].pos = Vec3(-w,-w,-w );
		verts[ 7 ].pos = Vec3(-w,-w, w );

		// +Y
		verts[ 8 ].pos = Vec3( w, w, w );
		verts[ 9 ].pos = Vec3(-w, w, w );
		verts[ 10 ].pos = Vec3(-w, w,-w );
		verts[ 11 ].pos = Vec3( w, w,-w );

		// -Y
		verts[ 12 ].pos = Vec3( w,-w, w );
		verts[ 13 ].pos = Vec3( w,-w,-w );
		verts[ 14 ].pos = Vec3(-w,-w,-w );
		verts[ 15 ].pos = Vec3(-w,-w, w );

		// +Z
		verts[ 16 ].pos = Vec3( w, w, w );
		verts[ 17 ].pos = Vec3(-w, w, w );
		verts[ 18 ].pos = Vec3(-w,-w, w );
		verts[ 19 ].pos = Vec3( w,-w, w );

		// -Z
		verts[ 20 ].pos = Vec3( w, w,-w );
		verts[ 21 ].pos = Vec3( w,-w,-w );
		verts[ 22 ].pos = Vec3(-w,-w,-w );
		verts[ 23 ].pos = Vec3(-w, w,-w );

		g_meshProjectile.LoadFromData( verts, 6 * 4, indices, 6 * 3 * 2 );

		for ( int i = 0; i < 6 * 4; i++ ) {
			verts[ i ].pos *= 100.0f;
		}
		g_modelSkyBox.LoadFromData( verts, 6 * 4, indices, 6 * 3 * 2 );
	}

	g_clothEntity = new ClothEntity;
	{
		Targa targa;
		targa.Load( "../../common/FlagAmerica256.tga" );

		TextureOpts_t opts;
		opts.wrapS = WM_CLAMP;
		opts.wrapR = WM_CLAMP;
		opts.wrapT = WM_CLAMP;
		opts.minFilter = FM_LINEAR;//_MIPMAP_NEAREST;
		opts.magFilter = FM_LINEAR;
		opts.dimX = targa.GetWidth();
		opts.dimY = targa.GetHeight();
		opts.dimZ = 1;
		opts.type = TT_TEXTURE_2D;
		opts.format = FMT_RGBA8;
		g_flagTexture = g_textureManager->GetTexture( "_flagTexture", opts, targa.DataPtr() );
	}

	ResetFluidSim();

	//
	// Set GLUT function pointers here
	//
	glutReshapeFunc( reshape );
	//glutDisplayFunc( DrawFrame );
	glutKeyboardFunc( keyboard );
	glutKeyboardUpFunc( keyboardup );
	glutSpecialFunc( special );
	glutMouseFunc( mouse );
	glutMotionFunc( motion );
	glutPassiveMotionFunc( motionPassive );
	glutEntryFunc( entry );

	// Setting the idle function to point to the DrawFrame function tells GLUT to call this function in GLUT's infinite loop
	glutIdleFunc( DrawFrame );
	
	//
	//	Perform the infinite loop
	//

	// Do the infinite loop.  This starts glut's infinite loop.  It will call our draw function over and over
	glutMainLoop();

	// The code reaches here, when the application is quiting.
	delete g_shaderManager;
	delete g_textureManager;
	delete g_physicsWorld;
	return 0;
}

//
//  Player.cpp
//
#include "Player.h"
#include "Physics/Body.h"
//#include "Renderer/Models.h"
#include "Miscellaneous/Input.h"
#include "Physics/Contact.h"
#include "Physics/Intersections.h"
//#include "../Scenes/Scene.h"

/*
====================================================
Player
====================================================
*/
Player::Player() : Entity() {
	m_isDashing = false;
	m_dashCount = 0;
	m_dashChargeTime = 0.0f;

	m_jumpTime = 0.0f;
	m_jumpAllow = false;
	m_jumpCount = 0;

	m_cameraPositionTheta = 3.1415f / 2.0f;
	m_cameraPositionPhi = 0;

	m_projectileLifeTime = 0;
	m_fireProjectile = false;
	m_fireAllow = false;
}

static float clampyClamp( float val, float mins, float maxs ) {
	if ( val < mins ) {
		val = mins;
	}
	if ( val > maxs ) {
		val = maxs;
	}
	return val;
}

/*
====================================================
Player::Update
====================================================
*/
void Player::Update( float dt_sec ) {
	const float moveSpeedGround = 15.0f;
	const float moveSpeedAir = 20.0f;
	const float moveAceelerationGround = 5.0f;
	const float moveAccelerationAir = 0.2f;

	m_jumpTime += dt_sec;

	if ( m_dashCount > 0 && !m_isDashing ) {
		m_dashChargeTime += dt_sec;
		if ( m_dashChargeTime >= 1.0f ) {
			m_dashCount = 0;
		}
	}

	
	Body * player = m_bodyid.body;

	Vec3 tmpVelocity = player->m_linearVelocity;
	tmpVelocity.z = 0.0f;
	if ( m_isDashing || tmpVelocity.GetMagnitude() > moveSpeedAir ) {
		float loss = 0.99f;
		player->m_linearVelocity.x *= loss;
		player->m_linearVelocity.y *= loss;

		if ( player->m_linearVelocity.GetMagnitude() < moveSpeedAir ) {
			m_isDashing = false;
		}
	}

	//
	//	Check for ground
	//
	ShapeSphere testSphere( 0.05f );
	Body testBody;
	testBody.m_position = player->m_position;
	testBody.m_position.z += player->m_shape->GetBounds().mins.z;
	testBody.m_shape = &testSphere;

	//bool isOnGround = false;
	bool isOnGround = true;
//	contact_t groundContact;
// 	if ( player->m_linearVelocity.z < 0.01f ) {
// 		for ( int i = 0; i < g_scene->m_entities.size() - 1; i++ ) {
// 			const Entity * entity = g_scene->m_entities[ i ];
// 			if ( entity->m_bodyid.id == m_bodyid.id ) {
// 				continue;
// 			}
// 
// 			if ( Intersect( &testBody, entity->m_bodyid.body, groundContact ) ) {
// 				m_jumpCount = 0;
// 				isOnGround = true;
// 				break;
// 			}
// 		}
// 	}
	
	//
	//	Keyboard
	//
	float moveMultipliers[ 4 ] = { 1.0f, -1.0f, 1.0f, -1.0f };
	bool moveForward = false;
	bool moveBack = false;
	bool moveLeft = false;
	bool moveRight = false;
	bool jump = false;
	bool dash = false;
	m_fireProjectile = false;
	const float projectileDelayTime = 0.05f;

	if ( g_keyboard.IsKeyDown( 'w' ) ) {
		moveForward = true;
	}
	if ( g_keyboard.IsKeyDown( 's' ) ) {
		moveBack = true;
	}
	if ( g_keyboard.IsKeyDown( 'a' ) ) {
		moveLeft = true;
	}
	if ( g_keyboard.IsKeyDown( 'd' ) ) {
		moveRight = true;
	}
	if ( g_keyboard.IsKeyDown( ' ' ) ) {
		jump = true;
	}
	if ( !g_keyboard.IsKeyDown( ' ' ) ) {
		m_jumpAllow = true;
	}

	Vec3 lookDir;
	lookDir.x = cosf( m_cameraPositionPhi ) * sinf( m_cameraPositionTheta );
	lookDir.y = sinf( m_cameraPositionPhi ) * sinf( m_cameraPositionTheta );
	lookDir.z = cosf( m_cameraPositionTheta );
	Vec3 left = Vec3( 0, 0, 1 ).Cross( lookDir );
	lookDir.z = 0.0f;
	left.z = 0.0f;
	lookDir.Normalize();
	left.Normalize();

	//player->m_linearVelocity.Zero();

	player->m_friction = 0.5f;
	if ( ( moveForward || moveBack || moveLeft || moveRight ) && !m_isDashing ) {
		player->m_friction = 0.0f;

		Vec3 moveDir;
		moveDir.Zero();
		if ( moveForward ) {
			moveDir += lookDir * moveMultipliers[ 0 ];
		}
		if ( moveBack ) {
			moveDir += lookDir * moveMultipliers[ 1 ];
		}
		if ( moveLeft ) {
			moveDir += left * moveMultipliers[ 2 ];
		}
		if ( moveRight ) {
			moveDir += left * moveMultipliers[ 3 ];
		}
		float speedScale = 2.0f;
		//player->m_linearVelocity = moveDir;
		player->m_linearVelocity.x = moveDir.x * speedScale;
		player->m_linearVelocity.y = moveDir.y * speedScale;

		Vec3 groundVelocity = Vec3( 0, 0, 0 );
// 		if ( isOnGround ) {
// 			groundVelocity = groundContact.bodyB->m_linearVelocity;
// 		}

		Vec3 velocity = player->m_linearVelocity;
		velocity.z = 0.0f;

		const float moveSpeed = isOnGround ? moveSpeedGround : moveSpeedAir;
		const float maxDeltaVelocity = ( isOnGround ? moveAceelerationGround : moveAccelerationAir );// * dt_sec;

		// The target velocity the player is aiming to be
		Vec3 targetVelocity = moveDir * moveSpeed - groundVelocity;

		// The velocity delta we need to reach the target velocity
		Vec3 velocityDelta = targetVelocity - velocity;

		// Clamp the maximum change in velocity
		velocityDelta.x = clampyClamp( velocityDelta.x, -maxDeltaVelocity, maxDeltaVelocity );
		velocityDelta.y = clampyClamp( velocityDelta.y, -maxDeltaVelocity, maxDeltaVelocity );

		// Apply the impulse
		Vec3 impulse = velocityDelta * ( 1.0f / player->m_invMass );
		player->ApplyImpulseLinear( impulse );
	}

	if ( jump /*&& m_jumpCount < 2*/ && ( isOnGround || m_jumpAllow ) && m_jumpTime > 0.5f ) {
		m_jumpCount++;
		m_jumpAllow = false;
		m_jumpTime = 0.0f;
		Vec3 targetVelocity = Vec3( 0, 0, 5 );
		Vec3 impulse = targetVelocity * ( 1.0f / player->m_invMass );
		player->ApplyImpulseLinear( impulse );
	}

	if ( dash && !m_isDashing && m_dashCount < 2 ) {
		m_isDashing = true;
		m_dashChargeTime = 0.0f;
		m_dashCount++;

		Vec3 moveDir;
		moveDir.Zero();
		if ( moveForward ) {
			moveDir += lookDir;
		}
		if ( moveBack ) {
			moveDir += lookDir * -1.0f;
		}
		if ( moveLeft ) {
			moveDir += left;
		}
		if ( moveRight ) {
			moveDir += left * -1.0f;
		}

		// Default the dash direction to the forward direction
		if ( moveDir.GetLengthSqr() < 0.1f ) {
			moveDir = lookDir;
		}

		player->m_linearVelocity = moveDir * moveSpeedAir * 2.0f;
	}
	if ( m_isDashing && player->m_linearVelocity.GetMagnitude() < moveSpeedAir ) {
		m_isDashing = false;	
	}

	player->m_enableGravity = !m_isDashing;
	m_projectileLifeTime += dt_sec;
}
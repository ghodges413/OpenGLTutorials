//
//  Player.h
//
#pragma once
#include "Entity.h"

class Model;

/*
====================================================
Entity
====================================================
*/
class Player : public Entity {
public:
	Player();
	Player( const Player & entity ) = delete;
	const Player & operator = ( const Player & entity ) = delete;

	virtual EntityType_t GetType() const override { return EntityType_t::ET_PLAYER; }

	Vec3 GetPosition() const { return m_bodyid.body->m_position; }

// 	Model *	m_model;
// 
// 	bool m_isProjectile;
// 	float m_lifeTime;
// 
// 	bool m_flash;




	float m_projectileLifeTime;
	bool m_fireProjectile;
	bool m_fireAllow;
	bool m_jumpAllow;
	int m_jumpCount;
	float m_jumpTime;

	bool m_isDashing;
	float m_dashChargeTime;
	int m_dashCount;

	float m_cameraPositionTheta;
	float m_cameraPositionPhi;

	virtual void Update( float dt_sec ) override;
};

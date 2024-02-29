//
//  Entity.h
//
#pragma once
#include "Physics/PhysicsWorld.h"

class Mesh;

enum EntityType_t {
	ET_NONE,
	ET_PLAYER,
	ET_PROJECTILE,
	ET_SPIDER,
};

/*
====================================================
Entity
====================================================
*/
class Entity {
public:
	Entity();
	Entity( const Entity & entity );
	const Entity & operator = ( const Entity & entity );
	virtual ~Entity() {}

	bodyID_t m_bodyid;

	Mesh * m_modelDraw;	// Model for rendering

	bool m_flash;
	int m_health;

	virtual void Update( float dt_sec ) {}
	virtual bool IsExpired() const { return false; }
	virtual EntityType_t GetType() const { return EntityType_t::ET_NONE; }
};

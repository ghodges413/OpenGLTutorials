//
//  Projectile.h
//
#pragma once
#include "Entities/Entity.h"

/*
====================================================
Projectile
====================================================
*/
class Projectile : public Entity {
public:
	Projectile();

	virtual void Update( float dt_sec ) override;
	virtual bool IsExpired() const override;
	virtual EntityType_t GetType() const override { return EntityType_t::ET_PROJECTILE; }

	float m_lifeTime;
};

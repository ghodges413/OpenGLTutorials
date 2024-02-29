//
//  Projectile.cpp
//
#include "Entities/Projectile.h"

/*
====================================================
Projectile::Projectile
====================================================
*/
Projectile::Projectile() : Entity() {
	m_lifeTime = 10.0f;
}

/*
====================================================
Projectile::Update
====================================================
*/
void Projectile::Update( float dt_sec ) {
	m_lifeTime -= dt_sec;
}

/*
====================================================
Projectile::IsExpired
====================================================
*/
bool Projectile::IsExpired() const {
	return ( m_lifeTime < 0.0f );
}
//
//  ClothEntity.h
//
#pragma once
#include "Entities/Entity.h"
#include "Physics/Cloth.h"

void DrawCloth();

/*
====================================================
ClothEntity
====================================================
*/
class ClothEntity : public Entity {
public:
	ClothEntity();
	~ClothEntity();

	virtual void Update( float dt_sec ) override;
	virtual bool IsExpired() const override;
	virtual EntityType_t GetType() const override { return EntityType_t::ET_CLOTH; }

	void Collide( ShapeSphere * shape, Vec3 pos ) { m_clothPhysics.Collide( shape, pos ); }

	Cloth m_clothPhysics;
};

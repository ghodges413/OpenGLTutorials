//
//  Entity.cpp
//
#include "Entity.h"
#include "Physics/Body.h"
#include "Graphics/Mesh.h"

/*
====================================================
Entity::Entity
====================================================
*/
Entity::Entity() : m_modelDraw( NULL ), m_flash( false ), m_health( 0 ) {}

/*
====================================================
Entity::Entity
====================================================
*/
Entity::Entity( const Entity & entity ) {
    *this = entity;
}

/*
====================================================
Entity::operator=
====================================================
*/
const Entity & Entity::operator=( const Entity & entity ) {
	m_bodyid    = entity.m_bodyid;
	m_modelDraw = entity.m_modelDraw;
	m_flash     = entity.m_flash;
    m_health    = entity.m_health;

    return *this;
}

//
//	ShapeCapsule.h
//
#pragma once
#include "Physics/Shapes/ShapeBase.h"

/*
====================================================
ShapeCapsule
====================================================
*/
class ShapeCapsule : public Shape {
public:
	explicit ShapeCapsule( const float radius, const float height ) : m_radius( radius ), m_height( height ) {
		m_height = height;
		m_radius = radius;

		m_centerOfMass.Zero();
		m_inertiaTensor = CalculateInertiaTensor( radius, height );
	}

	Vec3 Support( const Vec3 & dir, const Vec3 & pos, const Quat & orient, const float bias ) const override;

	Mat3 CalculateInertiaTensor( float radius, float height ) const;
	Mat3 InertiaTensor() const override;

	Bounds GetBounds( const Vec3 & pos, const Quat & orient ) const override;
	Bounds GetBounds() const override;

	shapeType_t GetType() const override { return SHAPE_CAPSULE; }

public:
	float m_radius;
	float m_height;

	Mat3 m_inertiaTensor;
};
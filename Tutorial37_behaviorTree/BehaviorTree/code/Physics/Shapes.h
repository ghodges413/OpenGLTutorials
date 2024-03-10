//
//	Shapes.h
//
#pragma once
#include "Shapes/ShapeSphere.h"
#include "Shapes/ShapeBox.h"
#include "Shapes/ShapeConvex.h"
#include "Shapes/ShapeCapsule.h"

extern Vec3 g_boxGround[ 8 ];
extern Vec3 g_boxWall0[ 8 ];
extern Vec3 g_boxWall1[ 8 ];
extern Vec3 g_boxUnit[ 8 ];
extern Vec3 g_boxSmall[ 8 ];
extern Vec3 g_boxBeam[ 8 ];
extern Vec3 g_boxPlatform[ 8 ];
extern Vec3 g_boxBody[ 8 ];
extern Vec3 g_boxLimb[ 8 ];
extern Vec3 g_boxHead[ 8 ];
extern Vec3 g_diamond[ 7 * 8 ];
extern Vec3 g_pencil[ 7 * 8 ];
extern Vec3 g_octagonGround[ 8 * 2 ];
extern Vec3 g_pentagonGround[ 5 * 2 ];
extern Vec3 g_pentagonGroundSmall[ 5 * 2 ];
extern Vec3 g_pentagonGroundPoints[ 5 ][ 3 * 2 ];
extern Vec3 g_pentagonGroundFillers[ 5 ][ 3 * 2 ];
void FillDiamond();



class ShapePool {
public:
	ShapePool() {}

	// TODO: Make a pool of shapes
	// We should probably use a hashmap so that when we request a shape, we can just re-use pre-built ones.
	// Or we could just do a single growable list, return Shape id's and use them to get the pointers.

	struct CreateParms_t {
		Shape::shapeType_t type;
	};

	std::vector< Shape * > m_shapes;


	std::vector< ShapeBox > m_boxes;
	std::vector< ShapeSphere > m_spheres;
	std::vector< ShapeConvex > m_convexes;
	std::vector< ShapeCapsule > m_capsules;
};
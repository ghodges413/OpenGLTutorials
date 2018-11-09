/*
 *  Mesh.h
 *
 */

#pragma once
#include "Array.h"
#include "Vector.h"

struct vert_t {
	Vec3d pos;
	Vec3d norm;
	Vec2d st;
};

/*
 ================================
 Mesh
 ================================
 */
class Mesh {
public:
	bool Load( const char * name );

private:
	Array< Vec3d > mPositions;
	Array< Vec3d > mNormals;
	Array< Vec2d > mST;

public:
	Array< vert_t > mVerts;
};
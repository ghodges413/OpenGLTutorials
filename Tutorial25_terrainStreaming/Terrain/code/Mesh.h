/*
 *  Mesh.h
 *
 */

#pragma once
#include "Array.h"
#include "Vector.h"
#include "VertexArrayObject.h"
#include "VertexBufferObject.h"

struct vert_t {
	Vec3d pos;
	Vec3d norm;
	Vec3d tang;
	Vec2d st;
};

/*
 ================================
 Mesh
 ================================
 */
class Mesh {
public:
	Mesh();

	bool Load( const char * name );
	void Draw() const;

	int NumVerts() const { return mVerts.Num(); }

private:
	void CalculateTangents();
	void MakeVBO();

private:
	Array< Vec3d > mPositions;
	Array< Vec3d > mNormals;
	Array< Vec2d > mST;

	VertexBufferObject    mVBO;
    VertexArrayObject     mVAO;

public:
	Array< vert_t > mVerts;
};

void GenerateTerrainMesh( Mesh & mesh );
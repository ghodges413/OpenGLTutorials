/*
 *  MD5Model.h
 *
 */
#pragma once
#include "MD5Anim.h"
#include "Vector.h"
#include "Array.h"
#include "VertexArrayObject.h"
#include "VertexBufferObject.h"

#define MAX_MD5_WEIGHTS 6
struct md5Vertex {
	Vec2d     mST;
	int			mStartWeight;
	int			mWeightCount;
    int         mWeights[ MAX_MD5_WEIGHTS ];     // weight ids
};

struct md5Weight {
	int		mJointID;
	float	mBias;
	Vec3d	mPosition;
    Vec3d	mNormal;       // relative normal
    Vec3d	mTangnt;       // relative tangnt
};

struct md5Vert {
    Vec3d mPos;
    Vec3d mNorm;
    Vec3d mTang;
	Vec2d mST;
};

struct md5Mesh {
	Array< md5Vertex >		mVerts;
	Array< md5Weight >		mWeights;
    Array< unsigned short >	mTriIndices;

	// The transformed vertices, used for cpu skinning and initialization of the VBOs
	Array< md5Vert >          mTransformedVertices;
};

struct md5MeshVBO {
	VertexBufferObject	mVBOs[ 2 ];
    VertexArrayObject		mVAO;
};

/*
 ====================================
 MD5Model
 ====================================
 */
class MD5Model {
public:
	MD5Model();
	~MD5Model();

	bool Load( const char * fileName );
	void PrepareMesh( const md5Skeleton_t & skeleton );

	void GetTransformedNormals( Array< Vec3d > & normals );
	void GetTransformedTangnts( Array< Vec3d > & tangnts );

	void Draw();
	void MakeVBO();
	void UpdateVBO();
    
private:
    void CalculateNormals( const md5Skeleton_t & skeleton );
	
public:
	md5Skeleton_t		mJoints;		// base frame skeleton
	Array< md5Mesh >	mMeshes;		// the decomposed meshes

	Array< md5MeshVBO * > mMeshesVBO;
};


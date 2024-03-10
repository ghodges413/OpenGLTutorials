//
//	MD5Model.h
//
#pragma once
#include "Animation/MD5Anim.h"
#include "Math/Vector.h"
#include "Miscellaneous/Array.h"
#include "Graphics/Mesh.h"

#define MAX_MD5_WEIGHTS 6
struct md5Vertex {
	Vec2     mST;
	int			mStartWeight;
	int			mWeightCount;
    int         mWeights[ MAX_MD5_WEIGHTS ];     // weight ids
};

struct md5Weight {
	int		mJointID;
	float	mBias;
	Vec3	mPosition;
    Vec3	mNormal;       // relative normal
    Vec3	mTangnt;       // relative tangnt
};

// struct md5Vert {
//     Vec3 mPos;
//     Vec3 mNorm;
//     Vec3 mTang;
// 	Vec2 mST;
// };

struct md5Mesh {
	Array< md5Vertex >		mVerts;
	Array< md5Weight >		mWeights;
    Array< unsigned short >	mTriIndices;

	// The transformed vertices, used for cpu skinning and initialization of the VBOs
	Array< vert_t >          mTransformedVertices;
};

struct md5MeshGPU_t {
	VertexBufferObject vbo;
	VertexBufferObject ibo;
	VertexArrayObject vao;
};

/*
 ====================================
 MD5Model
 ====================================
 */
class MD5Model {
public:
	bool Load( const char * fileName );
	void PrepareMesh( const md5Skeleton_t & skeleton );
	void MakeVBO();
	void UpdateVBO();

// 	void GetTransformedNormals( Array< Vec3 > & normals );
// 	void GetTransformedTangnts( Array< Vec3 > & tangnts );
	void Draw();
    
private:
    void CalculateNormals( const md5Skeleton_t & skeleton );
	
public:
	md5Skeleton_t		mJoints;		// base frame skeleton
	Array< md5Mesh >	mMeshes;		// the decomposed meshes
	md5MeshGPU_t		m_gpuMeshes[ 10 ];		// the gpu meshes
};


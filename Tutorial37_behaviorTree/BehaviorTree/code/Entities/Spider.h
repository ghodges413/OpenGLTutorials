//
//	Spider.h
//
#pragma once
#include "Math/Vector.h"
#include "Animation/MD5Model.h"
#include "Entities/Entity.h"

class Player;

enum aiState_t {
	AI_IDLE = 0,
	AI_WALK,
};

/*
 ================================
 Spider
 ================================
 */
class Spider {
public:
	Spider();
	~Spider() {}

	void Load();
	void Update( const float dt_ms );
	void UpdateBT( const float dt_ms );
	void Draw();

	const Vec3 & GetPosition()	const { return mPosition; }
	const Vec3 & GetLookDir()	const { return mLookDir; }

	const md5Skeleton_t & GetSkeleton()	const { return *mSkeleton; }
	const ShapeSphere & GetSphere()	const { return mSphere; }

	// Returns the matrix used to move the model from model space to world space
	Mat4 GetOrientationMatrix() const;

	MD5Model & GetModel() { return mModel; }

//private:
	Vec3	mPosition;
	Vec3	mLookDir;

	ShapeSphere mSphere;
	Vec3 mRenderPosition;
	static const float sRadius;

	MD5Model    mModel;
    MD5Anim     mAnimIdle;
	MD5Anim     mAnimWalk;
	MD5Anim		mAnimAttack;
	MD5Anim		mAnimPain;

	bool mIsAttacking;
	bool mIsWalking;
	bool mIsPain;

	const md5Skeleton_t *	mSkeleton;

	aiState_t m_state;
};








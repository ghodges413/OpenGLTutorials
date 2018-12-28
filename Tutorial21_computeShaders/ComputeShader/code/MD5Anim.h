/*
 *  MD5Anim.h
 *
 */
#pragma once
#include "Array.h"
#include "Quat.h"
#include "Matrix.h"

struct md5JointInfo {
	int			mParentID;
	int			mFlags;
	int			mStartIndex;
};

struct md5Joint {
	int			mParentID;
	Vec3d     mPosition;
	Quat		mOrientation;
    Matrix    mRotMat;
    Matrix    mTransMat;
    Matrix    mOrientMat;
};

typedef Array< md5Joint > md5Skeleton_t;

/*
 ====================================
 MD5Anim
 ====================================
 */
class MD5Anim {
public:
	bool Load( const char * fileName );
	void BuildFrameSkeleton( const int frame, const int numJoints, const int numAnimatedComponents, const float * animFrameData );
	const md5Skeleton_t & operator [] ( const int i ) const { return mSkeletonFrames[ ( i % mSkeletonFrames.Num() ) ]; };

	int FrameRate()			const { return mFrameRate; }
	int NumberOfFrames()	const { return mSkeletonFrames.Num(); }	
	
private:
	int						mFrameRate;
	Array< md5JointInfo >	mJointInfos;
	md5Skeleton_t			mBaseFrame;
	Array< md5Skeleton_t >	mSkeletonFrames;
};


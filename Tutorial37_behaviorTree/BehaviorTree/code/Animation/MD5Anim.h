//
//	MD5Anim.h
//
#pragma once
#include "Miscellaneous/Array.h"
#include "Math/Quat.h"
#include "Animation/AnimMatrix.h"

struct md5JointInfo {
	int			mParentID;
	int			mFlags;
	int			mStartIndex;
};

#define USE_ANIMATRIX

struct md5Joint {
	int		mParentID;
	Vec3	mPosition;
	Quat	mOrientation;
#if !defined( USE_ANIMATRIX )
    Mat4	mRotMat;
    Mat4	mTransMat;
    Mat4	mOrientMat;
#else
	Matrix	mRotMat;
    Matrix	mTransMat;
    Matrix	mOrientMat;
#endif
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


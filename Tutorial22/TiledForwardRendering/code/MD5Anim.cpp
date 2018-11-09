/*
 *  MD5Anim.cpp
 *
 */
#include "MD5Anim.h"
#include <stdio.h>
#include <string>
#include "Fileio.h"

/*
 ====================================
 MD5Anim::Load
 ====================================
 */
bool MD5Anim::Load( const char * fileName ) {
	char buff[ 512 ] = { 0 };
	float * animFrameData = NULL;
	int version = 0;
	int numAnimatedComponents = 0;
	int frameIndex = -1;
	int numFrames = 0;
	int numJoints = 0;
	int frameRate = 0;
	
	char fullPath[ 2048 ];
	RelativePathToFullPath( fileName, fullPath );	
	
	FILE * fp = fopen( fullPath, "rb" );
	if ( !fp ) {
		fprintf( stderr, "Error: couldn't open \"%s\"!\n", fullPath );
		return false;
    }
	
	while ( !feof( fp ) ) {
		// Read whole line
		fgets( buff, sizeof( buff ), fp );
		
		if ( sscanf( buff, " MD5Version %i", &version ) == 1 ) {
			if ( version != 10 ) {
				// Bad version
				fprintf( stderr, "Error: bad animation version\n");
				fclose( fp );
				return false;
			}
		} else if ( sscanf( buff, " numFrames %i", &numFrames ) == 1 ) {
			printf( "numFrames: %d\n", numFrames );
			// Allocate memory for skeleton frames and bounding boxes
			if ( numFrames > 0 ) {
				mSkeletonFrames.Resize( numFrames );
                md5Skeleton_t skel;
                for ( int i = 0; i < numFrames; ++i ) {
                    mSkeletonFrames.Append( skel );
                }
			}
		} else if ( sscanf( buff, " numJoints %i", &numJoints ) == 1 ) {
			if ( numJoints > 0 ) {				
				// Allocate temporary memory for building skeleton frames
				mJointInfos.Resize( numJoints );
				mJointInfos.Clear();

				mBaseFrame.Resize( numJoints );
				mBaseFrame.Clear();
			}
		} else if ( sscanf( buff, " frameRate %i", &frameRate ) == 1 ) {
			mFrameRate = frameRate;
		} else if ( sscanf( buff, " numAnimatedComponents %i", &numAnimatedComponents ) == 1 ) {
			if ( numAnimatedComponents > 0 ) {
				// Allocate memory for animation frame data
				animFrameData = (float *)malloc( sizeof( float ) * numAnimatedComponents );
			}
		} else if ( strncmp( buff, "hierarchy {", 11 ) == 0 ) {
			for ( int i = 0; i < numJoints; ++i ) {
				// Read whole line
				fgets( buff, sizeof( buff ), fp );
				
				// Read joint info
				char name[ 128 ] = { 0 };
				md5JointInfo jointInfo;
				sscanf( buff, " %s %i %i %i", name, &jointInfo.mParentID, &jointInfo.mFlags, &jointInfo.mStartIndex );
				//jointInfo.mName = name;
				mJointInfos.Append( jointInfo );
			}
		} else if ( strncmp( buff, "bounds {", 8 ) == 0 ) {
			for ( int i = 0; i < numFrames; ++i ) {
				// Read whole line
				fgets( buff, sizeof( buff ), fp );
				
				// Read bounding box
				Vec3d min;
				Vec3d max;
				sscanf( buff, " ( %f %f %f ) ( %f %f %f )",
						&min.x, &min.y, &min.z,
						&max.x, &max.y, &max.z );
			}
		} else if ( strncmp( buff, "baseframe {", 10 ) == 0 ) {
			printf( "baseFrame\n" );
			for ( int i = 0; i < numJoints; ++i ) {
				// Read whole line 
				fgets( buff, sizeof( buff ), fp );
				
				// Read base frame joint 
				md5Joint joint;
				if ( sscanf( buff, " ( %f %f %f ) ( %f %f %f )",
							&joint.mPosition.x, &joint.mPosition.y, &joint.mPosition.z,
							&joint.mOrientation.x, &joint.mOrientation.y, &joint.mOrientation.z ) == 6 ) {
					// Compute the w component
					joint.mOrientation.ComputeW();
					mBaseFrame.Append( joint );
				}
			}
		} else if ( sscanf( buff, " frame %i", &frameIndex ) == 1 ) {
			// Read frame data
			for ( int i = 0; i < numAnimatedComponents; ++i ) {
				fscanf( fp, "%f", &animFrameData[ i ] );
			}
			
			// Build frame skeleton from the collected data 
			BuildFrameSkeleton( frameIndex, numJoints, numAnimatedComponents, animFrameData );
		}
	}
	fclose( fp );
    
    printf( "Num Joints: %i\n", mBaseFrame.Num() );
	
	return true;	
}

/*
 ====================================
 MD5Anim::BuildFrameSkeleton
 ====================================
 */
void MD5Anim::BuildFrameSkeleton( const int frame, const int numJoints, const int numAnimatedComponents, const float * animFrameData ) {
	mSkeletonFrames[ frame ].Resize( numJoints );
	mSkeletonFrames[ frame ].Clear();
	for ( int i = 0; i < numJoints; ++i ) {
		md5Joint *	baseJoint = &mBaseFrame[ i ];
		Vec3d		animatedPos;
		Quat		animatedOrient;
		
		animatedPos = baseJoint->mPosition;
		animatedOrient = baseJoint->mOrientation;
		
		int j = 0;
		if ( mJointInfos[ i ].mFlags & 1 ) { // Tx
			animatedPos.x = animFrameData[ mJointInfos[ i ].mStartIndex + j ];
			++j;
		}
		if ( mJointInfos[ i ].mFlags & 2 ) { // Ty
			animatedPos.y = animFrameData[ mJointInfos[ i ].mStartIndex + j ];
			++j;
		}
		if ( mJointInfos[ i ].mFlags & 4 ) { // Tz
			animatedPos.z = animFrameData[ mJointInfos[ i ].mStartIndex + j ];
			++j;
		}
		
		if ( mJointInfos[ i ].mFlags & 8 ) { // Qx
			animatedOrient.x = animFrameData[ mJointInfos[ i ].mStartIndex + j ];
			++j;
		}
		if ( mJointInfos[ i ].mFlags & 16 ) { // Qy
			animatedOrient.y = animFrameData[ mJointInfos[ i ].mStartIndex + j ];
			++j;
		}
		if ( mJointInfos[ i ].mFlags & 32 ) { // Qz
			animatedOrient.z = animFrameData[ mJointInfos[ i ].mStartIndex + j ];
			++j;
		}
		
		// Compute orient quaternion's w value
		animatedOrient.ComputeW();

		// calculate this joint
		md5Joint joint;
		joint.mParentID = mJointInfos[ i ].mParentID;		

		if ( joint.mParentID < 0 ) {
			joint.mPosition = animatedPos;
			joint.mOrientation = animatedOrient;
            joint.mRotMat = joint.mOrientation.ToMatrix();
            joint.mTransMat.SetTranslationMatrix( joint.mPosition );
            
            joint.mOrientMat = joint.mRotMat * joint.mTransMat;
		} else {
			const md5Joint & parentJoint = mSkeletonFrames[ frame ][ joint.mParentID ];
			
			// Add positions
			Vec3d rpos = parentJoint.mOrientation.RotatePoint( animatedPos );
			joint.mPosition = rpos + parentJoint.mPosition;
			
			// Concatenate rotations
			joint.mOrientation = parentJoint.mOrientation * animatedOrient;
			joint.mOrientation.Normalize();
            joint.mRotMat = joint.mOrientation.ToMatrix();
            joint.mTransMat.SetTranslationMatrix( joint.mPosition );
            
            // Concatenate matrices
            joint.mOrientMat = joint.mRotMat * joint.mTransMat;
		}
		
		// add animated joint to this skeleton frame
		mSkeletonFrames[ frame ].Append( joint );
    }
}


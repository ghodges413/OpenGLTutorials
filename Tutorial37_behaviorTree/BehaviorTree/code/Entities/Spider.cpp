//
//	Spider.cpp
//
#include "Entities/Spider.h"
#include "Entities/Player.h"
#include "NavMesh/PathFinding.h"

const float Spider::sRadius = 0.25f;

extern Player g_player;

/*
 ================================
 Spider::Spider
 ================================
 */
Spider::Spider() : mSphere( Spider::sRadius ) {
	mLookDir = Vec3( 1, 0, 0 );
	mPosition = Vec3( 10, 0, 0 );
}

/*
 ================================
 Spider::Load
 ================================
 */
void Spider::Load() {
    mModel.Load( "../../common/animation/trite.md5mesh" );
	mModel.MakeVBO();

    mAnimIdle.Load( "../../common/animation/alert_idle.md5anim" );
	mAnimWalk.Load( "../../common/animation/walk1.md5anim" );
    mAnimAttack.Load( "../../common/animation/melee1.md5anim" );
    mAnimPain.Load( "../../common/animation/evade_left.md5anim" );
}

std::vector< Vec3 > g_pathPts;
std::vector< navEdge_t > g_navEdges;

/*
 ================================
 Spider::Update
 ================================
 */
void Spider::UpdateBT( const float dt_ms ) {
	Vec3 posToRenderPos = mRenderPosition - mPosition;
	float dist = posToRenderPos.GetMagnitude();
	mRenderPosition = mPosition - mLookDir * dist;

	//
	//  update Spider animation
	//

	// Choose the animation based upon the Spider state
	static MD5Anim * prevAnim = NULL;
	MD5Anim * spiderAnim = &mAnimIdle;
	if ( m_state == AI_WALK ) {
		spiderAnim = &mAnimWalk;
	}
	if ( m_state == AI_IDLE ) {
		spiderAnim = &mAnimIdle;
	}

	static float animTime = 0.0f;

	// Reset the frame number if the animation changes
	if ( prevAnim != spiderAnim ) {
		prevAnim = spiderAnim;

		animTime = 0.0f;
		mRenderPosition = mPosition;
	}

	// Update the animation time and calculate the current anim frame
	animTime					+= dt_ms * 0.001f;
	//animTime = 0;
	const int frameRate			= spiderAnim->FrameRate();
	const int numFrames			= spiderAnim->NumberOfFrames();
	const float totalAnimTime	= (float)numFrames / (float)frameRate;
	const float percentTime		= animTime / totalAnimTime;
	int currentFrame			= percentTime * numFrames;
	
	if ( animTime >= totalAnimTime ) {
		mRenderPosition	= mPosition;
		animTime		= 0.0f;
		currentFrame	= 0;
	}

	// Animate and update mesh
	const md5Skeleton_t & skeleton = (*spiderAnim)[ currentFrame ];
	mModel.PrepareMesh( skeleton );
	mModel.UpdateVBO();

	mSkeleton = &skeleton;

	// Get the skeleton's base position in world space
	Vec3 pos3 = (*mSkeleton)[ 0 ].mPosition * ( 1.0f / 64.0f );
	Vec4 pos4 = Vec4( pos3.x, pos3.y, pos3.z, 1.0f );
	pos4 = GetOrientationMatrix() * pos4;
	mPosition = Vec3( pos4.x, pos4.y, pos4.z );
}

#define USE_BEHAVIOR_TREE

/*
 ================================
 Spider::Update
 ================================
 */
void Spider::Update( const float dt_ms ) {
#if defined( USE_BEHAVIOR_TREE )
	UpdateBT( dt_ms );
#else
	Vec3 newDir( 0.0f );
	Vec3 prevLook = mLookDir;
	const Vec3 playerPos = g_player.GetPosition() + Vec3( 0, 0, 0.1f );

	Vec3 posToRenderPos = mRenderPosition - mPosition;

	if ( mPosition.z < 0 ) {
		mPosition.z = 0;
	}

	const bool isSpiderInNav = IsPositionInNavMesh( mPosition );
	const bool isPlayerInNav = IsPositionInNavMesh( playerPos );

	if ( !isSpiderInNav ) {
		mPosition = Vec3( 0.0f );
	}

	g_pathPts.clear();
	g_navEdges.clear();
	Vec3 targetPos = mPosition;
	if ( PathFind( mPosition, playerPos, g_pathPts, g_navEdges ) ) {
		g_pathPts.clear();
		for ( int i = 0; i < g_navEdges.size(); i++ ) {
			navEdge_t edge = g_navEdges[ i ];
			g_pathPts.push_back( ( edge.a + edge.b ) * 0.5f );
		}

		targetPos = playerPos;
		if ( g_pathPts.size() > 0 ) {
			targetPos = g_pathPts[ 0 ];
			Vec3 delta = targetPos - mPosition;
			if ( delta.GetMagnitude() < 0.01f ) {
				targetPos = playerPos;
				if ( g_pathPts.size() > 1 ) {
					targetPos = g_pathPts[ 1 ];
				}
			}
		}
	}

	//
	//	update Spider input
	//
	const Vec3 rayToPlayer = playerPos - mPosition;
	mLookDir = targetPos - mPosition;
	mLookDir.Normalize();
	if ( mLookDir.GetMagnitude() < 0.9f ) {
		mLookDir = prevLook;
	}
	float dist = posToRenderPos.GetMagnitude();
	mRenderPosition = mPosition - mLookDir * dist;

	//
	//  update Spider animation
	//

	// Choose the animation based upon the Spider state
	static MD5Anim * prevAnim = NULL;
	MD5Anim * spiderAnim = &mAnimIdle;
	if ( rayToPlayer.GetMagnitude() > 2.0f ) {
		spiderAnim = &mAnimWalk;
	}
	if ( !isSpiderInNav ) {
		spiderAnim = &mAnimIdle;
	}
	if ( !isPlayerInNav ) {
		spiderAnim = &mAnimIdle;
	}

	static float animTime = 0.0f;

	// Reset the frame number if the animation changes
	if ( prevAnim != spiderAnim ) {
		prevAnim = spiderAnim;

		animTime = 0.0f;
		mRenderPosition = mPosition;
	}

	// Update the animation time and calculate the current anim frame
	animTime					+= dt_ms * 0.001f;
	//animTime = 0;
	const int frameRate			= spiderAnim->FrameRate();
	const int numFrames			= spiderAnim->NumberOfFrames();
	const float totalAnimTime	= (float)numFrames / (float)frameRate;
	const float percentTime		= animTime / totalAnimTime;
	int currentFrame			= percentTime * numFrames;
	
	if ( animTime >= totalAnimTime ) {
		mRenderPosition	= mPosition;
		animTime		= 0.0f;
		currentFrame	= 0;
	}

	// Animate and update mesh
	const md5Skeleton_t & skeleton = (*spiderAnim)[ currentFrame ];
	mModel.PrepareMesh( skeleton );
	mModel.UpdateVBO();

	mSkeleton = &skeleton;

	// Get the skeleton's base position in world space
	Vec3 pos3 = (*mSkeleton)[ 0 ].mPosition * ( 1.0f / 64.0f );
	Vec4 pos4 = Vec4( pos3.x, pos3.y, pos3.z, 1.0f );
	pos4 = GetOrientationMatrix() * pos4;
	mPosition = Vec3( pos4.x, pos4.y, pos4.z );

#if 0
	// Move the Spider and collide with the scene
	const Vec3 velocity( 0, 0, -1 );
	const Vec3 tmpPos	= mPosition + Vec3( 0, 0, sRadius );
	mSphere					= hbSphere( tmpPos + velocity, sRadius );
	scene.CollideSphere( mSphere );

	// Update Spider position after collision
	const Vec3 dr	= mSphere.mCenter - tmpPos;
	mRenderPosition		+= dr;
	mPosition			+= dr;
    
    mRenderEntity.mOrientation = GetOrientationMatrix();
#endif
#endif
}

/*
================================
Spider::Draw
================================
*/
void Spider::Draw() {
	mModel.Draw();
}

/*
 ================================
 Spider::GetOrientationMatrix
 ================================
 */
Mat4 Spider::GetOrientationMatrix() const {
	Mat4 matOrient;
	matOrient.Orient( mRenderPosition, mLookDir, Vec3( 0, 0, 1 ) );
	return matOrient;
}
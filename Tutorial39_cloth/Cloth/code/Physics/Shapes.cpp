//
//  Shapes.cpp
//
#include "Physics/Shapes.h"

static const float w = 50;
static const float h = 25;

Vec3 g_boxGround[] = {
	Vec3(-w,-h, 0 ),
	Vec3( w,-h, 0 ),
	Vec3(-w, h, 0 ),
	Vec3( w, h, 0 ),

	Vec3(-w,-h,-1 ),
	Vec3( w,-h,-1 ),
	Vec3(-w, h,-1 ),
	Vec3( w, h,-1 ),
};

Vec3 g_boxWall0[] = {
	Vec3(-1,-h, 0 ),
	Vec3( 1,-h, 0 ),
	Vec3(-1, h, 0 ),
	Vec3( 1, h, 0 ),

	Vec3(-1,-h, 5 ),
	Vec3( 1,-h, 5 ),
	Vec3(-1, h, 5 ),
	Vec3( 1, h, 5 ),
};

Vec3 g_boxWall1[] = {
	Vec3(-w,-1, 0 ),
	Vec3( w,-1, 0 ),
	Vec3(-w, 1, 0 ),
	Vec3( w, 1, 0 ),

	Vec3(-w,-1, 5 ),
	Vec3( w,-1, 5 ),
	Vec3(-w, 1, 5 ),
	Vec3( w, 1, 5 ),
};

Vec3 g_boxUnit[] = {
	Vec3(-1,-1,-1 ),
	Vec3( 1,-1,-1 ),
	Vec3(-1, 1,-1 ),
	Vec3( 1, 1,-1 ),

	Vec3(-1,-1, 1 ),
	Vec3( 1,-1, 1 ),
	Vec3(-1, 1, 1 ),
	Vec3( 1, 1, 1 ),
};

static const float t = 0.25f;
Vec3 g_boxSmall[] = {
	Vec3(-t,-t,-t ),
	Vec3( t,-t,-t ),
	Vec3(-t, t,-t ),
	Vec3( t, t,-t ),

	Vec3(-t,-t, t ),
	Vec3( t,-t, t ),
	Vec3(-t, t, t ),
	Vec3( t, t, t ),
};

static const float l = 3.0f;
Vec3 g_boxBeam[] = {
	Vec3(-l,-t,-t ),
	Vec3( l,-t,-t ),
	Vec3(-l, t,-t ),
	Vec3( l, t,-t ),

	Vec3(-l,-t, t ),
	Vec3( l,-t, t ),
	Vec3(-l, t, t ),
	Vec3( l, t, t ),
};

Vec3 g_boxPlatform[] = {
	Vec3(-l,-l,-t ),
	Vec3( l,-l,-t ),
	Vec3(-l, l,-t ),
	Vec3( l, l,-t ),

	Vec3(-l,-l, t ),
	Vec3( l,-l, t ),
	Vec3(-l, l, t ),
	Vec3( l, l, t ),
};

static const float t2 = 0.25f;//0.25f;
static const float w2 = t2 * 2.0f;//0.25f;
static const float h3 = t2 * 4.0f;
Vec3 g_boxBody[] = {
	Vec3(-t2,-w2,-h3 ),
	Vec3( t2,-w2,-h3 ),
	Vec3(-t2, w2,-h3 ),
	Vec3( t2, w2,-h3 ),

	Vec3(-t2,-w2, h3 ),
	Vec3( t2,-w2, h3 ),
	Vec3(-t2, w2, h3 ),
	Vec3( t2, w2, h3 ),
};

static const float h2 = 0.25f;
Vec3 g_boxLimb[] = {
	Vec3(-h3,-h2,-h2 ),
	Vec3( h3,-h2,-h2 ),
	Vec3(-h3, h2,-h2 ),
	Vec3( h3, h2,-h2 ),

	Vec3(-h3,-h2, h2 ),
	Vec3( h3,-h2, h2 ),
	Vec3(-h3, h2, h2 ),
	Vec3( h3, h2, h2 ),
};

Vec3 g_boxHead[] = {
	Vec3(-h2,-h2,-h2 ),
	Vec3( h2,-h2,-h2 ),
	Vec3(-h2, h2,-h2 ),
	Vec3( h2, h2,-h2 ),

	Vec3(-h2,-h2, h2 ),
	Vec3( h2,-h2, h2 ),
	Vec3(-h2, h2, h2 ),
	Vec3( h2, h2, h2 ),
};

void FillPencil();
void FillOctagon();
void FillPentagon();
void FillPentagonShapes();

Vec3 g_diamond[ 7 * 8 ];
void FillDiamond() {
	Vec3 pts[ 4 + 4 ];
	pts[ 0 ] = Vec3( 0.1f, 0, -1 );
	pts[ 1 ] = Vec3( 1, 0, 0 );
	pts[ 2 ] = Vec3( 1, 0, 0.1f );
	//pts[ 3 ] = Vec3( 1, 0, 0.2f );
	pts[ 3 ] = Vec3( 0.4f, 0, 0.4f );

	const float pi = acosf( -1.0f );
	const Quat quatHalf( Vec3( 0, 0, 1 ), 2.0f * pi * 0.125f * 0.5f );
	pts[ 4 ] = Vec3( 0.8f, 0, 0.3f );
	pts[ 4 ] = quatHalf.RotatePoint( pts[ 4 ] );
	pts[ 5 ] = quatHalf.RotatePoint( pts[ 1 ] );
	pts[ 6 ] = quatHalf.RotatePoint( pts[ 2 ] );

	const Quat quat( Vec3( 0, 0, 1 ), 2.0f * pi * 0.125f );
	int idx = 0;
	for ( int i = 0; i < 7; i++ ) {
		g_diamond[ idx ] = pts[ i ];
		idx++;
	}

	Quat quatAccumulator;
	for ( int i = 1; i < 8; i++ ) {
		quatAccumulator = quatAccumulator * quat;
		for ( int pt = 0; pt < 7; pt++ ) {
			g_diamond[ idx ] = quatAccumulator.RotatePoint( pts[ pt ] );
			idx++;
		}
	}

	FillPencil();
	FillOctagon();
	FillPentagon();
	FillPentagonShapes();
}

Vec3 g_pencil[ 7 * 8 ];
void FillPencil() {
	Vec3 pts[ 4 + 4 ];
	pts[ 0 ] = Vec3( 0.1f, 0, -1 );
	pts[ 1 ] = Vec3( 0.8f, 0, 0 );
	pts[ 2 ] = Vec3( 0.5f, 0, 2.9f );
	//pts[ 3 ] = Vec3( 1, 0, 0.2f );
	pts[ 3 ] = Vec3( 0.2f, 0, 3.0f );//0.4f );

	const float pi = acosf( -1.0f );
	const Quat quatHalf( Vec3( 0, 0, 1 ), 2.0f * pi * 0.125f * 0.5f );
	pts[ 4 ] = Vec3( 0.6f, 0, 0.3f );
	pts[ 4 ] = quatHalf.RotatePoint( pts[ 4 ] );
	pts[ 5 ] = quatHalf.RotatePoint( pts[ 1 ] );
	pts[ 6 ] = quatHalf.RotatePoint( pts[ 2 ] );

	const Quat quat( Vec3( 0, 0, 1 ), 2.0f * pi * 0.125f );
	int idx = 0;
	for ( int i = 0; i < 7; i++ ) {
		g_pencil[ idx ] = pts[ i ];
		idx++;
	}

	Quat quatAccumulator;
	for ( int i = 1; i < 8; i++ ) {
		quatAccumulator = quatAccumulator * quat;
		for ( int pt = 0; pt < 7; pt++ ) {
			g_pencil[ idx ] = quatAccumulator.RotatePoint( pts[ pt ] );
			idx++;
		}
	}
}

Vec3 g_octagonGround[ 8 * 2 ];
void FillOctagon() {
	const float radius = 100.0f;

	Vec3 pts[ 8 * 2 ];
	pts[ 0 ] = Vec3( radius, 0, 0 );
	pts[ 1 ] = Vec3( radius, 0, -100 );

	const float pi = acosf( -1.0f );
	const float angle = 2.0f * pi * 0.125f;

	for ( int i = 1; i < 8; i++ ) {
		const Quat quatRotator( Vec3( 0, 0, 1 ), angle * (float)i );
		pts[ 2 * i + 0 ] = quatRotator.RotatePoint( pts[ 0 ] );
		pts[ 2 * i + 1 ] = quatRotator.RotatePoint( pts[ 1 ] );
	}

	for ( int i = 0; i < 8 * 2; i++ ) {
		g_octagonGround[ i ] = pts[ i ];
	}
}

Vec3 g_pentagonGround[ 5 * 2 ];
void FillPentagon() {
	const float radius = 100.0f;

	Vec3 pts[ 5 * 2 ];
	pts[ 0 ] = Vec3( radius, 0, 0 );
	pts[ 1 ] = Vec3( radius, 0, -100 );

	const float pi = acosf( -1.0f );
	const float angle = 2.0f * pi * 0.2f;//125f;

	for ( int i = 1; i < 5; i++ ) {
		const Quat quatRotator( Vec3( 0, 0, 1 ), angle * (float)i );
		pts[ 2 * i + 0 ] = quatRotator.RotatePoint( pts[ 0 ] );
		pts[ 2 * i + 1 ] = quatRotator.RotatePoint( pts[ 1 ] );
	}

	for ( int i = 0; i < 5 * 2; i++ ) {
		g_pentagonGround[ i ] = pts[ i ];
	}
}

Vec3 g_pentagonGroundSmall[ 5 * 2 ];
Vec3 g_pentagonGroundPoints[ 5 ][ 3 * 2 ];
Vec3 g_pentagonGroundFillers[ 5 ][ 3 * 2 ] = { Vec3( 0.0f ) };
void FillPentagonShapes() {
	const float radius = 50.0f;;

	Vec3 pts[ 5 * 2 ];
	pts[ 0 ] = Vec3( radius, 0, 0 );
	pts[ 1 ] = Vec3( radius, 0, -100 );

	const float pi = acosf( -1.0f );
	const float angle = 2.0f * pi * 0.2f;//125f;

	for ( int i = 1; i < 5; i++ ) {
		const Quat quatRotator( Vec3( 0, 0, 1 ), angle * (float)i );
		pts[ 2 * i + 0 ] = quatRotator.RotatePoint( pts[ 0 ] );
		pts[ 2 * i + 1 ] = quatRotator.RotatePoint( pts[ 1 ] );
	}

	for ( int i = 0; i < 5 * 2; i++ ) {
		g_pentagonGroundSmall[ i ] = pts[ i ];
	}

	//
	//	Points
	//
	const Quat rotator( Vec3( 0, 0, 1 ), angle );
	const Quat rotatorHalf( Vec3( 0, 0, 1 ), angle * 0.5f );
	const float multiplier = 2.618037f;
	pts[ 0 ] = Vec3( radius, 0, 0 );
	pts[ 1 ] = rotator.RotatePoint( Vec3( radius, 0, 0 ) );
	pts[ 2 ] = rotatorHalf.RotatePoint( Vec3( radius * multiplier, 0, 0 ) );
	pts[ 3 ] = Vec3( radius, 0, -100 );
	pts[ 4 ] = rotator.RotatePoint( Vec3( radius, 0, -100 ) );
	pts[ 5 ] = rotatorHalf.RotatePoint( Vec3( radius * multiplier, 0, -100 ) );

	for ( int i = 0; i < 5; i++ ) {
		const Quat quatRotator( Vec3( 0, 0, 1 ), angle * (float)i );
		for ( int pt = 0; pt < 6; pt++ ) {
			g_pentagonGroundPoints[ i ][ pt ] = quatRotator.RotatePoint( pts[ pt ] );
		}
	}

	//
	//	Fillers
	//
	pts[ 0 ] = rotator.RotatePoint( Vec3( radius, 0, 0 ) );
	pts[ 1 ] = rotatorHalf.RotatePoint( Vec3( radius * multiplier, 0, 0 ) );
	pts[ 2 ] = rotator.RotatePoint( rotatorHalf.RotatePoint( Vec3( radius * multiplier, 0, 0 ) ) );
	pts[ 3 ] = rotator.RotatePoint( Vec3( radius, 0, -100 ) );
	pts[ 4 ] = rotatorHalf.RotatePoint( Vec3( radius * multiplier, 0, -100 ) );
	pts[ 5 ] = rotator.RotatePoint( rotatorHalf.RotatePoint( Vec3( radius * multiplier, 0, -100 ) ) );

	for ( int i = 0; i < 5; i++ ) {
		const Quat quatRotator( Vec3( 0, 0, 1 ), angle * (float)i );
		for ( int pt = 0; pt < 6; pt++ ) {
			g_pentagonGroundFillers[ i ][ pt ] = quatRotator.RotatePoint( pts[ pt ] );
		}
	}
}
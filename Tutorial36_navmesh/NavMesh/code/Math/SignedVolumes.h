//
//	SignedVolumes.h
//
#pragma once
#include "Vector.h"
#include "Quat.h"
#include "../Physics/Shapes.h"


/*
========================================================================================================

SignedVolumes

This is the signed volumes alternative distance algorithm outlined in the paper:
"Improving the GJK algorithm for faster and more reliable distance queries between convex objects"
by Mattia Montanari, Nik Petrinic, and Ettore Barbieri

========================================================================================================
*/

Vec2 SignedVolume1D( const Vec3 & s1, const Vec3 & s2 );
Vec3 SignedVolume2D( const Vec3 & s1, const Vec3 & s2, const Vec3 & s3 );
Vec4 SignedVolume3D( const Vec3 & s1, const Vec3 & s2, const Vec3 & s3, const Vec3 & s4 );

void TestSignedVolumeProjection();
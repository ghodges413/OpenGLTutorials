//
//	lcp.h
//
#pragma once
#include "Vector.h"
#include "Matrix.h"

/*
================================
LCP
Linear Complimentary Problem
Solves the set of linear equations Ax = b
================================
*/

Vec3 LCP_GaussSeidel( const Mat3 & A, const Vec3 & b );
VecN LCP_GaussSeidel( const MatN & A, const VecN & b );

void LCP_Test();
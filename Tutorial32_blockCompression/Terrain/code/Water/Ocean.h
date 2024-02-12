//
//  Ocean.cpp
//
#pragma once
#include "Math/vector.h"
#include "Math/Matrix.h"

struct OceanUpdateParms_t {
	float m_timeMS;
};
void OceanUpdate( const OceanUpdateParms_t & parms );

struct OceanFillGBufferParms_t {
	Matrix m_matView;
	Matrix m_matProj;
	Matrix m_matViewProj;
	Vec3d m_camPos;
	Vec3d m_dirToSun;
};
void OceanFillGBuffer( const OceanFillGBufferParms_t & parms );

struct OceanDrawParms_t {
	Matrix m_matView;
	Matrix m_matProj;
	Matrix m_matViewProj;
	Vec3d m_camPos;
	Vec3d m_dirToSun;
};
void OceanDraw( const OceanDrawParms_t & parms );

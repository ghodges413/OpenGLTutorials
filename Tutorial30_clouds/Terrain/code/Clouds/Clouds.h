//
//  Clouds.cpp
//
#pragma once
#include "Math/vector.h"
#include "Math/Matrix.h"

void CloudInit();

// struct CloudUpdateParms_t {
// 	float m_timeMS;
// };
// void CloudUpdate( const CloudUpdateParms_t & parms );

struct CloudDrawParms_t {
	Matrix m_matView;
	Matrix m_matProj;
	Matrix m_matViewProj;
	Vec3d m_camPos;
	Vec3d m_dirToSun;
	float m_time;
};
void CloudDraw( const CloudDrawParms_t & parms );

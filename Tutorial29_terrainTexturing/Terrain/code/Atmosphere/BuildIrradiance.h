//
//  BuildIrradiance.h
//
#pragma once
#ifndef __BuildIrradiance__H_
#define __BuildIrradiance__H_

#include "Math/Vector.h"
#include "Atmosphere/BrunetonUtils.h"

void InitIrradiance( const BrunetonData_t & data, const Vec4d * transmittance, Vec4d * image );
void DeltaGroundIrradiance( const BrunetonData_t & data, const Vec4d * deltaScatter, Vec4d * image );

#endif /* defined(__BuildIrradiance__H_) */

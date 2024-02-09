//
//  BuildScatter.h
//
#pragma once
#ifndef __BuildScatter__H_
#define __BuildScatter__H_

#include "Math/Vector.h"
#include "Atmosphere/BrunetonUtils.h"

void SingleScattering( const BrunetonData_t & data, const Vec4d * transmittance, Vec4d * image, Vec4d * deltaScatter );

void DeltaLightIrradianceMT( const BrunetonData_t & data, const Vec4d * transmittance, const Vec4d * deltaIrradiance, const Vec4d * deltaScatter, Vec4d * image );
void DeltaLightIrradiance( const BrunetonData_t & data, const Vec4d * transmittance, const Vec4d * deltaIrradiance, const Vec4d * deltaScatter, Vec4d * image );
void DeltaScatter( const BrunetonData_t & data, const Vec4d * transmittance, const Vec4d * deltaLightScatterTable, Vec4d * image );

#endif /* defined(__BuildScatter__H_) */

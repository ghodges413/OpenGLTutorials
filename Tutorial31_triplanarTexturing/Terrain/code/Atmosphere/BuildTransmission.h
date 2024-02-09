//
//  BuildTransmission.h
//
#pragma once
#ifndef __BuildTransmission__H_
#define __BuildTransmission__H_

#include "Math/Vector.h"
#include "Atmosphere/BrunetonUtils.h"

/*
 =====================================
 Build Transmission
 =====================================
 */
void BuildTransmission( const BrunetonData_t & data, Vec4d * image );

#endif /* defined(__BuildTransmission__H_) */

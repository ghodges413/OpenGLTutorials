//
//  Common.h
//
#pragma once
#include "../Vector.h"
#include "../Mesh.h"
#include "../RenderSurface.h"
#include "../Shader.h"
#include "../Types.h"
#include <unordered_map>

struct PixelFeedback_t {
	uint8 r;
	uint8 g;
	uint8 b;
	uint8 a;

	bool operator==( const PixelFeedback_t & rhs ) {
		if ( r != rhs.r ) {
			return false;
		}
		if ( g != rhs.g ) {
			return false;
		}
		if ( b != rhs.b ) {
			return false;
		}
		if ( a != rhs.a ) {
			return false;
		}
		return true;
	}

	// When drawing the sampler feedback, we clear the buffer to
	// white to signal that it is not a part of the mega-texture
	bool IsValid() const {
		if ( r != 0xFF ) {
			return true;
		}
		if ( g != 0xFF ) {
			return true;
		}
		if ( b != 0xFF ) {
			return true;
		}
		if ( a != 0xFF ) {
			return true;
		}
		return false;
	}
};


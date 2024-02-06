//
//  WorelyNoise.cpp
//
#pragma once
#include "Math/Vector.h"

/*
The MIT License (MIT)

Copyright(c) 2017 Sébastien Hillaire

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated 
documentation files (the "Software"), to deal in the Software without restriction, including without limitation 
the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, 
and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED 
TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL 
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF 
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
DEALINGS IN THE SOFTWARE.

https://github.com/sebh/TileableVolumeNoise/tree/master
*/

class Tileable3dNoise {
public:
	/// @return Tileable Worley noise value in [0, 1].
	/// @param p 3d coordinate in [0, 1], being the range of the repeatable pattern.
	/// @param cellCount the number of cell for the repetitive pattern.
	static float WorleyNoise( const Vec3d & p, float cellCount );

	/// @return Tileable Perlin noise value in [0, 1].
	/// @param p 3d coordinate in [0, 1], being the range of the repeatable pattern.
	static float PerlinNoise( const Vec3d & p, float frequency, int octaveCount );

private:

	///
	/// Worley noise function based on https://www.shadertoy.com/view/Xl2XRR by Marc-Andre Loyer
	///

	static float Hash( float n );
	static float Noise( const Vec3d & x );
	static float Cells( const Vec3d & p, float numCells );

};

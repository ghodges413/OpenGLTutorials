//
//  Matrix.cpp
//
#include "Math/Matrix.h"

/*
 ================================
 Matrix::RotationMatrix
 ================================
 */
Matrix Matrix::RotationMatrix( const float x, const float y, const float z, const float angleDegrees ) {
    Matrix mat;
    mat.SetRotationMatrix( x, y, z, angleDegrees );
    return mat;
}

/*
 ================================
 Matrix::RotationMatrix
 ================================
 */
Matrix Matrix::RotationMatrix( const Vec3d & axis, const float angleDegrees ) {
    Matrix mat;
    mat.SetRotationMatrix( axis, angleDegrees );
    return mat;
}

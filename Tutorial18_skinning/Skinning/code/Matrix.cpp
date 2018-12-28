/*
 *  Matrix.cpp
 *
 */

#include "Matrix.h"
#include "Quat.h"

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

/*
 ================================
 Matrix::ToQuaternion
 ================================
 */
Quat Matrix::ToQuaternion() const {
    Quat quat;
	/*
	float tr = m00 + m11 + m22

	if (tr > 0) { 
	  float S = sqrt(tr+1.0) * 2; // S=4*qw 
	  qw = 0.25 * S;
	  qx = (m21 - m12) / S;
	  qy = (m02 - m20) / S; 
	  qz = (m10 - m01) / S; 
	} else if ((m00 > m11)&(m00 > m22)) { 
	  float S = sqrt(1.0 + m00 - m11 - m22) * 2; // S=4*qx 
	  qw = (m21 - m12) / S;
	  qx = 0.25 * S;
	  qy = (m01 + m10) / S; 
	  qz = (m02 + m20) / S; 
	} else if (m11 > m22) { 
	  float S = sqrt(1.0 + m11 - m00 - m22) * 2; // S=4*qy
	  qw = (m02 - m20) / S;
	  qx = (m01 + m10) / S; 
	  qy = 0.25 * S;
	  qz = (m12 + m21) / S; 
	} else { 
	  float S = sqrt(1.0 + m22 - m00 - m11) * 2; // S=4*qz
	  qw = (m10 - m01) / S;
	  qx = (m02 + m20) / S;
	  qy = (m12 + m21) / S;
	  qz = 0.25 * S;
	}
	*/

	const float trace = column0[ 0 ] + column1[ 1 ] + column2[ 2 ];
	
	// TODO: test both of these, figure out which is correct
#if 0
	const float m00 = column0[ 0 ];
	const float m01 = column0[ 1 ];
	const float m02 = column0[ 2 ];

	const float m10 = column1[ 0 ];
	const float m11 = column1[ 1 ];
	const float m12 = column1[ 2 ];

	const float m20 = column2[ 0 ];
	const float m21 = column2[ 1 ];
	const float m22 = column2[ 2 ];
#else
	const float m00 = column0[ 0 ];
	const float m10 = column0[ 1 ];
	const float m20 = column0[ 2 ];

	const float m01 = column1[ 0 ];
	const float m11 = column1[ 1 ];
	const float m21 = column1[ 2 ];

	const float m02 = column2[ 0 ];
	const float m12 = column2[ 1 ];
	const float m22 = column2[ 2 ];
#endif
	
	// TODO: handle zeros and undefines better

	if ( trace > 0.0f ) { 
		const float S = sqrtf( trace + 1.0f ) * 2.0f; // S=4*qw 
		const float invS = 1.0f / S;
		quat.w = 0.25f * S;
		quat.x = ( m21 - m12 ) * invS;
		quat.y = ( m02 - m20 ) * invS;
		quat.z = ( m10 - m01 ) * invS;
	} else if ( ( m00 > m11 ) & ( m00 > m22 ) ) { 
		const float S = sqrtf( 1.0f + m00 - m11 - m22 ) * 2.0f; // S=4*qx
		const float invS = 1.0f / S;
		quat.w = ( m21 - m12 ) * invS;
		quat.x = 0.25f * S;
		quat.y = ( m01 + m10 ) * invS;
		quat.z = ( m02 + m20 ) * invS;
	} else if ( m11 > m22 ) { 
		const float S = sqrtf( 1.0f + m11 - m00 - m22 ) * 2.0f; // S=4*qy
		const float invS = 1.0f / S;
		quat.w = ( m02 - m20 ) * invS;
		quat.x = ( m01 + m10 ) * invS;
		quat.y = 0.25f * S;
		quat.z = ( m12 + m21 ) * invS;
	} else { 
		const float S = sqrtf( 1.0f + m22 - m00 - m11 ) * 2.0f; // S=4*qz
		const float invS = 1.0f / S;
		quat.w = ( m10 - m01 ) * invS;
		quat.x = ( m02 + m20 ) * invS;
		quat.y = ( m12 + m21 ) * invS;
		quat.z = 0.25f * S;
	}

	return quat;
}
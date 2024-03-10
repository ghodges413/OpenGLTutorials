//
//	MD5Model.cpp
//
#include "Animation/MD5Model.h"
#include <stdio.h>
#include <stdlib.h>
#include "Graphics/Graphics.h"
#include "Miscellaneous/Fileio.h"

/*
 ====================================
 MD5Model::Load
 ====================================
 */
bool MD5Model::Load( const char * fileName ) {
	printf( "Loading model: %s\n", fileName );
	char buff[ 512 ] = { 0 };
	int version = 0;
	int currentMesh = 0;
	
	int numJoints = 0;
	int numMeshes = 0;
	mJoints.Clear();
	mMeshes.Clear();

	char fullPath[ 2048 ];
	RelativePathToFullPath( fileName, fullPath );	
	
	FILE * fp = fopen( fullPath, "rb" );
	if ( !fp ) {
		fprintf( stderr, "Error: couldn't open \"%s\"!\n", fullPath );
		return false;
    }
	
	while ( !feof( fp ) ) {
		// Read whole line
		fgets( buff, sizeof( buff ), fp );
		
		if ( sscanf( buff, " MD5Version %i", &version ) == 1 ) {
			if ( version != 10) {
				// Bad version 
				fprintf (stderr, "Error: bad model version\n");
				fclose (fp);
				return 0;
			}
		} else if ( sscanf( buff, " numJoints %i", &numJoints ) == 1 ) {		// numJoints
			if ( numJoints > 0 ) {
				// Allocate memory for base skeleton joints 
				mJoints.Resize( numJoints );
				mJoints.Clear();
			}
		} else if ( sscanf( buff, " numMeshes %i", &numMeshes ) == 1 ) {		// numMeshes
			if ( numMeshes > 0) {
				// Allocate memory for meshes 
				mMeshes.Resize( numMeshes );
				mMeshes.Clear();
			}
		} else if ( strncmp( buff, "joints {", 8 ) == 0 ) {		// parse the joints
			// Read each joint
			for ( int i = 0; i < numJoints; ++i ) {
				// Read whole line 
				fgets( buff, sizeof( buff ), fp );
				char name[ 256 ];
				int parentID;
				float x;
				float y;
				float z;
				float qx;
				float qy;
				float qz;
				
				if ( sscanf( buff, "%s %i ( %f %f %f ) ( %f %f %f )",
							 name, &parentID,
							 &x, &y, &z,
							 &qx, &qy, &qz ) == 8 ) {
					md5Joint joint;
					//joint.mName = name;
					joint.mParentID = parentID;
					joint.mPosition = Vec3( x, y, z );
					joint.mOrientation = Quat( qx, qy, qz );
					mJoints.Append( joint );
				}
			}
		} else if ( strncmp( buff, "mesh {", 6 ) == 0 ) {				// read in the meshes
			md5Mesh mesh;
			int vert_index = 0;
			int tri_index = 0;
			int weight_index = 0;
			float fdata[ 4 ];
			int idata[ 3 ];
			
			while ( ( buff[ 0 ] != '}' ) && !feof( fp ) ) {
				int numVerts;
				int numTris;
				int numWeights;
				char shader[ 1024 ];
				
				// Read whole line
				fgets( buff, sizeof( buff ), fp );
				
				if ( strstr( buff, "shader " ) ) {
					int quote = 0, j = 0;
					
					// Copy the shader name whithout the quote marks
					for ( int i = 0; i < sizeof( buff ) && ( quote < 2 ); ++i ) {
						if ( buff[ i ] == '\"' ) {
							++quote;
						}
						
						if ( ( quote == 1 ) && ( buff[ i ] != '\"' ) ) {
							shader[ j ] = buff[ i ];
							++j;
						}
					}
				} else if ( sscanf( buff, " numverts %i", &numVerts ) == 1 ) {		// numVerts
					if ( numVerts > 0 ) {
						// Allocate memory for vertices
						mesh.mVerts.Resize( numVerts );
						mesh.mVerts.Clear();
					}
				} else if ( sscanf( buff, " numtris %i", &numTris ) == 1 ) {		// numTris
					if ( numTris > 0 ) {
						// Allocate memory for triangles
						mesh.mTriIndices.Resize( numTris * 3 );
						mesh.mTriIndices.Clear();
					}
				} else if ( sscanf( buff, " numweights %i", &numWeights ) == 1 ) {		// numWeights
					if ( numWeights > 0 ) {
						// Allocate memory for vertex weights
						mesh.mWeights.Resize( numWeights );
						mesh.mWeights.Clear();
					}
				} else if ( sscanf( buff, " vert %i ( %f %f ) %i %i", &vert_index,
								 &fdata[ 0 ], &fdata[ 1 ], &idata[ 0 ], &idata[ 1 ] ) == 5 ) {	// copy Verts
					// Copy vertex data
					md5Vertex vert;
					vert.mST.x = fdata[ 0 ];
					vert.mST.y = fdata[ 1 ];
					vert.mStartWeight = idata[ 0 ];
					vert.mWeightCount = idata[ 1 ];
                    for ( int w = 0; w < MAX_MD5_WEIGHTS; ++w ) {
                        vert.mWeights[ w ] = 0;
                    }
                    for ( int w = 0; w < vert.mWeightCount; ++w ) {
                        assert( w < MAX_MD5_WEIGHTS );
                        vert.mWeights[ w ] = vert.mStartWeight + w;
                    }
					mesh.mVerts.Append( vert );
				} else if ( sscanf( buff, " tri %i %i %i %i", &tri_index,
								 &idata[ 0 ], &idata[ 1 ], &idata[ 2 ] ) == 4 ) {		// copy Tris
					// Copy triangle data
					mesh.mTriIndices.Append( (short)idata[ 0 ] );
					mesh.mTriIndices.Append( (short)idata[ 2 ] );
					mesh.mTriIndices.Append( (short)idata[ 1 ] );
				} else if ( sscanf( buff, " weight %i %i %f ( %f %f %f )",
								 &weight_index, &idata[ 0 ], &fdata[ 3 ],
								 &fdata[ 0 ], &fdata[ 1 ], &fdata[ 2 ] ) == 6 ) {		// copy Weights
					// Copy vertex data
					md5Weight weight;
					weight.mJointID = idata[ 0 ];
					weight.mBias = fdata[ 3 ];
					weight.mPosition.x = fdata[ 0 ];
					weight.mPosition.y = fdata[ 1 ];
					weight.mPosition.z = fdata[ 2 ];
					mesh.mWeights.Append( weight );
				}
			}
			mMeshes.Append( mesh );
			++currentMesh;
		}
    }
	fclose( fp );
	
#if 0
	// print out the mesh data to screen ( debug only )
	// print out joint info
	for ( int i = 0; i < mJoints.Num(); ++i ) {
		md5Joint & joint = mJoints[ i ];
		printf( "Joint: %s %i ( %f %f %f ) ( %f %f %f )\n",
				joint.name.cstr(),
				joint.parentID,
				joint.position.x, joint.position.y, joint.position.z,
				joint.orientation.x, joint.orientation.y, joint.orientation.z );
	}
	
	// print out mesh info
	printf( "printing meshes: %i\n", (int)mMeshes.Num() );
	for ( int i = 0; i < mMeshes.Num(); ++i ) {
		md5Mesh & mesh = mMeshes[ i ];
		
		// verts
		for ( int j = 0; j < mesh.verts.Num(); ++j ) {
			md5Vertex & vert = mesh.verts[ j ];
			printf( "Vert: %i ( %f %f ) %i %i\n",
					vert.index,
					vert.st.x, vert.st.y,
					vert.startWeight,
					vert.weightCount );
		}
		
		// tris
		for ( int j = 0; j < mesh.triangles.Num(); ++j ) {
			md5Triangle & triangle = mesh.triangles[ j ];
			printf( "Tri: %i %i %i %i\n",
					triangle.index,
					triangle.vertIdx0,
					triangle.vertIdx1,
					triangle.vertIdx2 );
		}
		
		// weights
		for ( int j = 0; j < mesh.weights.Num(); ++j ) {
			md5Weight & weight = mesh.weights[ j ];
			printf( "Weight: %i %i %f ( %f %f %f )\n",
					weight.index,
					weight.jointID,
				    weight.bias,
				    weight.position.x, weight.position.y, weight.position.z );
		}
	}
#endif
#if defined( USE_ANIMATRIX )
	// fill the base-pose skeleton's matrix versions
    for ( int i = 0; i < mJoints.Num(); ++i ) {
		mJoints[ i ].mRotMat = Matrix( mJoints[ i ].mOrientation );
        mJoints[ i ].mTransMat.SetTranslationMatrix( mJoints[ i ].mPosition );
        mJoints[ i ].mOrientMat = mJoints[ i ].mRotMat * mJoints[ i ].mTransMat;
    }
#elif 0
    // fill the base-pose skeleton's matrix versions
    for ( int i = 0; i < mJoints.Num(); ++i ) {
// 		Mat4 mat4 = mJoints[ i ].mOrientation.ToMat4();
// 		mJoints[ i ].mRotMat = Matrix( mat4.ToPtr() );
//		mJoints[ i ].mRotMat = Matrix( mJoints[ i ].mOrientation );
        mJoints[ i ].mRotMat = mJoints[ i ].mOrientation.ToMat4();
		//mJoints[ i ].mRotMat = mJoints[ i ].mOrientation.ToMat4().Transpose();
        mJoints[ i ].mTransMat.SetTranslationMatrix( mJoints[ i ].mPosition );
        mJoints[ i ].mOrientMat = mJoints[ i ].mRotMat * mJoints[ i ].mTransMat;
// 		mJoints[ i ].mOrientMat = mJoints[ i ].mOrientMat.Transpose();
 		mJoints[ i ].mOrientMat = mJoints[ i ].mTransMat * mJoints[ i ].mRotMat;
    }
#else
	// fill the base-pose skeleton's matrix versions
    for ( int i = 0; i < mJoints.Num(); ++i ) {
        mJoints[ i ].mRotMat = mJoints[ i ].mOrientation.ToMat4();
        mJoints[ i ].mTransMat.SetTranslationMatrix( mJoints[ i ].mPosition );
 		mJoints[ i ].mOrientMat = mJoints[ i ].mTransMat * mJoints[ i ].mRotMat;
		mJoints[ i ].mOrientMat = mJoints[ i ].mRotMat * mJoints[ i ].mTransMat;
		mJoints[ i ].mOrientMat = mJoints[ i ].mOrientMat.Transpose();
    }
#endif
	//
	// Pre-size and fill data members
	//
	{
		// Pre-size containers
		for ( unsigned int i = 0; i < mMeshes.Num(); ++i ) {
			md5Mesh & mesh	= mMeshes[ i ];
			
			mesh.mTransformedVertices.Resize( mesh.mVerts.Num() );
			mesh.mTransformedVertices.Clear();
		}
	}
    
	PrepareMesh( mJoints );
    CalculateNormals( mJoints );
	return true;
}
#if !defined( USE_ANIMATRIX )
/*
 ====================================
 MD5Model::PrepareMesh
 * cpu skinning
 ====================================
 */
void MD5Model::PrepareMesh( const md5Skeleton_t & skeleton ) {	
	//
	//	Verts and Faces
	//
	for ( int meshIdx = 0; meshIdx < mMeshes.Num(); ++meshIdx ) {
		md5Mesh & mesh = mMeshes[ meshIdx ];

		// Clear the transformed meshes vertices
		mesh.mTransformedVertices.Clear();

		// Setup vertices
		for ( int i = 0; i < mesh.mVerts.Num(); ++i ) { 
			Vec4 finalVertex( 0.0f );
			Vec4 finalNorm( 0.0f );
            Vec4 finalTang( 0.0f );

			vert_t vert;
			
			// Calculate final vertex to draw with weights
			float weightSum = 0;
			for ( int j = 0; j < mesh.mVerts[ i ].mWeightCount; ++j ) {
				const md5Weight & weight   = mesh.mWeights[ mesh.mVerts[ i ].mStartWeight + j ];
				const md5Joint & joint     = skeleton[ weight.mJointID ];

				Vec4 pos = Vec4( weight.mPosition.x, weight.mPosition.y, weight.mPosition.z, 1.0f );
				Vec4 norm = Vec4( weight.mNormal.x, weight.mNormal.y, weight.mNormal.z, 0.0f );
				Vec4 tang = Vec4( weight.mTangnt.x, weight.mTangnt.y, weight.mTangnt.z, 0.0f );

                // Calculate transformed vertex for this weight
                finalVertex += ( joint.mOrientMat * pos ) * weight.mBias;

				// Transform the tangent space
                finalNorm += ( joint.mOrientMat * ( norm + pos ) ) * weight.mBias;
                finalTang += ( joint.mOrientMat * ( tang + pos ) ) * weight.mBias;

				// The sum of all weight->bias should be 1.0
				weightSum += weight.mBias;
			}
            
            vert.st    = mesh.mVerts[ i ].mST;
            vert.pos   = Vec3( finalVertex.x, finalVertex.y, finalVertex.z ); / 64.0f;

			finalNorm = finalNorm - finalVertex;
			finalTang = finalTang - finalVertex;

			Vec3 norm  = Vec3( finalNorm.x, finalNorm.y, finalNorm.z );
			Vec3 tang  = Vec3( finalTang.x, finalTang.y, finalTang.z );

			norm.Normalize();
			tang.Normalize();

			Vec3ToByte4_n11( vert.norm, norm );
			Vec3ToByte4_n11( vert.tang, tang );
			
			// Append the transformed mesh vertices
			mesh.mTransformedVertices.Append( vert );
		}
	}
}
#else
/*
 ====================================
 MD5Model::PrepareMesh
 * cpu skinning
 ====================================
 */
void MD5Model::PrepareMesh( const md5Skeleton_t & skeleton ) {	
	//
	//	Verts and Faces
	//
	for ( int meshIdx = 0; meshIdx < mMeshes.Num(); ++meshIdx ) {
		md5Mesh & mesh = mMeshes[ meshIdx ];

		// Clear the transformed meshes vertices
		mesh.mTransformedVertices.Clear();

		// Setup vertices
		for ( int i = 0; i < mesh.mVerts.Num(); ++i ) { 
			Vec3 finalVertex( 0.0f );
			Vec3 finalNorm( 0.0f );
            Vec3 finalTang( 0.0f );

			vert_t vert;
			
			// Calculate final vertex to draw with weights
			float weightSum = 0;
			for ( int j = 0; j < mesh.mVerts[ i ].mWeightCount; ++j ) {
				const md5Weight & weight   = mesh.mWeights[ mesh.mVerts[ i ].mStartWeight + j ];
				const md5Joint & joint     = skeleton[ weight.mJointID ];

                // Calculate transformed vertex for this weight
                finalVertex += ( joint.mOrientMat * weight.mPosition ) * weight.mBias;

				// Transform the tangent space
                finalNorm += ( joint.mOrientMat * ( weight.mNormal + weight.mPosition ) ) * weight.mBias;
                finalTang += ( joint.mOrientMat * ( weight.mTangnt + weight.mPosition ) ) * weight.mBias;

				// The sum of all weight->bias should be 1.0
				weightSum += weight.mBias;
			}
            
            vert.st    = mesh.mVerts[ i ].mST;
            vert.pos   = finalVertex * ( 1.0f / 64.0f );

			finalNorm = finalNorm - finalVertex;
			finalTang = finalTang - finalVertex;

			Vec3 norm  = Vec3( finalNorm.x, finalNorm.y, finalNorm.z );
			Vec3 tang  = Vec3( finalTang.x, finalTang.y, finalTang.z );

			norm.Normalize();
			tang.Normalize();

			Vec3ToByte4_n11( vert.norm, norm );
			Vec3ToByte4_n11( vert.tang, tang );
			
			// Append the transformed mesh vertices
			mesh.mTransformedVertices.Append( vert );
		}
	}
}
#endif

/*
 ====================================
 MD5Model::CalculateNormals
 ====================================
 */
void MD5Model::CalculateNormals( const md5Skeleton_t & skeleton ) {
    // for each vertex, discover what triangles it is a part of...
    Array< int > tris;
    tris.Resize( 32 );
	for ( int m = 0; m < mMeshes.Num(); ++m ) {
		md5Mesh & mesh = mMeshes[ m ];
		
		for ( int v = 0; v < mesh.mTransformedVertices.Num(); ++v ) {
			tris.Clear();
			for ( int t = 0; t < mMeshes[ m ].mTriIndices.Num(); t += 3 ) {
				if ( mMeshes[ m ].mTriIndices[ t + 0 ] == v ||
					mMeshes[ m ].mTriIndices[ t + 1 ] == v ||
					mMeshes[ m ].mTriIndices[ t + 2 ] == v ) {
					tris.Append( t );
				}
			}
        
			// now that we know how many triangles we have
			// calculate the normal of each tri... then add them up
			Vec3 vNorm = Vec3( 0, 0, 0 );
			Vec3 vTang = Vec3( 0, 0, 0 );
			for ( int t = 0; t < tris.Num(); ++t ) {
				// grab the triangle
				int tidx = tris[ t ];
				int idxA = mMeshes[ m ].mTriIndices[ tidx + 0 ];
				int idxB = mMeshes[ m ].mTriIndices[ tidx + 1 ];
				int idxC = mMeshes[ m ].mTriIndices[ tidx + 2 ];
            
				// grab the vertices to the triangle
				Vec3 vA = mesh.mTransformedVertices[ idxA ].pos;
				Vec3 vB = mesh.mTransformedVertices[ idxB ].pos;
				Vec3 vC = mesh.mTransformedVertices[ idxC ].pos;
            
				Vec3 vAB = vB - vA;
				Vec3 vAC = vC - vA;
     
				// add up the normals
				vNorm += vAB.Cross( vAC );
            
				// get the ST mapping values for the triangle
				Vec2 stA = mesh.mTransformedVertices[ idxA ].st;
				Vec2 stB = mesh.mTransformedVertices[ idxB ].st;
				Vec2 stC = mesh.mTransformedVertices[ idxC ].st;
				Vec2 deltaSTab = stB - stA;
				Vec2 deltaSTac = stC - stA;
				deltaSTab.Normalize();
				deltaSTac.Normalize();
            
				Vec3 tangent;
            
				// calculate tangents
				Vec3 v1 = vAC;
				Vec3 v2 = vAB;
				Vec2 st1 = deltaSTac;
				Vec2 st2 = deltaSTab;
				const float denominator = ( st1.x * st2.y - st2.x * st1.y );
				float coef = 1.0f / denominator;

				// HACK: This is a hack for models with bad st coordinates
				if ( denominator < 0.01f ) {
					coef = 1.0f;
				}

				// Calculate the tangent
				tangent.x = coef * ( ( v1.x * st2.y ) + ( v2.x * -st1.y ) );
				tangent.y = coef * ( ( v1.y * st2.y ) + ( v2.y * -st1.y ) );
				tangent.z = coef * ( ( v1.z * st2.y ) + ( v2.z * -st1.y ) );
            
				// sum up the weighted tangents
				vTang += tangent;
			}
			
			// Ensure the normal space is normalized
			vNorm.Normalize();
			vTang.Normalize();

			// HACK: This is a hack for models with bad st coordinates
			if ( vTang.GetMagnitude() < 0.9f ) {
				vTang = vNorm.Cross( Vec3( 0, 0, 1 ) );
				if ( vTang.GetMagnitude() < 0.9f ) {
					vTang = Vec3( 1, 0, 0 );
				}
			}

			Vec3 biTang = vNorm.Cross( vTang );
			vTang = biTang.Cross( vNorm );
			vTang.Normalize();

			// Store the calculated normal space
			Vec3ToByte4_n11( mesh.mTransformedVertices[ v ].norm, vNorm );
			Vec3ToByte4_n11( mesh.mTransformedVertices[ v ].tang, vTang );
        
			// now here's what we need to do...
			// we need to...
			// decompose the newly computed normals and tangents
			// and store them in each weight
			for ( int w = 0; w < mMeshes[ m ].mVerts[ v ].mWeightCount; ++w ) {
				const int meshID		= m;
				const int weightID		= mMeshes[ m ].mVerts[ v ].mWeights[ w ];
				md5Mesh * mesh          = &mMeshes[ meshID ];
				md5Weight * weight      = &mesh->mWeights[ weightID ];
				const md5Joint * joint  = &skeleton[ weight->mJointID ];

				// get the inverse rotation for this normal
				Quat orient = joint->mOrientation;
				orient.Normalize();
				orient.Invert();
				Quat invOrient = orient;
            
				// decompose the normal
				Vec3 decomposedNormal = invOrient.RotatePoint( vNorm );
				Vec3 decomposedTangnt = invOrient.RotatePoint( vTang );
				decomposedNormal.Normalize();
				decomposedTangnt.Normalize();
            
				// store the decomposed normals & tangents
				weight->mNormal = decomposedNormal;
				weight->mTangnt = decomposedTangnt;
			}
		}
	}
}

/*
====================================
MD5Model::MakeVBO
====================================
*/
void MD5Model::MakeVBO() {
	for ( int i = 0; i < mMeshes.Num(); i++ ) {
		const md5Mesh & mesh = mMeshes[ i ];

		const int sizeIBO = mesh.mTriIndices.Num() * sizeof( unsigned short );
		m_gpuMeshes[ i ].ibo.Generate( GL_ELEMENT_ARRAY_BUFFER, sizeIBO, mesh.mTriIndices.ToPtr(), GL_STATIC_DRAW );

		const int stride = sizeof( vert_t );
		const int sizeVBO = mesh.mTransformedVertices.Num() * stride;
		m_gpuMeshes[ i ].vbo.Generate( GL_ARRAY_BUFFER, sizeVBO, mesh.mTransformedVertices.ToPtr(), GL_DYNAMIC_DRAW );

		m_gpuMeshes[ i ].vao.Generate();
		m_gpuMeshes[ i ].vao.Bind();
    
		m_gpuMeshes[ i ].vbo.Bind();
		m_gpuMeshes[ i ].ibo.Bind();

		unsigned long offset = 0;
		glEnableVertexAttribArray( 0 );
		glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset );
		offset += sizeof( Vec3 );
    
		glEnableVertexAttribArray( 1 );
		glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, stride, (const void *)offset );
		offset += sizeof( Vec2 );
    
		glEnableVertexAttribArray( 2 );
		glVertexAttribPointer( 2, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, (const void *)offset );
		offset += sizeof( unsigned char ) * 4;
    
		glEnableVertexAttribArray( 3 );
		glVertexAttribPointer( 3, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, (const void *)offset );
		offset += sizeof( unsigned char ) * 4;

		glEnableVertexAttribArray( 4 );
		glVertexAttribPointer( 4, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, (const void *)offset );
		offset += sizeof( unsigned char ) * 4;

		m_gpuMeshes[ i ].vao.UnBind();
		m_gpuMeshes[ i ].vbo.UnBind();
		m_gpuMeshes[ i ].ibo.UnBind();
	}
}

/*
====================================
MD5Model::UpdateVBO
====================================
*/
void MD5Model::UpdateVBO() {
	for ( int i = 0; i < mMeshes.Num(); i++ ) {
		m_gpuMeshes[ i ].vbo.Update( mMeshes[ i ].mTransformedVertices.ToPtr() );
	}
}

void MD5Model::Draw() {
	for ( int i = 0; i < mMeshes.Num(); i++ ) {
		m_gpuMeshes[ i ].vao.Bind();

		// Draw
		int num = mMeshes[ i ].mTriIndices.Num();
		glDrawElements( GL_TRIANGLES, num, GL_UNSIGNED_SHORT, 0 );

		m_gpuMeshes[ i ].vao.UnBind();
	}
}
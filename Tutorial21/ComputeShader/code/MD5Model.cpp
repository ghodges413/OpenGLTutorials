/*
 *  MD5Model.cpp
 *
 */
#include "MD5Model.h"
#include <stdio.h>
#include <stdlib.h>
#include "Graphics.h"
#include "Fileio.h"

/*
 ====================================
 MD5Model::MD5Model
 ====================================
 */
MD5Model::MD5Model() {}

/*
 ====================================
 MD5Model::~MD5Model
 ====================================
 */
MD5Model::~MD5Model() {
	for ( int i = 0; i < mMeshesVBO.Num(); ++i ) {
		delete mMeshesVBO[ i ];
		mMeshesVBO[ i ] = NULL;
	}
	mMeshesVBO.Clear();
}

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
					joint.mPosition = Vec3d( x, y, z );
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
    
    // fill the base-pose skeleton's matrix versions
    for ( int i = 0; i < mJoints.Num(); ++i ) {
        mJoints[ i ].mRotMat = mJoints[ i ].mOrientation.ToMatrix();
        mJoints[ i ].mTransMat.SetTranslationMatrix( mJoints[ i ].mPosition );
        mJoints[ i ].mOrientMat = mJoints[ i ].mRotMat * mJoints[ i ].mTransMat;
    }

	//
	// Presize and fill data members
	//
	{
		// Presize containers
		for ( unsigned int i = 0; i < mMeshes.Num(); ++i ) {
			md5Mesh & mesh	= mMeshes[ i ];
			
			mesh.mTransformedVertices.Resize( mesh.mVerts.Num() );
			mesh.mTransformedVertices.Clear();
		}
	}
    
	PrepareMesh( mJoints );
    CalculateNormals( mJoints );
	MakeVBO();
	return true;
}

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
			Vec3d finalVertex( 0.0f );
			Vec3d finalNorm( 0.0f );
            Vec3d finalTang( 0.0f );

			md5Vert vert;
			
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
            
            vert.mST    = mesh.mVerts[ i ].mST;
            vert.mPos   = finalVertex;

			vert.mNorm  = ( finalNorm - finalVertex ).Normalize();
			vert.mTang  = ( finalTang - finalVertex ).Normalize();

			vert.mNorm.Normalize();
			vert.mTang.Normalize();
			
			// Append the transformed mesh vertices
			mesh.mTransformedVertices.Append( vert );
		}
	}
}

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
			Vec3d vNorm = Vec3d( 0, 0, 0 );
			Vec3d vTang = Vec3d( 0, 0, 0 );
			for ( int t = 0; t < tris.Num(); ++t ) {
				// grab the triangle
				int tidx = tris[ t ];
				int idxA = mMeshes[ m ].mTriIndices[ tidx + 0 ];
				int idxB = mMeshes[ m ].mTriIndices[ tidx + 1 ];
				int idxC = mMeshes[ m ].mTriIndices[ tidx + 2 ];
            
				// grab the vertices to the triangle
				Vec3d vA = mesh.mTransformedVertices[ idxA ].mPos;
				Vec3d vB = mesh.mTransformedVertices[ idxB ].mPos;
				Vec3d vC = mesh.mTransformedVertices[ idxC ].mPos;
            
				Vec3d vAB = vB - vA;
				Vec3d vAC = vC - vA;
     
				// add up the normals
				vNorm += vAB.Cross( vAC );
            
				// get the ST mapping values for the triangle
				Vec2d stA = mesh.mTransformedVertices[ idxA ].mST;
				Vec2d stB = mesh.mTransformedVertices[ idxB ].mST;
				Vec2d stC = mesh.mTransformedVertices[ idxC ].mST;
				Vec2d deltaSTab = stB - stA;
				Vec2d deltaSTac = stC - stA;
				deltaSTab.Normalize();
				deltaSTac.Normalize();
            
				Vec3d tangent;
            
				// calculate tangents
				Vec3d v1 = vAC;
				Vec3d v2 = vAB;
				Vec2d st1 = deltaSTac;
				Vec2d st2 = deltaSTab;
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
				vTang = vNorm.Cross( Vec3d( 0, 0, 1 ) );
				if ( vTang.GetMagnitude() < 0.9f ) {
					vTang = Vec3d( 1, 0, 0 );
				}
			}

			Vec3d biTang = vNorm.Cross( vTang );
			vTang = biTang.Cross( vNorm );
			vTang.Normalize();

			// Store the calculated normal space
			mesh.mTransformedVertices[ v ].mNorm = vNorm;
			mesh.mTransformedVertices[ v ].mTang = vTang;
        
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
				orient.NormalizeAndInvert();
				Quat invOrient = orient;
            
				// decompose the normal
				Vec3d decomposedNormal = invOrient.RotatePoint( vNorm );
				Vec3d decomposedTangnt = invOrient.RotatePoint( vTang );
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
 MD5Model::GetTransformedNormals
 ====================================
 */
void MD5Model::GetTransformedNormals( Array< Vec3d > & normals ) {
	for ( int i = 0; i < mMeshes.Num(); ++i ) {
		const md5Mesh & mesh = mMeshes[ i ];

		for ( int j = 0; j < mesh.mTransformedVertices.Num(); ++j ) {
			const Vec3d & pos = mesh.mTransformedVertices[ j ].mPos;
			const Vec3d & norm = mesh.mTransformedVertices[ j ].mNorm;
			
			Vec3d pt2 = pos + 2.5f * norm;
			normals.Append( pos );
			normals.Append( pt2 );
		}
	}
}

/*
 ====================================
 MD5Model::GetTransformedTangnts
 ====================================
 */
void MD5Model::GetTransformedTangnts( Array< Vec3d > & tangnts ) {
	for ( int i = 0; i < mMeshes.Num(); ++i ) {
		const md5Mesh & mesh = mMeshes[ i ];

		for ( int j = 0; j < mesh.mTransformedVertices.Num(); ++j ) {
			const Vec3d & pos = mesh.mTransformedVertices[ j ].mPos;
			const Vec3d & tang = mesh.mTransformedVertices[ j ].mTang;

			Vec3d pt2 = pos + 2.5f * tang;
			tangnts.Append( pos );
			tangnts.Append( pt2 );
		}
	}
}


/*
 ====================================
 MD5Model::Draw
 ====================================
 */
void MD5Model::Draw() {
	for ( int i = 0; i < mMeshes.Num(); ++i ) {
		mMeshesVBO[ i ]->mVAO.Bind();
		glDrawElements( GL_TRIANGLES, mMeshes[ i ].mTriIndices.Num(), GL_UNSIGNED_SHORT, 0 );
		mMeshesVBO[ i ]->mVAO.UnBind();
	}
}

/*
 ====================================
 MD5Model::MakeVBO
 ====================================
 */
void MD5Model::MakeVBO() {
	const int stride = sizeof( md5Vert );
	md5Vert sampleVert;

	//
	//  Generate and Fill the vbos with Vertex and Index data
	//
	for ( int i = 0; i < mMeshes.Num(); ++i ) {
		const md5Mesh & mesh = mMeshes[ i ];
		md5MeshVBO * meshVBO = new md5MeshVBO;

		meshVBO->mVBOs[ 0 ].Generate( GL_ARRAY_BUFFER, mesh.mTransformedVertices.Num() * stride, mesh.mTransformedVertices.ToPtr(), GL_DYNAMIC_DRAW );
		meshVBO->mVBOs[ 1 ].Generate( GL_ELEMENT_ARRAY_BUFFER, mesh.mTriIndices.Num() * sizeof( unsigned short ), mesh.mTriIndices.ToPtr(), GL_STATIC_DRAW );
    
		//
		//  Generate and Fill the VAO
		//
		meshVBO->mVAO.Generate();
		meshVBO->mVAO.Bind();
    
		meshVBO->mVBOs[ 0 ].Bind();
		meshVBO->mVBOs[ 1 ].Bind();
    
		unsigned long offset = 0;

		// The position vertices
		glEnableVertexAttribArray( 0 );
		glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset );
		offset += sizeof( sampleVert.mPos );
    
		// The normal vertices
		glEnableVertexAttribArray( 1 );
		glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset );
		offset += sizeof( sampleVert.mNorm );
    
		// The tangent vertices
		glEnableVertexAttribArray( 2 );
		glVertexAttribPointer( 2, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset );
		offset += sizeof( sampleVert.mTang );
    
		// The st coordinates
		glEnableVertexAttribArray( 3 );
		glVertexAttribPointer( 3, 2, GL_FLOAT, GL_FALSE, stride, (const void *)offset );

		meshVBO->mVAO.UnBind();
		meshVBO->mVBOs[ 0 ].UnBind();
		meshVBO->mVBOs[ 1 ].UnBind();

		// Check for errors
		myglGetError();

		mMeshesVBO.Append( meshVBO );
	}
}

/*
 ====================================
 MD5Model::UpdateVBO
 ====================================
 */
void MD5Model::UpdateVBO() {
	// Fill the vbo with the vertex data
	for ( int i = 0; i < mMeshes.Num(); ++i ) {
		mMeshesVBO[ i ]->mVBOs[ 0 ].Update( mMeshes[ i ].mTransformedVertices.ToPtr() );
	}
}

//
//  ClothEntity.cpp
//
#include "ClothEntity.h"
#include "Graphics/Graphics.h"
#include "Graphics/Mesh.h"

static std::vector< vert_t > s_vertices;
static std::vector< int > s_indices;

static VertexBufferObject    s_vertexBuffer;
static VertexBufferObject    s_indexBuffer;
static VertexArrayObject     s_vao;

/*
====================================================
BuildFromCloth
====================================================
*/
bool BuildFromCloth( float * verts, const int width, const int height, const int stride ) {
	const int num = width * height;
	s_indices.reserve( num * 3 );
	s_vertices.reserve( num );

	for ( int i = 0; i < num; i++ ) {
		vert_t vert;
		memset( &vert, 0, sizeof( vert ) );

		int idx = i * stride / 4;

		vert.pos[ 0 ] = verts[ idx + 0 ];
		vert.pos[ 1 ] = verts[ idx + 1 ];
		vert.pos[ 2 ] = verts[ idx + 2 ];

		int x = i % width;
		int y = i / height;
		vert.st[ 1 ] = float( x ) / float( width - 1 );
		vert.st[ 0 ] = float( y ) / float( height - 1 );

		vert.norm[ 0 ] = 0;
		vert.norm[ 1 ] = 255;
		vert.norm[ 2 ] = 0;
		vert.norm[ 3 ] = 0;

		vert.tang[ 0 ] = 0;
		vert.tang[ 1 ] = 0;
		vert.tang[ 2 ] = 255;
		vert.tang[ 3 ] = 0;

		s_vertices.push_back( vert );
	}

	for ( int y = 0; y < height - 1; y++ ) {
		for ( int x = 0; x < width - 1; x++ ) {
			int x1 = x + 1;
			int y1 = y + 1;

			int idx00 = x	+ y * width;
			int idx10 = x1	+ y * width;
			int idx01 = x	+ y1 * width;
			int idx11 = x1	+ y1 * width;

			s_indices.push_back( idx00 );
			s_indices.push_back( idx10 );
			s_indices.push_back( idx11 );

			s_indices.push_back( idx00 );
			s_indices.push_back( idx11 );
			s_indices.push_back( idx01 );


			s_indices.push_back( idx00 );
			s_indices.push_back( idx11 );
			s_indices.push_back( idx10 );

			s_indices.push_back( idx00 );
			s_indices.push_back( idx01 );
			s_indices.push_back( idx11 );
		}
	}
	
	//
	// Make VBO
	//
	const int sizeIBO = s_indices.size() * sizeof( unsigned int );
	s_indexBuffer.Generate( GL_ELEMENT_ARRAY_BUFFER, sizeIBO, &s_indices[ 0 ], GL_STATIC_DRAW );

	const int stride2 = sizeof( vert_t );
	const int sizeVBO = s_vertices.size() * stride2;
	s_vertexBuffer.Generate( GL_ARRAY_BUFFER, sizeVBO, &s_vertices[ 0 ], GL_DYNAMIC_DRAW );

	s_vao.Generate();
	s_vao.Bind();
    
	s_vertexBuffer.Bind();
	s_indexBuffer.Bind();	

	unsigned long offset = 0;
	glEnableVertexAttribArray( 0 );
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, stride2, (const void *)offset );
	offset += sizeof( Vec3 );
    
	glEnableVertexAttribArray( 1 );
	glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, stride2, (const void *)offset );
	offset += sizeof( Vec2 );
    
	glEnableVertexAttribArray( 2 );
	glVertexAttribPointer( 2, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride2, (const void *)offset );
	offset += sizeof( unsigned char ) * 4;
    
	glEnableVertexAttribArray( 3 );
	glVertexAttribPointer( 3, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride2, (const void *)offset );
	offset += sizeof( unsigned char ) * 4;

	glEnableVertexAttribArray( 4 );
	glVertexAttribPointer( 4, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride2, (const void *)offset );
	offset += sizeof( unsigned char ) * 4;

	s_vao.UnBind();
	s_vertexBuffer.UnBind();
	s_indexBuffer.UnBind();

	return true;
}

/*
====================================================
UpdateClothVerts
====================================================
*/
void UpdateClothVerts( float * verts, const int width, const int height, const int stride ) {
	int num = width * height;

	int bufferSize;

	for ( int i = 0; i < num; i++ ) {
		vert_t & vert = s_vertices[ i ];
		memset( &vert, 0, sizeof( vert ) );

		int idx = i * stride / 4;

		vert.pos[ 0 ] = verts[ idx + 0 ];
		vert.pos[ 1 ] = verts[ idx + 1 ];
		vert.pos[ 2 ] = verts[ idx + 2 ];

		int x = i % width;
		int y = i / height;
		vert.st[ 1 ] = float( x ) / float( width - 1 );
		vert.st[ 0 ] = float( y ) / float( height - 1 );
	}

	s_vertexBuffer.Update( s_vertices.data() );
}



/*
========================================================================================================

ClothEntity

========================================================================================================
*/

/*
====================================================
ClothEntity::ClothEntity
====================================================
*/
ClothEntity::ClothEntity() : Entity() {
	Body body;
	//body.m_position = Vec3( 35, 2, 3 );
	body.m_position = Vec3( 20, 2, 9 );
	body.m_orientation = Quat( 0, 0, 0, 1 );
	//body.m_orientation = Quat( Vec3( 0, 0, 1 ), 3.1415f * 0.5f );
	body.m_linearVelocity.Zero();
	body.m_angularVelocity.Zero();
	body.m_invMass = 0.0f;
	body.m_elasticity = 0.0f;
	body.m_friction = 0.0f;

	body.m_owner = this;
	body.m_bodyContents = BC_GENERIC;
	body.m_collidesWith = 0;

	m_bodyid = g_physicsWorld->AllocateBody( body );

	m_bodyid.body->m_shape = new ShapeBox( &m_clothPhysics.m_bounds.mins, 2 );


	m_clothPhysics.Reset( body.m_position );
	int width = m_clothPhysics.m_width;
	int height = m_clothPhysics.m_height;
 	BuildFromCloth( &m_clothPhysics.m_particles[ 0 ].m_position.x, width, height, sizeof( ClothParticle ) );

	m_modelDraw = NULL;
}

/*
====================================================
ClothEntity::~ClothEntity
====================================================
*/
ClothEntity::~ClothEntity() {
	g_physicsWorld->FreeBody( m_bodyid.id );
}

/*
====================================================
ClothEntity::Update
====================================================
*/
void ClothEntity::Update( float dt_sec ) {
	m_clothPhysics.Update( dt_sec );

	// We also need to update the model
	int width = m_clothPhysics.m_width;
	int height = m_clothPhysics.m_height;
	UpdateClothVerts( &m_clothPhysics.m_particles[ 0 ].m_position.x, width, height, sizeof( ClothParticle ) );
}

/*
====================================================
ClothEntity::IsExpired
====================================================
*/
bool ClothEntity::IsExpired() const {
	return false;
}

/*
================================
DrawCloth
================================
*/
void DrawCloth() {
	// Bind the VAO
    s_vao.Bind();
    
    // Draw
	const int num = s_indices.size();
	glDrawElements( GL_TRIANGLES, num, GL_UNSIGNED_INT, 0 );

	// Unbind the VAO
	s_vao.UnBind();
}




//
//  NavFile.cpp
//
#include "BSP/Map.h"
#include "Math/Matrix.h"
#include "Math/Bounds.h"
#include "Graphics/Graphics.h"
#include "Graphics/TextureManager.h"
#include "Graphics/ShaderManager.h"
#include "NavMesh/NavMesh.h"
#include "Miscellaneous/Fileio.h"
#include <math.h>
#include <algorithm>
#include <stdlib.h>
#include <assert.h>
#include <vector>

// std::vector< vert_t > s_navmeshVerts;
// std::vector< int > s_navmeshIndices;
// std::vector< Bounds > s_navmeshBounds;
// 
// static VertexBufferObject s_navmeshVBO;
// static VertexArrayObject s_navmeshVAO;
// static VertexBufferObject s_navmeshIBO;
// 
// MatN g_navAdjacencyMatrix;

extern MatN g_navAdjacencyMatrix;
extern MatN g_navEdgeListMatrix;

extern std::vector< vert_t > s_navmeshVerts;
extern std::vector< int > s_navmeshIndices;
extern std::vector< Bounds > s_navmeshBounds;
extern std::vector< navEdge_t > s_navmeshEdges;

extern void BuildNavMeshRenderGeo();

/*
================================================================

NavFile

================================================================
*/

#define NAV_FILE_MAGIC 4829
#define NAV_FILE_VERSION 1

struct navHeader_t {
	int magic;
	int version;
	int matDim;		// dimension of the adjacency matrix
	int numVerts;
	int numIndices;
	int numBounds;
	int numEdges;
};

/*
================================
ReadNavFile
================================
*/
bool ReadNavFile() {
	unsigned int size = 0;
	unsigned char * data = NULL;
	bool didRead = GetFileData( "data/generated/navfile.navmesh", &data, size );
	if ( !didRead ) {
		return false;
	}

	unsigned char * data_ptr = data;

	navHeader_t header;
	memcpy( &header, data_ptr, sizeof( header ) );
	data_ptr += sizeof( header );

	if ( header.magic != NAV_FILE_MAGIC ) {
		free( data );
		return false;
	}
	if ( header.version != NAV_FILE_VERSION ) {
		free( data );
		return false;
	}

	g_navAdjacencyMatrix = MatN( header.matDim );
	for ( int i = 0; i < header.matDim; i++ ) {
		memcpy( g_navAdjacencyMatrix.rows[ i ].data, data_ptr, sizeof( float ) * header.matDim );
		data_ptr += sizeof( float ) * header.matDim;
	}
	g_navEdgeListMatrix = MatN( header.matDim );
	for ( int i = 0; i < header.matDim; i++ ) {
		memcpy( g_navEdgeListMatrix.rows[ i ].data, data_ptr, sizeof( float ) * header.matDim );
		data_ptr += sizeof( float ) * header.matDim;
	}

	vert_t vert;
	s_navmeshVerts.clear();
	for ( int i = 0; i < header.numVerts; i++ ) {
		memcpy( &vert, data_ptr, sizeof( vert_t ) );
		s_navmeshVerts.push_back( vert );
		data_ptr += sizeof( vert_t );
	}

	int idx;
	s_navmeshIndices.clear();
	for ( int i = 0; i < header.numIndices; i++ ) {
		memcpy( &idx, data_ptr, sizeof( int ) );
		s_navmeshIndices.push_back( idx );
		data_ptr += sizeof( int );
	}

	Bounds bounds;
	s_navmeshBounds.clear();
	for ( int i = 0; i < header.numBounds; i++ ) {
		memcpy( &bounds, data_ptr, sizeof( bounds ) );
		s_navmeshBounds.push_back( bounds );
		data_ptr += sizeof( bounds );
	}

	navEdge_t edge;
	s_navmeshEdges.clear();
	for ( int i = 0; i < header.numEdges; i++ ) {
		memcpy( &edge, data_ptr, sizeof( edge ) );
		s_navmeshEdges.push_back( edge );
		data_ptr += sizeof( edge );
	}

	BuildNavMeshRenderGeo();
	
	free( data );
	return true;
}

/*
================================
WriteNavFile
================================
*/
bool WriteNavFile() {
	if ( !OpenFileWriteStream( "data/generated/navfile.navmesh" ) ) {
		return false;
	}

	navHeader_t header;
	header.magic = NAV_FILE_MAGIC;
	header.version = NAV_FILE_VERSION;
	header.matDim = g_navAdjacencyMatrix.numDimensions;
	header.numVerts = s_navmeshVerts.size();
	header.numIndices = s_navmeshIndices.size();
	header.numBounds = s_navmeshBounds.size();
	header.numEdges = s_navmeshEdges.size();
	WriteFileStream( &header, sizeof( header ) );

	for ( int i = 0; i < header.matDim; i++ ) {
		WriteFileStream( g_navAdjacencyMatrix.rows[ i ].data, sizeof( float ) * header.matDim );
	}
	for ( int i = 0; i < header.matDim; i++ ) {
		WriteFileStream( g_navEdgeListMatrix.rows[ i ].data, sizeof( float ) * header.matDim );
	}	

	WriteFileStream( s_navmeshVerts.data(), sizeof( vert_t ) * s_navmeshVerts.size() );
	WriteFileStream( s_navmeshIndices.data(), sizeof( int ) * s_navmeshIndices.size() );
	WriteFileStream( s_navmeshBounds.data(), sizeof( Bounds ) * s_navmeshBounds.size() );
	WriteFileStream( s_navmeshEdges.data(), sizeof( navEdge_t ) * s_navmeshEdges.size() );

	CloseFileWriteStream();
	return true;
}


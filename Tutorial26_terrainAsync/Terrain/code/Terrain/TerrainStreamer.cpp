//
//  TerrainStream.cpp
//
#include "TerrainStreamer.h"
#include "TerrainPool.h"

#include "TerrainFile.h"
#include "Terrain.h"

#include "../Threading/Threads.h"
#include "../Threading/Mutex.h"
#include "../Threading/ThreadLocks.h"


#define MAX_STREAM_REQUETS 1024

static Mutex * s_streamerMutex = NULL;


static streamerCmd_t s_cmdQueue[ MAX_STREAM_REQUETS ];
static int s_low = 0;
static int s_high = 0;

static Thread s_streamerThread;

/*
================================
PushStreamerCmd
================================
*/
void PushStreamerCmd( streamerCmd_t cmd ) {
	ScopedLock lock( *s_streamerMutex );

	// TODO: We should probably check that this isn't a repeat cmd waiting to be processed

	s_cmdQueue[ s_high ] = cmd;
	s_high = ( s_high + 1 ) % MAX_STREAM_REQUETS;
}

/*
================================
PopStreamerCmd
================================
*/
streamerCmd_t * PopStreamerCmd() {
	if ( s_low == s_high ) {
		return NULL;
	}

	ScopedLock lock( *s_streamerMutex );
	
	streamerCmd_t * cmd = &s_cmdQueue[ s_low ];
	s_low = ( s_low + 1 ) % MAX_STREAM_REQUETS;
	return cmd;
}

extern poolEntry_t * GetPoolEntry( int id );
extern TerrainTileFile * GetTerrainTileFile( int tileX, int tileY );

/*
================================
TerrainStreamer
================================
*/
ThreadReturnType_t TerrainStreamer( ThreadInputType_t data ) {
	while ( 1 ) {
		// Sit and wait for commands to stream
		streamerCmd_t * cmd = PopStreamerCmd();
		if ( NULL != cmd ) {
			// Load the thing
			poolEntry_t * entry = GetPoolEntry( cmd->poolId );
			if ( !entry->isLoaded ) {
				TerrainTileFile * file = GetTerrainTileFile( cmd->tileX, cmd->tileY );
 				file->ReadTerrainIsland( cmd->x, cmd->y, cmd->depth, entry->terra.verts );
				entry->isLoaded = true;
			}
		}
	}

	return NULL;
}

/*
================================
InitStreamer
================================
*/
void InitStreamer() {
	s_streamerMutex = new Mutex;
	s_streamerThread.Create( TerrainStreamer, NULL );
}

/*
================================
EndStreamer
================================
*/
void EndStreamer() {
	s_streamerThread.Join();

	delete s_streamerMutex;
	s_streamerMutex = NULL;
}
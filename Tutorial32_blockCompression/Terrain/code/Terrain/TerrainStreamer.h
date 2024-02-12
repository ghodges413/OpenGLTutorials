//
//  TerrainStream.h
//
#pragma once

struct streamerCmd_t {
	int tileX;
	int tileY;
	int depth;
	int x;
	int y;

	int poolId;
	int mapId;
};

void PushStreamerCmd( streamerCmd_t cmd );
 
void InitStreamer();

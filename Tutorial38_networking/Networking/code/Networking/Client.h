//
//  Client.h
//
#pragma once
#include "Math/Vector.h"

/*
===============================
TestClient
===============================
*/
void TestClient();

void InitClientThread();
void EndClientThread();

void ClientSendViewPos( float theta, float phi, Vec3 pos );
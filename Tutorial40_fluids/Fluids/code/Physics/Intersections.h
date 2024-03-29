//
//	Intersections.h
//
#pragma once

class Body;
class Vec3;
class ShapeSphere;
struct contact_t;

bool Intersect( Body * bodyA, Body * bodyB, contact_t & contact );
bool Intersect( Body * bodyA, Body * bodyB, const float dt, contact_t & contact );

#pragma once

#include "geometry.h"

float Dot(Vec3f v0, Vec3f v1) 
{
	return v0.x * v1.x + v0.y * v1.y + v0.z * v1.z;
}

float Dot(Vec2f v0, Vec2f v1)
{
	return v0.x * v1.x + v0.y * v1.y;
}

Vec3f Cross(Vec3f v0,Vec3f v1)
{
	float x = v0.y * v1.z - v0.z * v1.y;
	float y = v0.z * v1.x - v0.x * v1.z;
	float z = v0.x * v1.y - v0.y * v1.x;

	return Vec3f(x, y, z);
}

float Max(float x, float y) 
{
	if (x >= y)
	{
		return x;
	}
	else return y;
}

float Min(float x, float y)
{
	if (x >= y)
	{
		return y;
	}
	else return x;
}

float ABS(float number) 
{
	if (number < 0) 
		return -number;
}
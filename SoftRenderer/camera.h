#pragma once

#include "geometry.h"

class Camera
{
private:
	Vector3f position;
	Vector3f rotation;
public:
	Camera();
	Camera(float x, float y, float z);
	void SetPosition(float x, float y, float z);
	Vector3f GetPosition();
};
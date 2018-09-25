#pragma once

#include "geometry.h"

class Camera
{
public:
	Vector4f position;
	Vector4f lookat;
	Vector4f up;
	Vector4f rotation;

	Matrix view;
public:
	Camera();
	Camera(float x, float y, float z);
	void SetPosition(float x, float y, float z);
	void SetCamera(Vector4f camera_lookat, Vector4f camera_up);
	float plane_camera_cos(Vector4f center, Vector4f normal);
	Vector4f GetPosition();
};
#include "camera.h"

Camera::Camera() 
{
	position.x = 0;
	position.y = 0;
	position.z = 0;
}

Camera::Camera(float x, float y, float z)
{
	position.x = x;
	position.y = y;
	position.z = z;
}

void Camera::SetPosition(float x, float y, float z)
{
	position.x = x;
	position.y = y;
	position.z = z;
}

Vector3f Camera::GetPosition()
{
	return position;
}
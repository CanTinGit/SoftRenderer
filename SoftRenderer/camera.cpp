#include "camera.h"

Camera::Camera()
{
	position.x = 0;
	position.y = 0;
	position.z = 0;
	position.w = 1.0f;
	view = Matrix::Identity(4);
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

Vector4f Camera::GetPosition()
{
	return position;
}

void Camera::SetCamera(Vector4f camera_lookat, Vector4f camera_up)
{
	lookat = camera_lookat;
	up = camera_up;
	Vector4f u, v, n;        //U,V,N
	n = lookat - position;
	n.normalize();
	u = up ^ n;
	u.normalize();
	v = n ^ u;
	v.normalize();

	view.m[0][0] = u.x;
	view.m[1][0] = u.y;
	view.m[2][0] = u.z;
	view.m[3][0] = -(position*u);

	view.m[0][1] = v.x;
	view.m[1][1] = v.y;
	view.m[2][1] = v.z;
	view.m[3][1] = -(position*v);

	view.m[0][2] = n.x;
	view.m[1][2] = n.y;
	view.m[2][2] = n.z;
	view.m[3][2] = -(position*n);

	view.m[0][3] = 0;
	view.m[1][3] = 0;
	view.m[2][3] = 0;
	view.m[3][3] = 1;

	//view.m[0][0] = u.x;
	//view.m[0][1] = u.y;
	//view.m[0][2] = u.z;
	//view.m[0][3] = -(position*u);

	//view.m[1][0] = v.x;
	//view.m[1][1] = v.y;
	//view.m[1][2] = v.z;
	//view.m[1][3] = -(position*v);

	//view.m[2][0] = n.x;
	//view.m[2][1] = n.y;
	//view.m[2][2] = n.z;
	//view.m[2][3] = -(position*n);

	//view.m[3][0] = 0;
	//view.m[3][1] = 0;
	//view.m[3][2] = 0;
	//view.m[3][3] = 1;
}

float Camera::plane_camera_cos(Vector4f center, Vector4f normal)
{
	Vector4f eye_dir;
	eye_dir = position - center;
	eye_dir.normalize();
	normal.normalize();
	return normal * eye_dir;
}
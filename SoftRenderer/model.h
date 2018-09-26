///////////////////////////////////////////////////////////////////////////////////////////////////
//refer:https://github.com/ssloy/tinyrenderer/blob/f6fecb7ad493264ecd15e230411bfb1cca539a12/model.h
///////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include <vector>
#include "geometry.h"


class Model {
public:
	Vector4f position;
	Vector4f rotation;
	std::vector<Vertex> vertices;
	std::vector<std::vector<int> > faces;
	std::vector<Vector4f> normals;
	std::vector<Vector2f> uvs;
	float max_radius;
	Vector3f dir;
public:
	Model(const char *filename);
	~Model();
	int nverts();
	int nfaces();
	Vertex vert(int i);
	std::vector<int> face(int idx);
	void SetPosition(float x, float y, float z);
	void SetRotation(float x, float y, float z, float theta);
	void UpdateWorldPosition();
	Vector4f Position();
	float Max_Radius();
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//refer:https://github.com/ssloy/tinyrenderer/blob/f6fecb7ad493264ecd15e230411bfb1cca539a12/model.h
///////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include <vector>
#include "geometry.h"

class Model {
private:
	Vector4f position;
	std::vector<Vector4f> verts_;
	std::vector<std::vector<int> > faces_;
	float max_radius;
	Vector3f dir;
public:
	Model(const char *filename);
	~Model();
	int nverts();
	int nfaces();
	Vector4f vert(int i);
	std::vector<int> face(int idx);
	void SetPosition(float x, float y, float z);
	Vector4f Position();
	float Max_Radius();
};

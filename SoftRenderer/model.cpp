///////////////////////////////////////////////////////////////////////////////////////////////////
//refer:https://github.com/ssloy/tinyrenderer/blob/f6fecb7ad493264ecd15e230411bfb1cca539a12/model.cpp
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"

Model::Model(const char *filename) : verts_(), faces_() {
	
	position.x = 0;
	position.y = 0;
	position.z = 0;

	dir = Vector3f(0, 0, 1);
	
	max_radius = 0;
	int radius = 0;

	std::ifstream in;
	in.open(filename, std::ifstream::in);
	if (in.fail()) return;
	std::string line;
	while (!in.eof()) {
		std::getline(in, line);
		std::istringstream iss(line.c_str());
		char trash;
		if (!line.compare(0, 2, "v ")) {
			iss >> trash;
			Vector3f v;
			for (int i = 0; i < 3; i++) iss >> v[i];
			verts_.push_back(v);
			radius = v.x * v.x + v.y*v.y + v.z*v.z;
			if (radius > max_radius)
			{
				max_radius = radius;
			}
		}
		else if (!line.compare(0, 2, "f ")) {
			std::vector<int> f;
			int itrash, idx;
			iss >> trash;
			while (iss >> idx >> trash >> itrash >> trash >> itrash) {
				idx--; // in wavefront obj all indices start at 1, not zero
				f.push_back(idx);
			}
			faces_.push_back(f);
		}
	}
	max_radius = std::sqrt(max_radius);
	std::cerr << "# v# " << verts_.size() << " f# " << faces_.size() << std::endl;
}

Model::~Model() {
}

int Model::nverts() {
	return (int)verts_.size();
}

int Model::nfaces() {
	return (int)faces_.size();
}

std::vector<int> Model::face(int idx) {
	return faces_[idx];
}

Vector3f Model::vert(int i) {
	return verts_[i];
}

void Model::SetPosition(float x, float y, float z)
{
	position.x = x;
	position.y = y;
	position.z = z;
}

Vector3f Model::Position()
{
	return position;
}

float Model::Max_Radius()
{
	return max_radius;
}
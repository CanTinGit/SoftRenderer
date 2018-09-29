///////////////////////////////////////////////////////////////////////////////////////////////////
//refer:https://github.com/ssloy/tinyrenderer/blob/f6fecb7ad493264ecd15e230411bfb1cca539a12/model.cpp
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"

Model::Model(const char *filename) : vertices(), faces() {
	
	position.x = 0;
	position.y = 0;
	position.z = 0;
	position.w = 1;

	rotation.x = 0;
	rotation.y = 0;
	rotation.z = 0;
	rotation.w = 0;

	dir = Vector4f(0, 0, 1,0);
	
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
			Vector4f v;
			for (int i = 0; i < 3; i++) iss >> v[i];
			v.z = -v.z;
			v.w = 1.0f;
			Vertex ver;
			ver.local = v;
			vertices.push_back(ver);
			radius = v.x * v.x + v.y*v.y + v.z*v.z;
			if (radius > max_radius)
			{
				max_radius = radius;
			}
		}
		else if (!line.compare(0, 3, "vt "))
		{
			char trash2;
			iss >> trash >> trash2;
			Vector2f uv;
			for (int i = 0; i < 2; i++) iss >> uv[i];
			uvs.push_back(uv);
		}
		else if (!line.compare(0, 3, "vn "))
		{
			char trash2;
			iss >> trash >> trash2;
			Vector4f normal;
			for (int i = 0; i < 3; i++) iss >> normal[i];
			normals.push_back(normal);
		}
		else if (!line.compare(0, 2, "f ")) {
			std::vector<int> f;
			int itrash, idx, idx_vt, idx_vn;
			iss >> trash;
			while (iss >> idx >> trash >> idx_vt >> trash >> idx_vn) 
			{
				idx--; // in wavefront obj all indices start at 1, not zero
				idx_vt--;
				idx_vn--;
				f.push_back(idx);
				vertices[idx].normal = normals[idx_vn];
				vertices[idx].u = uvs[idx_vt].x;
				vertices[idx].v = uvs[idx_vt].y;
			}
			faces.push_back(f);
		}
	}
	max_radius = std::sqrt(max_radius);
	std::cerr << "# v# " << vertices.size() << " f# " << faces.size() << std::endl;
}

Model::~Model() {
}

int Model::nverts() {
	return (int)vertices.size();
}

int Model::nfaces() {
	return (int)faces.size();
}

std::vector<int> Model::face(int idx) {
	return faces[idx];
}

Vertex Model::vert(int i) {
	return vertices[i];
}

void Model::SetPosition(float x, float y, float z)
{
	position.x = x;
	position.y = y;
	position.z = z;
}

void Model::SetRotation(float x, float y, float z, float theta)
{
	rotation.x = x;
	rotation.y = y;
	rotation.z = z;
	rotation.w = theta;
}


Vector4f Model::Position()
{
	return position;
}

float Model::Max_Radius()
{
	return max_radius;
}

void Model::UpdateWorldPosition()
{
	for(int i = 0; i<vertices.size();i++)
	{
		vertices[i].worldCoordinates;
	}
}

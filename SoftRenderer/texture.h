#pragma once
#include "geometry.h"
#include <opencv2/opencv.hpp>
#include <string>

using namespace std;
class Texture
{
public:
	cv::Mat buf;
	int width;
	int height;
	string name;
	void Init(int w, int h);
	void Load(const char *filename);
	Vector4i Map(float tu, float tv);
};

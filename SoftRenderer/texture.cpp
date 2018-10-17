#include "texture.h"

//////////////////////
////Œ∆¿Ì¿‡∫Ø ˝////////
//////////////////////
void Texture::Init(int w, int h)
{
	width = w;
	height = h;
	buf = cv::Mat(w, h, CV_32S);
}

void Texture::Load(const char* filename)
{
	buf = cv::imread(filename);
	width = buf.size().width;
	height = buf.size().height;
	if (buf.empty()) cout << "Load Texture Error" << endl;
}

Vector4i Texture::Map(float tu, float tv)
{
	Vector4i result;
	result.x = 0;
	result.y = 0;
	result.z = 0;
	result.w = 0;
	if (buf.empty()) return result;
	int u = (int)(tu*width) % width;
	int v = (int)(tv*width) % height;
	//if (u < 0 || v < 0)
	//	cout << tu << ' ' << tv << endl;
	u = u >= 0 ? u : -u;
	v = v >= 0 ? v : -v;

	cv::Vec3b tex_w = buf.at<cv::Vec3b>(v, u);
	result.x = tex_w[2];
	result.y = tex_w[1];
	result.z = tex_w[0];
	result.w = 0;
	//result = { tex_w[2],tex_w[1],tex_w[0],0 };
	return result;
}
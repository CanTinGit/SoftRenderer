#pragma once
#include "geometry.h"
#include "mymath.h"
#include "camera.h"
#include <string>
#include <opencv2/opencv.hpp>

using namespace std;

typedef unsigned char BYTE;
typedef unsigned int UINT32;

#define INTERP(x1,x2,t) ((x1) + ((x2) - (x1))*(t))
inline float CMID(float x, float min, float max) { return (x < min) ? min : ((x > max) ? max : x); }
inline Vector4f Vector_Interp(Vector4f p1, Vector4f p2,float t)
{
	Vector4f result;
	result.x = INTERP(p1.x, p2.x, t);
	result.y = INTERP(p1.y, p2.y, t);
	result.z = INTERP(p1.z, p2.z, t);
	result.w = 1.0f;
	return result;
}

typedef  struct { int v1, v2, v3; } Face;

class Transform
{
public:
	Matrix world; //世界坐标变换矩阵
	Matrix view;  //相机坐标变换矩阵
	Matrix projection; //投影坐标变换
	Matrix transform; //transform = projection * view * world;
	float width, height;  //屏幕大小

	void Update();
	void Init(int _width, int _height);
	void Apply(Vector4f &op, Vector4f &re);      //对矢量做投影变换
	void Apply(Vertex &op, Vertex &re);          //对顶点做投影变换
	void Apply(Vertex &op, Vertex &re, Vector2f uv);
	//void Apply(Vertex &op, Vertex &re);       //对顶点做投影变换
	void Homogenize(Vector4f &op, Vector4f &re);
	void Homogenize(Vertex &op, Vertex &re);
	void Set_Perspective(float fovy, float aspect, float near_z, float far_z);
};

class Texture
{
public:
	cv::Mat buf;
	int width;
	int height;
	string name;
	void Load(const char *filename);
	Vector4i Map(float tu, float tv);
};

//扫描线相关属性，便于插值
class ScanLineData
{
public:
	int currentY;
	float diffuse;
	float ndotla;
	float ndotlb;
	float ndotlc;
	float ndotld;

	Vector4f world_a;
	Vector4f world_b;
	Vector4f world_c;
	Vector4f world_d;

	Vector4f n;

	Vector4i leftColor, rightColor;

	float ua, ub, uc, ud;
	float va, vb, vc, vd;

	float rhwa, rhwb, rhwc, rhwd;
};

class Light
{
public:
	Vector4f position;
	Vector4i color;
	float intensity;

	Light();
	void SetPosition(float x, float y, float z);
	void SetColor(int r, int g, int b);
	void SetIntensity(float a);
	float LightCos(Vector4f point, Vector4f normal);
};

class Device
{
public:
	Transform transform;      //变换矩阵
	int width, height;
	UINT32 **framebuffer;     //像素缓存
	float **zbuffer;          //深度缓存
	UINT32 background;        //背景颜色
	Camera my_camera;         //相机
	Texture texture;
	Light diffuselight,ambientLight,speculaLight;
	float specularPower;

	Device(int w, int h, void *fb);
	~Device();

	void Clear(int mode);
	bool BackfaceCulling(Vertex pa_v, Vertex pb_v, Vertex pc_v, Vector4f normal);
	//bool BackfaceCulling(Vertex pa_v, Vertex pb_v, Vertex pc_v);

	void PutPixel(int x, int y, UINT32 &color);
	void PutPixel(int x, int y, float z, UINT32 &color);

	void DrawLine(Vector2i p1, Vector2i p2, UINT32 color);
	void DrawLine(Vector3i p1, Vector3i p2, UINT32 color);

	void ProcessScanLine(int curY, Vector4f &pa, Vector4f &pb, Vector4f &pc, Vector4f &pd, UINT32& color);
	void ProcessScanLine(ScanLineData scanline, Vector4f &pa, Vector4f &pb, Vector4f &pc, Vector4f &pd);
	void ProcessScanLineTexture(ScanLineData scanline, Vector4f &pa, Vector4f &pb, Vector4f &pc, Vector4f &pd,Texture &tex);

	//扫描线法填充
	void DrawTriangleFrame(Vertex A, Vertex B, Vertex C, UINT32 color);
	void DrawTriangleFlat(Vertex A, Vertex B, Vertex C, UINT32 color);
	void DrawTriangleFlat(Vertex A, Vertex B, Vertex C);
	void DrawTriangleTexture(Vertex A, Vertex B, Vertex C);

	void Render(Model &model, int op);
};

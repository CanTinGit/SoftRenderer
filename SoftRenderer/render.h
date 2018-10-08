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
inline int CMID(int x, int min, int max) { return (x < min) ? min : ((x > max) ? max : x); }

typedef  struct { int v1, v2, v3; } Face;

class Transform
{
public:
	Matrix world; //��������任����
	Matrix view;  //�������任����
	Matrix projection; //ͶӰ����任
	Matrix transform; //transform = projection * view * world;
	float width, height;  //��Ļ��С

	void Update();
	void Init(int _width, int _height);
	void Apply(Vector4f &op, Vector4f &re);      //��ʸ����ͶӰ�任
	void Apply(Vertex &op, Vertex &re);          //�Զ�����ͶӰ�任
	void Apply(Vertex &op, Vertex &re, Vector2f uv);
	//void Apply(Vertex &op, Vertex &re);       //�Զ�����ͶӰ�任
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

//ɨ����������ԣ����ڲ�ֵ
class ScanLineData
{
public:
	int currentY;
	float ndotla;
	float ndotlb;
	float ndotlc;
	float ndotld;

	Vector4i leftColor, rightColor;

	float ua, ub, uc, ud;
	float va, vb, vc, vd;

	float rhwa, rhwb, rhwc, rhwd;
};

class Device
{
public:
	Transform transform;      //�任����
	int width, height;
	UINT32 **framebuffer;     //���ػ���
	float **zbuffer;          //��Ȼ���
	UINT32 background;        //������ɫ
	Camera my_camera;         //���
	Texture texture;

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

	//ɨ���߷����
	void DrawTriangleFrame(Vertex A, Vertex B, Vertex C, UINT32 color);
	void DrawTriangleFlat(Vertex A, Vertex B, Vertex C, UINT32 color);
	void DrawTriangleFlat(Vertex A, Vertex B, Vertex C);
	void DrawTriangleTexture(Vertex A, Vertex B, Vertex C);

	void Render(Model &model, int op);
};

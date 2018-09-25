#pragma once
#include "geometry.h"
#include "mymath.h"
#include "camera.h"
#include <string>

using namespace std;

typedef unsigned char BYTE;
typedef unsigned int UINT32;

#define INTERP(x1,x2,t) ((x1) + ((x2) - (x1))*(t))
inline int CMID(int x, int min, int max) { return (x < min) ? min : ((x > max) ? max : x); }

class Color
{
public:
	union 
	{
		BYTE argb[4]; //bgra С�˴洢
		UINT32 uint32;
	};
	void Set(UINT32 x, float s);
	inline Color operator*(const Color &color) {
		Color result; result.argb[0] = argb[0] * color.argb[0]; result.argb[1] = argb[1] * color.argb[1]; 
		result.argb[2] = argb[2] * color.argb[2]; return result;
	}
};

class Vertex
{
public:
	Vector4f normal;                 //������
	Vector4f coordinates;			 //ͶӰ�������
	Vector4f worldCoordinates;		 //ԭ��������
};

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
	//void Apply(Vertex &op, Vertex &re);       //�Զ�����ͶӰ�任
	void Homogenize(Vector4f &op, Vector4f &re);
	void Set_Perspective(float fovy, float aspect, float near_z, float far_z);

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

	Device(int w, int h, void *fb);
	~Device();

	void Clear(int mode);
	bool BackfaceCulling(Vector4f pa_v, Vector4f pb_v, Vector4f pc_v);
	//bool BackfaceCulling(Vertex pa_v, Vertex pb_v, Vertex pc_v);

	void PutPixel(int x, int y, UINT32 &color);
	void PutPixel(int x, int y, int z, UINT32 &color);

	void DrawLine(Vector2i p1, Vector2i p2, UINT32 color);
	void DrawLine(Vector3i p1, Vector3i p2, UINT32 color);

	//ɨ���߷����
	void DrawTriangle(Vector3i A, Vector3i B, Vector3i C, UINT32 color);


	void Render(Model &model, int op);
};
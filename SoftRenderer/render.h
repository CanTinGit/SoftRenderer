#pragma once
#include "geometry.h"
#include "model.h"
#include "camera.h"

using namespace std;

typedef unsigned char BYTE;
typedef unsigned int UINT32;

#define INTERP(x1,x2,t) ((x1) + ((x2) - (x1))*(t))
inline int CMID(int x, int min, int max) { return (x < min) ? min : ((x > max) ? max : x); }
inline float CMIDF(float x, float min, float max) { return (x < min) ? min : ((x > max) ? max : x); }
inline Vector4f Vector_Interp(Vector4f p1, Vector4f p2, float t)
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
	Matrix world; //��������任����
	Matrix view;  //�������任����
	Matrix projection; //ͶӰ����任
	Matrix ortho;      //����ͶӰ�任 - ����shadow map
	Matrix transform; //transform = world*view*projection;
	Matrix inverTransform;
	Matrix worldToProjection; // worldToProjection = view *projection
	float width, height;  //��Ļ��С

	void Update();
	void LightUpdate();
	void Init(int _width, int _height);
	void Apply(Vector4f &op, Vector4f &re);      //��ʸ����ͶӰ�任
	void Apply(Vertex &op, Vertex &re);          //�Զ�����ͶӰ�任
	void Apply(Vertex &op, Vertex &re, Vector2f uv);
	//void Apply(Vertex &op, Vertex &re);       //�Զ�����ͶӰ�任
	void Homogenize(Vector4f &op, Vector4f &re);
	void Homogenize(Vertex &op, Vertex &re);
	void ShadowHomogenize(Vertex &op, Vertex &re, int w, int h);
	void Set_Perspective(float fovy, float aspect, float near_z, float far_z);
	void Set_Ortho(float w, float h, float near_z, float far_z);

	void ScreenToWorld(Vertex &re, float rhw);
};

//ɨ����������ԣ����ڲ�ֵ
class ScanLineData
{
public:
	int currentY;
	float diffuse;

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
	Vector4f lightDir;

	Light();
	void SetPosition(float x, float y, float z);
	void SetColor(int r, int g, int b);
	void SetIntensity(float a);
	float DiffuseLightCos(Vector4f normal);
	void SetDiffuseLightDir(float x, float y, float z);
};

class Device
{
public:
	Transform transform, lightTransform;      //�任����
	int width, height;
	int shadowWidth, shadowHeight;
	UINT32 **framebuffer;     //���ػ���
	float **zbuffer;          //��Ȼ���
	UINT32 background;        //������ɫ
	Camera my_camera;         //���
	Light diffuselight, ambientLight, speculaLight;
	float specularPower;
	std::vector<std::vector<float>> shadowDepthbuffer;
	Device(int w, int h, void *fb, int sw, int sh);
	~Device();

	void Clear(int mode);
	void ClearShadowBuf();
	bool BackfaceCulling(Vertex pa_v, Vertex pb_v, Vertex pc_v, Vector4f normal);
	Vector3f PointInLightSpace(Vector4f worldCoord);
	//int Check_CVV(Vertex)

	void PutPixel(int x, int y, UINT32 &color);
	void PutPixel(int x, int y, float z, UINT32 &color);

	void DrawLine(Vector2i p1, Vector2i p2, UINT32 color);
	void DrawLine(Vector3i p1, Vector3i p2, UINT32 color);

	void ProcessScanLine(int curY, Vector4f &pa, Vector4f &pb, Vector4f &pc, Vector4f &pd, UINT32& color);
	void ProcessScanLine(ScanLineData scanline, Vector4f &pa, Vector4f &pb, Vector4f &pc, Vector4f &pd);
	void ProcessScanLineTexture(ScanLineData scanline, Vector4f &pa, Vector4f &pb, Vector4f &pc, Vector4f &pd, Texture &tex);
	void ProcessScanLineToTexture(ScanLineData scanline, Vector4f &pa, Vector4f &pb, Vector4f &pc, Vector4f &pd);

	//ɨ���߷����
	void DrawTriangleFrame(Vertex A, Vertex B, Vertex C, UINT32 color);
	void DrawTriangleFlat(Vertex A, Vertex B, Vertex C, UINT32 color);
	void DrawTriangleFlat(Vertex A, Vertex B, Vertex C);
	void DrawTriangleTexture(Vertex A, Vertex B, Vertex C,Texture texture);
	void DrawTriangleToTexture(Vertex A, Vertex B, Vertex C);

	void RenderToShadowTexture(vector<Model> models);
	void Render(vector<Model> models, int op);
};

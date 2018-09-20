#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "mymath.h"
#include <algorithm>

using namespace std;
static const int width = 800;
static const int height = 800;
static const int depth = 255;
const TGAColor white = TGAColor(255, 255, 255, 255);
Vector3f light(0, 0, -1);
Vector3f camera(1, 1, 3);
Vector3f center(0, 0, 0);

// x,y : 视矩形左下角的坐标
// width,height : 视矩形的宽度和高度
//Matrix ViewPortMatrix(int x, int y, int width, int height, int depth)
//{
//	Matrix m = Matrix::Identity(4);
//	m[0][3] = x + width / 2.0f;
//	m[1][3] = y + height / 2.0f;
//	m[2][3] = depth / 2.0f;
//
//	m[0][0] = width / 2.0f;
//	m[1][1] = height / 2.0f;
//	m[2][2] = depth / 2.0f;
//	return m;
//}

Matrix ViewPortMatrix(int x, int y, int w, int h) {
	Matrix m = Matrix::Identity(4);
	m[0][3] = x + w / 2.f;
	m[1][3] = y + h / 2.f;
	m[2][3] = depth / 2.f;

	m[0][0] = w / 2.f;
	m[1][1] = h / 2.f;
	m[2][2] = depth / 2.f;
	return m;
}

//Matrix LookAt(Vector3f cameraPosition, Vector3f center, Vector3f up)
//{
//	Vector3f z = (cameraPosition - center).normalize();
//	Vector3f x = (up^z).normalize();
//	Vector3f y = (z^x).normalize();
//	Matrix rotateMatrix = Matrix::Identity(4);     //坐标轴旋转矩阵
//	Matrix transposeMatrix = Matrix::Identity(4);  //原点平移矩阵
//	Matrix result = Matrix::Identity(4);
//	for (int i = 0; i < 3; i++)
//	{
//		rotateMatrix[0][i] = x[i];
//		rotateMatrix[1][i] = y[i];
//		rotateMatrix[2][i] = z[i];
//		transposeMatrix[i][3] = -center[i];
//	}
//	result = rotateMatrix * transposeMatrix;
//	return result;
//}

Matrix LookAt(Vector3f eye, Vector3f center, Vector3f up) {
	Vector3f z = (eye - center).normalize();
	Vector3f x = (up^z).normalize();
	Vector3f y = (z^x).normalize();
	Matrix res = Matrix::Identity(4);
	for (int i = 0; i < 3; i++) {
		res[0][i] = x[i];
		res[1][i] = y[i];
		res[2][i] = z[i];
		res[i][3] = -center[i];
	}
	return res;
}


void Line(Vector2f p1, Vector2f p2, TGAImage &image, TGAColor color)
{
	int x0 = p1.x;
	int y0 = p1.y;
	int x1 = p2.x;
	int y1 = p2.y;
	bool steep = false;

	if (abs(x1-x0)<abs(y1-y0))
	{
		swap(x0, y0);
		swap(x1, y1);
		steep = true;
	}

	if (x0>x1)
	{
		swap(x0, x1);
		swap(y0, y1);
	}

	int dx = x1 - x0;
	int dy = y1 - y0;
	int e = 0;
	int de = abs(dy) * 2;
	int y = y0;
	for (int x = x0; x <= x1; x++)
	{
		if (steep)
		{
			image.set(y, x, color);
		}
		else
		{
			image.set(x, y, color);
		}

		e += de;
		if (e> dx)
		{
			y += (y1 > y0 ? 1 : -1);
			e -= dx*2;
		}
	}
}

//利用重心法检测是否在某个图形范围内
Vector3f Barycentric(Vector3f A, Vector3f B, Vector3f C, Vector3f P)
{
	Vector3f v0;
	Vector3f v1;

	v0.x = C.x - A.x;
	v0.y = B.x - A.x;
	v0.z = A.x - P.x;
	v1.x = C.y - A.y;
	v1.y = B.y - A.y;
	v1.z = A.y - P.y;

	Vector3f u = v0 ^ v1;
	if (std::abs(u.z) > 1e-2)
		return Vector3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
	return Vector3f(-1, 1, 1); 
}
//
//void triangle(Vector2i t0, Vector2i t1, Vector2i t2, TGAImage &image, TGAColor color) {
//	// sort the vertices, t0, t1, t2 lower−to−upper (bubblesort yay!) 
//	if (t0.y > t1.y) std::swap(t0, t1);
//	if (t0.y > t2.y) std::swap(t0, t2);
//	if (t1.y > t2.y) std::swap(t1, t2);
//	int total_height = t2.y - t0.y;
//	for (int y = t0.y; y <= t1.y; y++) {
//		int segment_height = t1.y - t0.y + 1;
//		float alpha = (float)(y - t0.y) / total_height;
//		float beta = (float)(y - t0.y) / segment_height; // be careful with divisions by zero 
//		Vector2i A = t0 + (t2 - t0)*alpha;
//		Vector2i B = t0 + (t1 - t0)*beta;
//		if (A.x > B.x) std::swap(A, B);
//		for (int j = A.x; j <= B.x; j++) {
//			image.set(j, y, color); // attention, due to int casts t0.y+i != A.y 
//		}
//	}
//	for (int y = t1.y; y <= t2.y; y++) {
//		int segment_height = t2.y - t1.y + 1;
//		float alpha = (float)(y - t0.y) / total_height;
//		float beta = (float)(y - t1.y) / segment_height; // be careful with divisions by zero 
//		Vector2i A = t0 + (t2 - t0)*alpha;
//		Vector2i B = t1 + (t2 - t1)*beta;
//		if (A.x > B.x) std::swap(A, B);
//		for (int j = A.x; j <= B.x; j++) {
//			image.set(j, y, color); // attention, due to int casts t0.y+i != A.y 
//		}
//	}
//}

//扫线法参考
//void Atriangle(Vector3i t0, Vector3i t1, Vector3i t2, TGAImage &image, float intensity, int *zbuffer) {
//	if (t0.y == t1.y && t0.y == t2.y) return; // i dont care about degenerate triangles
//	if (t0.y > t1.y) { std::swap(t0, t1);  }
//	if (t0.y > t2.y) { std::swap(t0, t2);  }
//	if (t1.y > t2.y) { std::swap(t1, t2);}
//
//	int total_height = t2.y - t0.y;
//	for (int i = 0; i < total_height; i++) {
//		bool second_half = i > t1.y - t0.y || t1.y == t0.y;
//		int segment_height = second_half ? t2.y - t1.y : t1.y - t0.y;
//		float alpha = (float)i / total_height;
//		float beta = (float)(i - (second_half ? t1.y - t0.y : 0)) / segment_height; //(float)(i - (second_half ? t1.y - t0.y : 0)) / segment_height; // be careful: with above conditions no division by zero here
//		Vector3i A = t0 + Vector3f(t2 - t0)*alpha;
//		Vector3i B = second_half ? t1 + Vector3f(t2 - t1)*beta : t0 + Vector3f(t1 - t0)*beta;
//		if (A.x > B.x) { std::swap(A, B);  }
//		for (int j = A.x; j <= B.x; j++) {
//			float phi = B.x == A.x ? 1. : (float)(j - A.x) / (float)(B.x - A.x);
//			Vector3i   P = Vector3f(A) + Vector3f(B - A)*phi;
//			int idx = P.x + P.y*width;
//			if (zbuffer[idx] < P.z) {
//				zbuffer[idx] = P.z;
//				image.set(P.x, P.y, TGAColor(255*intensity, 255*intensity, 255*intensity,255));
//			}
//		}
//	}
//}

void triangle(Vector3i A, Vector3i B, Vector3i C, int *zbuffer, TGAImage&image, TGAColor color)
{
	if (A.y == B.y && B.y == C.y)
		return;
	if (A.y > B.y) std::swap(A, B);
	if (A.y > C.y) std::swap(A, C);
	if (B.y > C.y) std::swap(B, C);
	int total_height = C.y - A.y;

	for (int i = 0; i < total_height; i++)
	{
		bool second_half = i > B.y - A.y || B.y == A.y;
		int segment_height = second_half ? C.y - B.y : B.y - A.y;
		float alpha = (float)i / total_height;
		float beta = (float)(i - (second_half ? B.y - A.y : 0)) / segment_height; //second_half ? (float)(i - B.y) / segment_height : (float)(i - A.y) / segment_height;
		Vector3i X = A + Vector3f(C - A) *alpha;
		Vector3i Y = second_half ? B + Vector3f(C - B)*beta : A + Vector3f(B - A)*beta;
		if (X.x > Y.x) swap(X, Y);
		for (int x = X.x; x <= Y.x; x++)
		{
			float phi = Y.x == X.x ? 1.0f : (float)(x - X.x) / (float)(Y.x - X.x);
			Vector3i P = Vector3f(X) + Vector3f(Y - X)*phi;
			int idx = P.x + P.y*width;
			if (zbuffer[idx] < P.z)
			{
				zbuffer[idx] = P.z;
				image.set(P.x, P.y, color);
			}
		}
	}
}


//重心法光栅化三角形,目前有bug,zbuffer的计算不正确;
//void Triangle(Vector3f pointA, Vector3f pointB, Vector3f pointC, float* zbuffer,TGAImage &image, TGAColor color)
//{
//
//	Vector2f bboxmin(image.get_width(), image.get_height());
//	Vector2f bboxmax(0, 0);
//	Vector2f clamp(image.get_width()-1, image.get_height()-1);
//	
//	bboxmin.x = std::max(0.0f, std::min(bboxmin.x, pointA.x));
//	bboxmin.y = std::max(0.0f,std::min(bboxmin.y, pointA.y));
//	bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, pointA.x));
//	bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, pointA.y));
//
//	bboxmin.x = std::max(0.0f, std::min(bboxmin.x, pointB.x));
//	bboxmin.y = std::max(0.0f, std::min(bboxmin.y, pointB.y));
//	bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, pointB.x));
//	bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, pointB.y));
//
//	bboxmin.x = std::max(0.0f, std::min(bboxmin.x, pointC.x));
//	bboxmin.y = std::max(0.0f, std::min(bboxmin.y, pointC.y));
//	bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, pointC.x));
//	bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, pointC.y));
//
//	Vector3f P;
//	for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++)
//	{
//		for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++)
//		{
//			Vector3f bc_screen = Barycentric(pointA, pointB, pointC, P);
//			if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) continue;
//			P.z = pointA.z * bc_screen.x + pointB.z * bc_screen.y + pointC.z * bc_screen.z;
//			int idx = int(P.x + P.y*width);
//			if (P.z > zbuffer[idx])
//			{
//				image.set(P.x, P.y, color);
//				zbuffer[idx] = P.z;
//			}
//		}
//	}
//}


Vector3f WorldToScreen(Vector3f v)
{
	return Vector3f(int((v.x + 1.)*width / 2. + .5), int((v.y + 1.)*height / 2. + .5), v.z);
}

void main() 
{
	Model *model = new Model("obj/african_head.obj");
	TGAImage image(width, height, TGAImage::RGB);	

	//float zbuffer[800][800];      
	int* zbuffer = new int[width*height];
	//Initialize Zbuffer
	for (int i = 0; i < width*height; i++)
		zbuffer[i] = -std::numeric_limits<float>::max();

	//triangle({ 0,0,0 }, { 100,100,0 }, { 500,300,0 },zbuffer,image,white);

	//image.flip_vertically();
	//image.write_tga_file("output/Triangle.tga");
	Matrix modelView = LookAt(camera, center, Vector3f(0, 1, 0));
	Matrix projectionMatrix = Matrix::Identity(4);
	Matrix viewPort = ViewPortMatrix(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
	projectionMatrix[3][2] = -1.0f / (camera - center).norm();

	Matrix z = (viewPort * projectionMatrix*modelView);

	for (int i = 0; i < model->nfaces(); i++)
	{
		vector<int> face = model->face(i);
		Vector3i screen_coords[3];
		Vector3f world_coords[3];
		for (int j = 0; j < 3; j++)
		{
			Vector3f v = model->vert(face[j]);
			screen_coords[j] = MatrixToVector(viewPort*projectionMatrix*modelView*VectorToMatrix(v));//Vector3i(int((v.x + 1.)*width / 2.), int((v.y + 1.)*height / 2.),v.z);
			world_coords[j] = v;
		}

		Vector3f v0 = world_coords[2] - world_coords[0];
		Vector3f v1 = world_coords[1] - world_coords[0];

		Vector3f n = v0 ^ v1;

		float lightDiffuse = n.normalize()*light;

		if (lightDiffuse > 0) 
		{
			TGAColor lightColor(lightDiffuse * 255, lightDiffuse * 255, lightDiffuse * 255, 255);
			triangle(screen_coords[0], screen_coords[1], screen_coords[2], zbuffer, image, lightColor);
			//Atriangle(screen_coords[0], screen_coords[1], screen_coords[2], image,lightDiffuse,zbuffer);
		}
		
	}

	image.flip_vertically();
	image.write_tga_file("output/InFrontOfHead1.tga");
	TGAImage zbimage(width, height, TGAImage::GRAYSCALE);
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			zbimage.set(i, j, TGAColor(zbuffer[i + j * width]));
		}
	}
	zbimage.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	zbimage.write_tga_file("output/zbufferInFrontOfHead1.tga");

	delete model;
	delete[] zbuffer;
}
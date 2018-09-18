#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "mymath.h"
#include <algorithm>

using namespace std;
static const int width = 800;
static const int height = 800;
void Line(Vec2f p1, Vec2f p2, TGAImage &image, TGAColor color)
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
Vec3f Barycentric(Vec3f A, Vec3f B, Vec3f C, Vec3f P)
{
	Vec3f v0;
	Vec3f v1;

	v0.x = C.x - A.x;
	v0.y = B.x - A.x;
	v0.z = A.x - P.x;
	v1.x = C.y - A.y;
	v1.y = B.y - A.y;
	v1.z = A.y - P.y;

	Vec3f u = Cross(v0, v1);
	if (std::abs(u.z) > 1e-2)
		return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
	return Vec3f(-1, 1, 1); 
}


void Triangle(Vec3f pointA, Vec3f pointB, Vec3f pointC, float* zbuffer,TGAImage &image, TGAColor color)
{

	Vec2f bboxmin(image.get_width(), image.get_height());
	Vec2f bboxmax(0, 0);
	Vec2f clamp(image.get_width()-1, image.get_height()-1);
	
	bboxmin.x = Max(0.0f, Min(bboxmin.x, pointA.x));
	bboxmin.y = Max(0.0f,Min(bboxmin.y, pointA.y));
	bboxmax.x = Min(clamp.x, Max(bboxmax.x, pointA.x));
	bboxmax.y = Min(clamp.y, Max(bboxmax.y, pointA.y));

	bboxmin.x = Max(0.0f, Min(bboxmin.x, pointB.x));
	bboxmin.y = Max(0.0f, Min(bboxmin.y, pointB.y));
	bboxmax.x = Min(clamp.x, Max(bboxmax.x, pointB.x));
	bboxmax.y = Min(clamp.y, Max(bboxmax.y, pointB.y));

	bboxmin.x = Max(0.0f, Min(bboxmin.x, pointC.x));
	bboxmin.y = Max(0.0f, Min(bboxmin.y, pointC.y));
	bboxmax.x = Min(clamp.x, Max(bboxmax.x, pointC.x));
	bboxmax.y = Min(clamp.y, Max(bboxmax.y, pointC.y));

	Vec3f P;
	for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++)
	{
		for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++)
		{
			Vec3f bc_screen = Barycentric(pointA, pointB, pointC, P);
			if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) continue;
			P.z = pointA.z * bc_screen.x + pointB.z * bc_screen.y + pointC.z * bc_screen.z;
			
			if (P.z > zbuffer[int(P.x + P.y*width)])
			{
				image.set(P.x, P.y, color);
				zbuffer[int(P.x + P.y*width)] = P.z;
			}
		}
	}
}


void main() 
{
	Vec3f light(0, 0, -1);
	Model *model = new Model("obj/african_head.obj");
	const TGAColor white = TGAColor(255, 255, 255, 255);
	TGAImage image(width, height, TGAImage::RGB);	
	//float zbuffer[800][800];      数组过大导致堆栈溢出
	float* zbuffer = new float[width*height];
	//Initialize Zbuffer
	for (int i = 0; i < width*height; i++)
		zbuffer[i] = -std::numeric_limits<float>::max();

	for (int i = 0; i < model->nfaces(); i++)
	{
		vector<int> face = model->face(i);
		Vec3f screen_coords[3];
		Vec3f world_coords[3];
		for (int j = 0; j < 3; j++)
		{
			Vec3f v = model->vert(face[j]);
			screen_coords[j] = Vec3f(int((v.x + 1)*width / 2.0f), int((v.y + 1)*height / 2.0f),v.z);
			world_coords[j] = v;
		}

		Vec3f v0 = world_coords[2] - world_coords[0];
		Vec3f v1 = world_coords[1] - world_coords[0];

		Vec3f n = Cross(v0, v1);

		float lightDiffuse = Dot(n.normalize(),light);

		if (lightDiffuse > 0) 
		{
			TGAColor lightColor(lightDiffuse * 255, lightDiffuse * 255, lightDiffuse * 255, 255);
			Triangle(screen_coords[0], screen_coords[1], screen_coords[2],zbuffer, image, lightColor);
		}
		
	}

	image.flip_vertically();
	image.write_tga_file("output/ZBufferHead.tga");
}
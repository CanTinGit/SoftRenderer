#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "mymath.h"

using namespace std;

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
bool isInTriangle(Vec2f pointA, Vec2f pointB, Vec2f pointC, Vec2f point)
{
	Vec2f v0 = pointC - pointA;
	Vec2f v1 = pointB - pointA;
	Vec2f v2 = point - pointA;

	float u = (Dot(v1, v1)*Dot(v2, v0) - Dot(v1, v0)*Dot(v2, v1)) / (Dot(v0, v0)*Dot(v1, v1) - Dot(v0, v1)*Dot(v1, v0));
	float v = (Dot(v0, v0)*Dot(v2, v1) - Dot(v0, v1)*Dot(v2, v0)) / (Dot(v0, v0)*Dot(v1, v1) - Dot(v0, v1)*Dot(v1, v0));

	if (u<0 || u>1 || v<0 || v>1) //The point is outside of Triangle
	{
		return false;
	}

	return u + v <= 1;
}

void Triangle(Vec2f pointA, Vec2f pointB, Vec2f pointC, TGAImage &image, TGAColor color)
{
	Vec2f bboxmin(image.get_width() - 1, image.get_height() - 1);
	Vec2f bboxmax(0, 0);
	Vec2f clamp(image.get_width() - 1, image.get_height() - 1);
	
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

	Vec2f P;
	for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++)
	{
		for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++)
		{
			if (isInTriangle(pointA,pointB,pointC,P))
			{
				image.set(P.x, P.y, color);
			}
		}
	}
}


void main() 
{
	const float width = 1000;
	const float height = 1000;
	Vec3f light(0, 0, -1);
	Model *model = new Model("obj/african_head.obj");
	const TGAColor white = TGAColor(255, 255, 255, 255);
	TGAImage image(width, height, TGAImage::RGB);	

	//Triangle({ 0,0 }, { 170,500 }, { 300,300 }, image, white);

	for (int i = 0; i < model->nfaces(); i++)
	{
		vector<int> face = model->face(i);
		Vec2f screen_coords[3];
		Vec3f world_coords[3];
		for (int j = 0; j < 3; j++)
		{
			Vec3f v = model->vert(face[j]);
			screen_coords[j] = Vec2f((v.x + 1)*width / 2, (v.y + 1)*height / 2);
			world_coords[j] = v;
		}
		Vec3f v0 = world_coords[2] - world_coords[0];
		Vec3f v1 = world_coords[1] - world_coords[0];

		Vec3f n = Cross(v0, v1);

		float lightDiffuse = Dot(n.normalize(), light);

		TGAColor lightColor(lightDiffuse * 255, lightDiffuse * 255, lightDiffuse * 255, 255);
		Triangle(screen_coords[0], screen_coords[1], screen_coords[2], image, lightColor);
	}

	//Interpolation({ 10,10 }, { 70,70 }, image, white);
	image.flip_vertically();
	image.write_tga_file("output/LightedHead.tga");
}
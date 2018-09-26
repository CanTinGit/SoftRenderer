#include "render.h"

//矩阵更新计算：transform = projection * view * world
void Transform::Update()
{
	static Matrix m;
	m = Matrix::Identity(4);
	m = view * world;
	transform = projection * m;
}

//初始化
void Transform::Init(int _width, int _height)
{
	world = Matrix::Identity(4);
	view = Matrix::Identity(4);
	width = (float)_width;
	height = (float)_height;
}

//将op进行投影转换，z留作深度测试
void Transform::Apply(Vector4f &op, Vector4f &re)
{
	re = transform * op;
}

void Transform::Apply(Vertex &op, Vertex &re)
{
	re.coordinates = transform * op.local;
	re.worldCoordinates = world * op.local;
	re.normal = world * op.normal;
	re.u = op.u;
	re.v = op.v;
}

//归一化得到屏幕坐标
void Transform::Homogenize(Vector4f &op, Vector4f &re)
{
	float  rhw = 1.0f / op.w;
	re.x = (op.x *rhw + 1.0f) * width *0.5f;
	re.y = (1.0f - op.y * rhw) *height *0.5f;
	re.z = op.z *rhw;
	re.w = 1.0f;
}

//根据fov等参数设置透视矩阵, aspect—宽高比，near_z—近裁面到相机距离，far_z—远裁面到相机距离
void Transform::Set_Perspective(float fovy, float aspect, float near_z, float far_z)
{
	float fax = 1.0f / (float)tan(fovy*0.5f);
	projection = Matrix::ZeroMatrix(4);
	projection.m[0][0] = (float)(fax / aspect);
	projection.m[1][1] = (float)(fax);
	projection.m[2][2] = far_z / (far_z - near_z);
	projection.m[3][2] = near_z * far_z / (near_z - far_z);
	projection.m[2][3] = 1;
}

Mesh::Mesh(int count, int face_count)
{
	if (!count) return;
	this->vertex_cout = count;
	this->face_count = face_count;
	faces = NULL;
	vertices = NULL;
	if (count) vertices = new Vertex[count];
	if (face_count) faces = new Face[face_count];
}

Mesh::~Mesh()
{
	vertices = NULL;
	faces = NULL;
}

void Mesh::Get_face_normal(int i, Vector4f& normal)
{
	Vector4f edge1, edge2;
	edge1 = vertices[faces[i].v3].worldCoordinates - vertices[faces[i].v1].worldCoordinates;
	edge2 = vertices[faces[i].v2].worldCoordinates - vertices[faces[i].v1].worldCoordinates;
	normal = edge2 ^ edge1;
}




////////////////
//渲染抽象设备//
////////////////

//初始化设备
Device::Device(int w, int h,void*fb)
{
	width = w;
	height = h;
	transform.Init(width, height);

	int need = sizeof(void*)*(height * 2 + 1024) + width * height * 8;
	char *ptr = (char*)malloc(need + 64);
	char *framebuf, *zbuf;
	int j;
	assert(ptr != NULL);
	framebuffer = (UINT32**)ptr;
	zbuffer = (float**)(ptr + sizeof(void*)*height);
	ptr += sizeof(void*)*height * 2;
	ptr += sizeof(void*) * 1024;
	framebuf = (char*)ptr;
	zbuf = (char*)ptr + width * height * 4;
	ptr += width * height * 8;
	if (fb != NULL) framebuf = (char*)fb;
	for (j = 0; j< height; j++)
	{
		framebuffer[j] = (UINT32*)(framebuf + width * 4 * j);
		zbuffer[j] = (float*)(zbuf + width * 4 * j);
	}
	background = 0;
}

//释放指针
Device::~Device()
{
	if (framebuffer)
	{
		free(framebuffer);
	}
	framebuffer = NULL;
	zbuffer = NULL;
}

//清空framebuffer和zbuffer
void Device::Clear(int mode)
{
	int x, y, height = this->height;
	for (y = 0; y < height; y++)
	{
		UINT32 *dst = framebuffer[y];
		UINT32 cc = (height - 1 - y) * 230 / (height - 1);
		cc = (cc << 16) | (cc << 8) | cc;
		if (mode == 0) cc = background;
		for ( x = width; x > 0; dst++,x--)
		{
			dst[0] = cc;
		}
	}

	//将深度设置为足够大的默认值
	for ( y = 0; y < height; y++)
	{
		float *dst = zbuffer[y];
		for (x = width; x > 0; dst++, x--) dst[0] = 65535.f;
	}
}

//背面剔除
bool Device::BackfaceCulling(Vector4f p0, Vector4f p1, Vector4f p2)
{
	//计算三角形所在面的法线
	Vector4f normal;
	Vector4f v1 = p2 - p0;
	Vector4f v2 = p1 - p0;
	normal = v1 ^ v2;

	//计算三顶点的中心
	Vector4f center_point;
	center_point = (p0 + p1 + p2) * 0.333333f;
	if (my_camera.plane_camera_cos(center_point, normal)) return true;
	else return false;
}

//像素填充函数
void Device::PutPixel(int x, int y, UINT32& color)
{
	if (((UINT32)x) < (UINT32)width && ((UINT32)y)<(UINT32)height) 
	{
		framebuffer[y][x] = color;
	}
}

//带深度测试的像素填充函数
void Device::PutPixel(int x, int y, int z, UINT32& color)
{
	if (((UINT32)x) < (UINT32)width && ((UINT32)y) < (UINT32)height)
	{
		if (this->my_camera.GetPosition().z - z >= zbuffer[y][x]) return;
		zbuffer[y][x] = this->my_camera.GetPosition().z - z;
		framebuffer[y][x] = color;
	}
}

//利用bresenham line算法画线
void Device::DrawLine(Vector2i p1, Vector2i p2, UINT32 color)
{
	int x0 = p1.x;
	int y0 = p1.y;
	int x1 = p2.x;
	int y1 = p2.y;
	bool steep = false;

	if (abs(x1 - x0) < abs(y1 - y0))
	{
		swap(x0, y0);
		swap(x1, y1);
		steep = true;
	}

	if (x0 > x1)
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
			PutPixel(y, x, color);
		}
		else
		{
			PutPixel(x, y, color);
		}

		e += de;
		if (e > dx)
		{
			y += (y1 > y0 ? 1 : -1);
			e -= dx * 2;
		}
	}
}

//含有简单的z插值
void Device::DrawLine(Vector3i p1, Vector3i p2, UINT32 color)
{
	int x0 = p1.x;
	int y0 = p1.y;
	int x1 = p2.x;
	int y1 = p2.y;
	bool steep = false;

	if (abs(x1 - x0) < abs(y1 - y0))
	{
		swap(x0, y0);
		swap(x1, y1);
		steep = true;
	}

	if (x0 > x1)
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
		//z插值
		float gradient = 0;
		gradient = (x - x0) / (float)(x1 - x0);
		float z = p1.z + (p2.z - p1.z)*gradient;

		if (steep)
		{
			PutPixel(y, x, z, color);
		}
		else
		{
			PutPixel(x, y, z, color);
		}

		e += de;
		if (e > dx)
		{
			y += (y1 > y0 ? 1 : -1);
			e -= dx * 2;
		}
	}
}

void Device::DrawTriangle(Vertex A, Vertex B, Vertex C, UINT32 color)
{
	if (A.coordinates.y == B.coordinates.y && B.coordinates.y == C.coordinates.y)
		return;
	if (A.coordinates.y > B.coordinates.y) std::swap(A, B);
	if (A.coordinates.y > C.coordinates.y) std::swap(A, C);
	if (B.coordinates.y > C.coordinates.y) std::swap(B, C);
	int total_height = C.coordinates.y - A.coordinates.y;

	for (int i = 0; i < total_height; i++)
	{
		bool second_half = i > B.coordinates.y - A.coordinates.y || B.coordinates.y == A.coordinates.y;
		int segment_height = second_half ? C.coordinates.y - B.coordinates.y : B.coordinates.y - A.coordinates.y;
		float alpha = (float)i / total_height;
		float beta = (float)(i - (second_half ? B.coordinates.y - A.coordinates.y : 0)) / segment_height; //second_half ? (float)(i - B.y) / segment_height : (float)(i - A.y) / segment_height;
		Vector4f X = A.coordinates + Vector4f(C.coordinates - A.coordinates) *alpha;
		Vector4f Y = second_half ? B.coordinates + Vector4f(C.coordinates - B.coordinates)*beta : A.coordinates + Vector4f(B.coordinates - A.coordinates)*beta;
		if (X.x > Y.x) swap(X, Y);
		for (int x = X.x; x <= Y.x; x++)
		{
			float phi = Y.x == X.x ? 1.0f : (float)(x - X.x) / (float)(Y.x - X.x);
			Vector4f P = Vector4f(X) + Vector4f(Y - X)*phi;
			if (P.x > width || P.x < 0 || P.y>height || P.y < 0)
				continue;
			if (zbuffer[int(P.y)][int(P.x)] > P.z)
			{
				zbuffer[int(P.y)][int(P.x)] = P.z;
				PutPixel(int(P.x), int(P.y), color);
			}
		}
	}
}

void Device::Render(Model& model, int op)
{
	transform.Update();

	Clear(0);
	UINT32 color[] = { 0x00ff0000 ,0x0000ff00,0x000000ff,0x00ffff00,
		0x00efefef,0x00eeffcc,0x00cc00ff,0x0015ffff,
		0x00121212,0x00001233,0x5615cc,0x353578,
		0x00ffffff};

	transform.world = Matrix::TranslateMatrix(model.Position().x, model.Position().y, model.Position().z);
	transform.world = Matrix::RotateMatrix(model.rotation.x, model.rotation.y, model.rotation.z, model.rotation.w);
	transform.Update();

	Vertex re, re2, re3, re4;

	for (int i = 0; i < model.nfaces(); i++)
	{
		transform.Apply(model.vertices[model.faces[i][0]], re2);
		transform.Homogenize(re2.coordinates, re2.coordinates);
		transform.Apply(model.vertices[model.faces[i][1]], re3);
		transform.Homogenize(re3.coordinates, re3.coordinates);
		transform.Apply(model.vertices[model.faces[i][2]], re4);
		transform.Homogenize(re4.coordinates, re4.coordinates);
		//vector<int> face = model.face(i);
		//Vector3i screen_coords[3];
		//Vector4f world_coords[3];
		//for (int j = 0; j < 3; j++)
		//{
		//	Vector4f v = model.vert(face[j]);
		//	Vector4f re;
		//	transform.Apply(v, re);
		//	transform.Homogenize(re, re);
		//	screen_coords[j] = re;//Vector3i(int((v.x + 1.)*width / 2.), int((v.y + 1.)*height / 2.),v.z);
		//}
		DrawTriangle(re2, re3, re4, color[i]);
	}
}









#include "render.h"
#include <ctime>

Vector4i RandomColor(int index)
{
	srand((unsigned)time(NULL));
	Vector4i color;
	for (int i = 0; i < 3; i++)
	{
		unsigned seed = rand() % 100 +index;
		srand(seed);
		color[i] = rand() % 256;
	}

	return color;
}

UINT32 ConvertRGBTOUINT(Vector4i color)
{
	color.w = CMID(color.w, 0, 255);
	color.x = CMID(color.x, 0, 255);
	color.y = CMID(color.y, 0, 255);
	color.z = CMID(color.z, 0, 255);
	UINT32 color_ = ((color.w & 0xff) << 24) + ((color.x & 0xff) << 16) + ((color.y & 0xff) << 8) + (color.z & 0xff);
	return color_;
}

//矩阵更新计算：transform = projection * view * world
void Transform::Update()
{
	static Matrix m;
	m = Matrix::Identity(4);
	m = world * view;
	transform = m * projection;
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
	re.color = op.color;
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
	projection.m[3][2] = - near_z * far_z / (far_z - near_z);
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
bool Device::BackfaceCulling(Vertex p0, Vertex p1, Vertex p2, Vector4f normal)
{
	////计算三角形所在面的法线
	//Vector4f normal;
	float temp = float(1) / float(3);
	//normal = (p0.normal + p1.normal + p2.normal) * temp;

	//计算三顶点的中心
	Vector4f center_point;
	center_point = (p0.worldCoordinates + p1.worldCoordinates + p2.worldCoordinates) * temp;
	if (my_camera.plane_camera_cos(center_point, normal)> 0) return true;
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
void Device::PutPixel(int x, int y, float z, UINT32& color)
{
	if (((UINT32)x) < (UINT32)width && ((UINT32)y) < (UINT32)height)
	{
		if ( z >= zbuffer[y][x]) return;
		zbuffer[y][x] = z;
		framebuffer[y][x] =color;
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

//扫描线
void Device::ProcessScanLine(int curY, Vector4f& pa, Vector4f& pb, Vector4f& pc, Vector4f& pd, UINT32& color)
{
	float gradient_s = pa.y != pb.y ? (curY - pa.y) / (pb.y - pa.y) : 1;
	float gradient_e = pc.y != pd.y ? (curY - pc.y) / (pd.y - pc.y) : 1;

	int sx = INTERP(pa.x, pb.x, gradient_s);
	int ex = INTERP(pc.x, pd.x, gradient_e);

	//深度线性插值
	float z1 = INTERP(pa.z, pb.z, gradient_s);
	float z2 = INTERP(pc.z, pd.z, gradient_e);

	for (int x = sx; x <= ex; x++)
	{
		float gradient = (x - sx) / (float)(ex - sx);
		float z = INTERP(z1, z2, gradient);
		PutPixel(x, curY, z, color);
	}
}

void Device::ProcessScanLine(ScanLineData scanline, Vector4f& pa, Vector4f& pb, Vector4f& pc, Vector4f& pd)
{
	float gradient_s = pa.y != pb.y ? (scanline.currentY - pa.y) / (pb.y - pa.y) : 1;
	float gradient_e = pc.y != pd.y ? (scanline.currentY - pc.y) / (pd.y - pc.y) : 1;

	int sx = INTERP(pa.x, pb.x, gradient_s);
	int ex = INTERP(pc.x, pd.x, gradient_e);

	//深度线性插值
	float z1 = INTERP(pa.z, pb.z, gradient_s);
	float z2 = INTERP(pc.z, pd.z, gradient_e);

	for (int x = sx; x <= ex; x++)
	{
		float gradient = (x - sx) / (float)(ex - sx);
		Vector4i colorRGB = scanline.leftColor + (scanline.rightColor - scanline.leftColor)*gradient;
		UINT32 color = ConvertRGBTOUINT(colorRGB);
		float z = INTERP(z1, z2, gradient);
		PutPixel(x, scanline.currentY, z, color);
	}
}


void Device::DrawTriangleFrame(Vertex A, Vertex B, Vertex C, UINT32 color)
{
	if (A.coordinates.y == B.coordinates.y && B.coordinates.y == C.coordinates.y)
		return;
	Vector3i p1, p2, p3;
	p1 = A.coordinates;
	p2 = B.coordinates;
	p3 = C.coordinates;

	DrawLine(p1, p2,color);
	DrawLine(p1, p3, color);
	DrawLine(p2, p3,color);
}

void Device::DrawTriangleFlat(Vertex A, Vertex B, Vertex C, UINT32 color)
{
	Vector4f pa = A.coordinates;
	Vector4f pb = B.coordinates;
	Vector4f pc = C.coordinates;

	//按y值大小按从小到大顺序排列
	if (pa.y > pb.y) swap(pa, pb);
	if (pa.y > pc.y) swap(pa, pc);
	if (pb.y > pc.y) swap(pb, pc);
	//两种特殊情况
	//平顶
	if (pa.y == pb.y)
	{
		if (pa.x < pb.x) swap(pa, pb);
		for (int row = (int)pa.y; row <= (int)pc.y; row++)
		{
			ProcessScanLine(row, pb, pc, pa, pc, color);                     
		}
		return;
	}

	//平底
	if (pc.y == pb.y)
	{
		if (pc.x < pb.x) swap(pb, pc);
		for (int row = (int)pa.y; row <= (int)pc.y; row++)
		{
			ProcessScanLine(row, pa, pb, pc, pa, color);
		}

		return;
	}

	float dPaPb, dPaPc;
	if (pb.y - pa.y > 0)
		dPaPb = (pb.x - pa.x) / (pb.y - pa.y);
	else
		dPaPb = 0;

	if (pc.y - pa.y > 0)
		dPaPc = (pc.x - pa.x) / (pc.y - pa.y);
	else
		dPaPc = 0;

	if (dPaPb > dPaPc)
	{
		for (int row = (int)pa.y; row<=(int)pc.y;row++)
		{
			if (row < pb.y)
			{
				ProcessScanLine(row, pa, pc, pb, pa, color);
			}
			else
			{
				ProcessScanLine(row, pa, pc, pb, pc, color);
			}
		}
	}
	else
	{
		for (int row = (int)pa.y; row <= (int)pc.y; row++)
		{
			if (row < pb.y)
			{
				ProcessScanLine(row, pa, pb, pc, pa, color);
			}
			else
			{
				ProcessScanLine(row, pb, pc, pa, pc, color);
			}
		}
	}
}

void Device::DrawTriangleFlat(Vertex A, Vertex B, Vertex C)
{
	Vector4f pa = A.coordinates;
	Vector4f pb = B.coordinates;
	Vector4f pc = C.coordinates;

	//按y值大小按从小到大顺序排列
	if (pa.y > pb.y)
	{
		swap(pa, pb); swap(A, B);
	}
	if (pa.y > pc.y)
	{
		swap(pa, pc); swap(A, C);
	}
	if (pb.y > pc.y)
	{
		swap(pb, pc); swap(B, C);
	}

	ScanLineData scanline;

	//两种特殊情况
	//平顶
	if (pa.y == pb.y)
	{
		if (pa.x < pb.x)
		{
			swap(pa, pb); swap(A, B);
		}
		for (int row = (int)pa.y; row <= (int)pc.y; row++)
		{
			float gradient = (float)(row - (int)pa.y) / (float)(int(pc.y) - int(pa.y));
			//scanline.leftColor = { 128,50,0,0 };
			//scanline.rightColor = { 0,255,128,0 };
			scanline.currentY = row;
			scanline.leftColor = (C.color - B.color)*gradient + B.color;
			scanline.rightColor = (C.color - A.color)*gradient + A.color;
			ProcessScanLine(scanline, pb, pc, pa, pc);
		}
		return;
	}

	//平底
	if (pc.y == pb.y)
	{
		if (pc.x < pb.x)
		{
			swap(pb, pc);
			swap(B, C);
		}
		for (int row = (int)pa.y; row <= (int)pc.y; row++)
		{
			float gradient = (float)(row - (int)pa.y) / (float)(int(pc.y) - int(pa.y));
			scanline.leftColor = (B.color - A.color)*gradient + A.color;
			scanline.rightColor = (C.color - A.color)*gradient + A.color;
			scanline.currentY = row;
			ProcessScanLine(scanline, pa, pb, pc, pa);
		}
		return;
	}

	float dPaPb, dPaPc;
	if (pb.y - pa.y > 0)
		dPaPb = (pb.x - pa.x) / (pb.y - pa.y);
	else
		dPaPb = 0;

	if (pc.y - pa.y > 0)
		dPaPc = (pc.x - pa.x) / (pc.y - pa.y);
	else
		dPaPc = 0;

	if (dPaPb > dPaPc)
	{
		for (int row = (int)pa.y; row <= (int)pc.y; row++)
		{
			if (row < pb.y)
			{
				float gradient_l = (float)(row - (int)pa.y) / (float)(int(pc.y) - int(pa.y));
				float gradient_r = (float)(row - (int)pa.y) / (float)(int(pb.y) - int(pa.y));
				scanline.leftColor = A.color + (C.color - A.color)*gradient_l;
				scanline.rightColor = A.color + (B.color - A.color)*gradient_r;
				scanline.currentY = row;
				ProcessScanLine(scanline, pa, pc, pb, pa);
			}
			else
			{
				float gradient_l = (float)(row - (int)pa.y) / (float)(int(pc.y) - int(pa.y));
				float gradient_r = (float)(row - (int)pb.y) / (float)(int(pc.y) - int(pb.y));
				scanline.leftColor = A.color + (C.color - A.color)*gradient_l;
				scanline.rightColor = B.color + (C.color - B.color)*gradient_r;
				scanline.currentY = row;
				ProcessScanLine(scanline, pa, pc, pb, pc);
			}
		}
	}
	else
	{
		for (int row = (int)pa.y; row <= (int)pc.y; row++)
		{
			if (row < pb.y)
			{
				float gradient_l = (float)(row - (int)pa.y) / (float)(int(pb.y) - int(pa.y));
				float gradient_r = (float)(row - (int)pa.y) / (float)(int(pc.y) - int(pa.y));
				scanline.leftColor = A.color + (B.color - A.color)*gradient_l;
				scanline.rightColor = A.color + (C.color - A.color)*gradient_r;
				scanline.currentY = row;
				ProcessScanLine(scanline, pa, pb, pc, pa);
			}
			else
			{
				float gradient_l = (float)(row - (int)pb.y) / (float)(int(pc.y) - int(pb.y));
				float gradient_r = (float)(row - (int)pa.y) / (float)(int(pc.y) - int(pa.y));
				scanline.leftColor = B.color + (C.color - B.color)*gradient_l;
				scanline.rightColor = A.color + (C.color - A.color)*gradient_r;
				scanline.currentY = row;
				ProcessScanLine(scanline, pb, pc, pa, pc);
			}
		}
	}
}

void Device::Render(Model& model, int op)
{
	transform.Update();

	UINT32 color[] = { 0x00ff0000 ,0x0000ff00,0x000000ff,0x00ffff00,
		0x00efefef,0x00eeffcc,0x00cc00ff,0x0015ffff,
		0x00121212,0x00001233,0x5615cc,0x353578,
		0x00ffffff};
	Vector4i myColor = { 0,255,0,0 };
	UINT32 color1 = ConvertRGBTOUINT(myColor);
	UINT32 color2 = color[0];

	//transform.world = Matrix::TranslateMatrix(model.Position().x, model.Position().y, model.Position().z);
	transform.world = Matrix::RotateMatrix(model.rotation.x, model.rotation.y, model.rotation.z, model.rotation.w);
	transform.Update();
	Clear(0);
	Vertex re2, re3, re4,re5;
	int count_backface = 0;

	//每个顶点据时间产生随机颜色
	for (int i = 0; i < model.nverts(); i++)
		model.vertices[i].color = RandomColor(i);

	for (int i = 0; i < model.nfaces(); i++)
	{
		transform.Apply(model.vertices[model.faces[i][0]], re2);
		transform.Homogenize(re2.coordinates, re2.coordinates);
		transform.Apply(model.vertices[model.faces[i][1]], re3);
		transform.Homogenize(re3.coordinates, re3.coordinates);
		transform.Apply(model.vertices[model.faces[i][2]], re4);
		transform.Homogenize(re4.coordinates, re4.coordinates);
		DrawTriangleFlat(re2, re3, re4,color[i/2]);//画渐变三角形
		//if (BackfaceCulling(re2,re3,re4,model.normals[i/2]))
		//{
		//	DrawTriangle(re2, re3, re4, color[i/2]);
		//}
		//else
		//{
		//	count_backface++;
		//}
	}
	//cout << count_backface<<endl;
}









#include "render.h"
#include <ctime>
#include <consoleapi2.h>


int face = 0;

Vector4i RandomColor(int index)
{
	srand((unsigned)time(NULL));
	Vector4i color;
	for (int i = 0; i < 3; i++)
	{
		unsigned seed = rand() % 100 + index;
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
	re.texcoord = op.texcoord;
	re.color = op.color;
}

void Transform::Apply(Vertex& op, Vertex& re, Vector2f uv)
{
	re.coordinates = transform * op.local;
	re.worldCoordinates = world * op.local;
	re.normal = world * op.normal;
	re.texcoord = uv;
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

void Transform::Homogenize(Vertex& op, Vertex& re)
{
	float  rhw = 1.0f / op.coordinates.w;
	re.coordinates.x = (op.coordinates.x *rhw + 1.0f) * width *0.5f;
	re.coordinates.y = (1.0f - op.coordinates.y * rhw) *height *0.5f;
	re.coordinates.z = op.coordinates.z *rhw;
	re.rhw = rhw;
	re.texcoord.x = op.texcoord.x *rhw;
	re.texcoord.y = op.texcoord.y *rhw;
	re.coordinates.w = 1.0f;
}


//根据fov等参数设置透视矩阵, aspect—宽高比，near_z—近裁面到相机距离，far_z—远裁面到相机距离
void Transform::Set_Perspective(float fovy, float aspect, float near_z, float far_z)
{
	float fax = 1.0f / (float)tan(fovy*0.5f);
	projection = Matrix::ZeroMatrix(4);
	projection.m[0][0] = (float)(fax / aspect);
	projection.m[1][1] = (float)(fax);
	projection.m[2][2] = far_z / (far_z - near_z);
	projection.m[3][2] = -near_z * far_z / (far_z - near_z);
	projection.m[2][3] = 1;
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
	result = { 0,0,0,0 };
	if (buf.empty()) return result;
	int u = (int)(tu*width)%width;
	int v = (int)(tv*width)%height;
	//if (u < 0 || v < 0)
	//	cout << tu << ' ' << tv << endl;
	u = u >= 0 ? u : -u;
	v = v >= 0 ? v : -v;

	cv::Vec3b tex_w = buf.at<cv::Vec3b>(v, u);
	result = { tex_w[2],tex_w[1],tex_w[0],0 };
	return result;
}

Light::Light()
{
	position = { 0,0,0,1 };
	color = { 1,1,1,1 };
	intensity = 1;
}

void Light::SetPosition(float x, float y, float z)
{
	position.x = x;
	position.y = y;
	position.z = z;
}


void Light::SetColor(int r, int g, int b)
{
	color.x = r;
	color.y = g;
	color.z = b;
}

void Light::SetIntensity(float a)
{
	intensity = a;
}



////////////////
//渲染抽象设备//
////////////////

//初始化设备
Device::Device(int w, int h, void*fb)
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
	for (j = 0; j < height; j++)
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
		for (x = width; x > 0; dst++, x--)
		{
			dst[0] = cc;
		}
	}

	//将深度设置为足够大的默认值
	for (y = 0; y < height; y++)
	{
		float *dst = zbuffer[y];
		for (x = width; x > 0; dst++, x--) dst[0] = 65535.f;
	}
}

//背面剔除
bool Device::BackfaceCulling(Vertex p0, Vertex p1, Vertex p2, Vector4f normal)
{
	if (face == 8)
	{
		float a = 1;
	}
	////计算三角形所在面的法线
	//Vector4f normal;
	float temp = float(1) / float(3);
	//normal = (p0.normal + p1.normal + p2.normal) * temp;
	Vector4f edge1 = p2.worldCoordinates - p0.worldCoordinates;
	Vector4f edge2 = p1.worldCoordinates - p0.worldCoordinates;
	Vector4f n = edge1 ^ edge2;

	//计算三顶点的中心
	Vector4f center_point;
	center_point = (p0.worldCoordinates + p1.worldCoordinates + p2.worldCoordinates) * temp;
	if (my_camera.plane_camera_cos(center_point, n) > 0) return true;
	else return false;
}


float Light::LightCos(Vector4f point, Vector4f normal)
{
	Vector4f light_dir;
	light_dir = position - point;
	light_dir.normalize();
	normal.normalize();
	float diffuse = normal * light_dir;
	diffuse = diffuse < 0 ? 0 : diffuse;
	diffuse = diffuse > 1 ? 1 : diffuse;
	return diffuse;
}



//像素填充函数
void Device::PutPixel(int x, int y, UINT32& color)
{
	if (((UINT32)x) < (UINT32)width && ((UINT32)y) < (UINT32)height)
	{
		framebuffer[y][x] = color;
	}
}

//带深度测试的像素填充函数
void Device::PutPixel(int x, int y, float z, UINT32& color)
{
	if (((UINT32)x) < (UINT32)width && ((UINT32)y) < (UINT32)height)
	{
		if (z >= zbuffer[y][x]) return;
		zbuffer[y][x] = z;
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
		//颜色的线性插值
		Vector4i colorRGB = scanline.leftColor + (scanline.rightColor - scanline.leftColor)*gradient;
		UINT32 color = ConvertRGBTOUINT(colorRGB);
		float z = INTERP(z1, z2, gradient);
		PutPixel(x, scanline.currentY, z, color);
	}
}

void Device::ProcessScanLineTexture(ScanLineData scanline, Vector4f& pa, Vector4f& pb, Vector4f& pc, Vector4f& pd,Texture &tex)
{
	float gradient_s = pa.y != pb.y ? (scanline.currentY - pa.y) / (pb.y - pa.y) : 1;
	float gradient_e = pc.y != pd.y ? (scanline.currentY - pc.y) / (pd.y - pc.y) : 1;

	int sx = INTERP(pa.x, pb.x, gradient_s);
	int ex = INTERP(pc.x, pd.x, gradient_e);

	//深度线性插值
	float z1 = INTERP(pa.z, pb.z, gradient_s);
	float z2 = INTERP(pc.z, pd.z, gradient_e);

	//纹理坐标插值
	float su = INTERP(scanline.ua, scanline.ub, gradient_s);
	float eu = INTERP(scanline.uc, scanline.ud, gradient_e);
	float sv = INTERP(scanline.va, scanline.vb, gradient_s);
	float ev = INTERP(scanline.vc, scanline.vd, gradient_e);

	//光照强度插值
	float sn = INTERP(scanline.ndotla, scanline.ndotlb, gradient_s);
	float en = INTERP(scanline.ndotlc, scanline.ndotld, gradient_e);

	//高光插值
	float specular_s = INTERP(scanline.speculardotla, scanline.speculardotlb, gradient_s);
	float specular_e = INTERP(scanline.speculardotlc, scanline.speculardotld, gradient_e);

	float srhw = INTERP(scanline.rhwa, scanline.rhwb, gradient_s);
	float erhw = INTERP(scanline.rhwc, scanline.rhwd, gradient_e);

	UINT32 color;
	for (int x = sx; x <= ex; x++)
	{
		float gradient = (x - sx) / (float)(ex - sx);
		float z = INTERP(z1, z2, gradient);
		float rhw = INTERP(srhw, erhw, gradient);
		rhw = 1 / rhw;
		//纹理映射插值
		float u = INTERP(su, eu, gradient);
		float v = INTERP(sv, ev, gradient);
		float diffuse = INTERP(sn, en, gradient);
		float specular = INTERP(specular_s, specular_e, gradient);
		//if (u>1 || v>1)
		//{
		//	cout << u << v;
		//}
		u = u * rhw;
		v = v * rhw;

		if (!tex.buf.empty())
		{
			Vector4i colorRGB = tex.Map(u, v);
			Vector4i diffuseColor = colorRGB * diffuse * diffuselight.intensity;
			Vector4i ambientColor = colorRGB + ambientLight.color * ambientLight.intensity;
			Vector4i specularColor = speculaLight.color * specular;
			colorRGB = diffuseColor + ambientColor + specularColor;
			color = ConvertRGBTOUINT(colorRGB);
		}
		else color = 0x00000000;
		//int colorR = (float)(u) * (int)255;
		//Vector4i colorRGB = { colorR,0,0,0 };
		//UINT tempColor = ConvertRGBTOUINT(colorRGB);
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

	DrawLine(p1, p2, color);
	DrawLine(p1, p3, color);
	DrawLine(p2, p3, color);
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
		for (int row = (int)pa.y; row <= (int)pc.y; row++)
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

void Device::DrawTriangleTexture(Vertex A, Vertex B, Vertex C)
{
	ScanLineData scanline;

	float temp = float(1) / float(3);
	//normal = (p0.normal + p1.normal + p2.normal) * temp;
	Vector4f edge1 = C.worldCoordinates - A.worldCoordinates;
	Vector4f edge2 = B.worldCoordinates - A.worldCoordinates;
	Vector4f n = edge1 ^ edge2;

	float ndotA = diffuselight.LightCos(A.worldCoordinates, n);
	float ndotB = diffuselight.LightCos(B.worldCoordinates, n);
	float ndotC = diffuselight.LightCos(C.worldCoordinates, n);

	float specularPower = 1.0f;

	Vector4f lightDir =speculaLight.position - A.worldCoordinates;
	Vector4f reflectionVector = n * 2 * ndotA - lightDir;
	Vector4f viewDir = my_camera.position - A.worldCoordinates;
	reflectionVector.normalize();
	float specularA = pow(reflectionVector*viewDir, specularPower);

	lightDir = speculaLight.position - B.worldCoordinates;
	reflectionVector = n * 2 * ndotA - lightDir;
	viewDir = my_camera.position - B.worldCoordinates;
	reflectionVector.normalize();
	float specularB = pow(reflectionVector*viewDir, specularPower);

	lightDir = speculaLight.position - C.worldCoordinates;
	reflectionVector = n * 2 * ndotA - lightDir;
	viewDir = my_camera.position - C.worldCoordinates;
	reflectionVector.normalize();
	float specularC = pow(reflectionVector*viewDir, specularPower);

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

	if (face == 8)
	{
		float a = 1;
	}

	if (pa.y == pb.y)
	{
		if (pa.x < pb.x)
		{
			swap(pa, pb);
			swap(A, B);
		}
		for (int row = (int)pa.y;row<=(int)pc.y;row++)
		{
			scanline.currentY = row;
			scanline.ua = B.texcoord.x;
			scanline.ub = C.texcoord.x;
			scanline.uc = A.texcoord.x;
			scanline.ud = C.texcoord.x;
			scanline.va = B.texcoord.y;
			scanline.vb = C.texcoord.y;
			scanline.vc = A.texcoord.y;
			scanline.vd = C.texcoord.y;
			scanline.rhwa = B.rhw;
			scanline.rhwb = C.rhw;
			scanline.rhwc = A.rhw;
			scanline.rhwd = C.rhw;
			scanline.ndotla = ndotB;
			scanline.ndotlb = ndotC;
			scanline.ndotlc = ndotA;
			scanline.ndotld = ndotC;
			scanline.speculardotla = specularB;
			scanline.speculardotlb = specularC;
			scanline.speculardotlc = specularA;
			scanline.speculardotld = specularC;
			ProcessScanLineTexture(scanline, pb, pc, pa, pc, texture);
		}
		return;
	}

	if (pc.y == pb.y)
	{
		if (pc.x < pb.x)
		{
			swap(pc, pb);
			swap(C, B);
		}
		for (int row = (int)pa.y; row <= (int)pc.y; row++)
		{
			scanline.currentY = row;
			scanline.ua = A.texcoord.x;
			scanline.ub = B.texcoord.x;
			scanline.uc = C.texcoord.x;
			scanline.ud = A.texcoord.x;
			scanline.va = A.texcoord.y;
			scanline.vb = B.texcoord.y;
			scanline.vc = C.texcoord.y;
			scanline.vd = A.texcoord.y;
			scanline.rhwa = A.rhw;
			scanline.rhwb = B.rhw;
			scanline.rhwc = C.rhw;
			scanline.rhwd = A.rhw;
			scanline.ndotla = ndotA;
			scanline.ndotlb = ndotB;
			scanline.ndotlc = ndotC;
			scanline.ndotld = ndotA;
			scanline.speculardotla = specularA;
			scanline.speculardotlb = specularB;
			scanline.speculardotlc = specularC;
			scanline.speculardotld = specularA;
			ProcessScanLineTexture(scanline, pa, pb, pc, pa, texture);
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
			scanline.currentY = row;
			if (row < pb.y)
			{
				scanline.ua = A.texcoord.x;
				scanline.ub = C.texcoord.x;
				scanline.uc = B.texcoord.x;
				scanline.ud = A.texcoord.x;
				scanline.va = A.texcoord.y;
				scanline.vb = C.texcoord.y;
				scanline.vc = B.texcoord.y;
				scanline.vd = A.texcoord.y;
				scanline.rhwa = A.rhw;
				scanline.rhwb = C.rhw;
				scanline.rhwc = B.rhw;
				scanline.rhwd = A.rhw;
				scanline.ndotla = ndotA;
				scanline.ndotlb = ndotC;
				scanline.ndotlc = ndotB;
				scanline.ndotld = ndotA;
				scanline.speculardotla = specularA;
				scanline.speculardotlb = specularC;
				scanline.speculardotlc = specularB;
				scanline.speculardotld = specularA;
				ProcessScanLineTexture(scanline, pa, pc, pb, pa, texture);
			}
			else
			{
				scanline.ua = A.texcoord.x;
				scanline.ub = C.texcoord.x;
				scanline.uc = B.texcoord.x;
				scanline.ud = C.texcoord.x;
				scanline.va = A.texcoord.y;
				scanline.vb = C.texcoord.y;
				scanline.vc = B.texcoord.y;
				scanline.vd = C.texcoord.y;
				scanline.rhwa = A.rhw;
				scanline.rhwb = C.rhw;
				scanline.rhwc = B.rhw;
				scanline.rhwd = C.rhw;
				scanline.ndotla = ndotA;
				scanline.ndotlb = ndotC;
				scanline.ndotlc = ndotB;
				scanline.ndotld = ndotC;
				scanline.speculardotla = specularA;
				scanline.speculardotlb = specularC;
				scanline.speculardotlc = specularB;
				scanline.speculardotld = specularC;
				ProcessScanLineTexture(scanline, pa, pc, pb, pc, texture);
			}
		}
	}
	else
	{
		for (int row = (int)pa.y; row <= (int)pc.y; row++)
		{
			scanline.currentY = row;
			if (row < pb.y)
			{
				scanline.ua = A.texcoord.x;
				scanline.ub = B.texcoord.x;
				scanline.uc = C.texcoord.x;
				scanline.ud = A.texcoord.x;
				scanline.va = A.texcoord.y;
				scanline.vb = B.texcoord.y;
				scanline.vc = C.texcoord.y;
				scanline.vd = A.texcoord.y;
				scanline.rhwa = A.rhw;
				scanline.rhwb = B.rhw;
				scanline.rhwc = C.rhw;
				scanline.rhwd = A.rhw;
				scanline.ndotla = ndotA;
				scanline.ndotlb = ndotB;
				scanline.ndotlc = ndotC;
				scanline.ndotld = ndotA;
				scanline.speculardotla = specularA;
				scanline.speculardotlb = specularB;
				scanline.speculardotlc = specularC;
				scanline.speculardotld = specularA;
				ProcessScanLineTexture(scanline, pa, pb, pc, pa, texture);
			}
			else
			{
				scanline.ua = B.texcoord.x;
				scanline.ub = C.texcoord.x;
				scanline.uc = A.texcoord.x;
				scanline.ud = C.texcoord.x;
				scanline.va = B.texcoord.y;
				scanline.vb = C.texcoord.y;
				scanline.vc = A.texcoord.y;
				scanline.vd = C.texcoord.y;
				scanline.rhwa = B.rhw;
				scanline.rhwb = C.rhw;
				scanline.rhwc = A.rhw;
				scanline.rhwd = C.rhw;
				scanline.ndotla = ndotB;
				scanline.ndotlb = ndotC;
				scanline.ndotlc = ndotA;
				scanline.ndotld = ndotC;
				scanline.speculardotla = specularB;
				scanline.speculardotlb = specularC;
				scanline.speculardotlc = specularA;
				scanline.speculardotld = specularC;
				ProcessScanLineTexture(scanline, pb, pc, pa, pc, texture);
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
		0x00ffffff };
	Vector4i myColor = { 0,255,0,0 };
	UINT32 color1 = ConvertRGBTOUINT(myColor);
	UINT32 color2 = color[0];

	transform.world = Matrix::TranslateMatrix(model.Position().x, model.Position().y, model.Position().z);
	transform.world = Matrix::RotateMatrix(model.rotation.x, model.rotation.y, model.rotation.z, model.rotation.w);
	transform.Update();
	Clear(0);
	Vertex re2, re3, re4, re5;
	int count_backface = 0;

	//每个顶点据时间产生随机颜色
	//for (int i = 0; i < model.nverts(); i++)
	//	model.vertices[i].color = RandomColor(i);

	for (int i = 0; i < model.nfaces(); i++)
	{
		transform.Apply(model.vertices[model.faces[i][0][0]], re2,model.getUV(i,0));
		transform.Homogenize(re2, re2);
		transform.Apply(model.vertices[model.faces[i][1][0]], re3, model.getUV(i, 1));
		transform.Homogenize(re3, re3);
		transform.Apply(model.vertices[model.faces[i][2][0]], re4, model.getUV(i, 2));
		transform.Homogenize(re4, re4);
		face = i;
		//float temp = 1 / re4.rhw;
		//re4.texcoord.x *= temp;
		//re4.texcoord.y *= temp;
		////DrawTriangleFrame(re2, re3, re4, color[i / 2]);
		if (!BackfaceCulling(re2, re3, re4, model.normals[i / 2]))
		{
			continue;
		}
		switch (op)
		{
		case 0:DrawTriangleFrame(re2, re3, re4, color[i / 2]); break;
		case 1:DrawTriangleFlat(re2, re3, re4, color[i / 2]); break;
		case 2:DrawTriangleFlat(re2, re3, re4); break;
		case 3:DrawTriangleTexture(re2, re3, re4); break;
		}
	}
}
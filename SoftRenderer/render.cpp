#include "render.h"
#include <ctime>

int face = 0;
int modelNum = -1;
//HANDLE my_mutex;
//int nowY;
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
	worldToProjection = view * projection;
	inverTransform = transform.Inverse();
}

void Transform::LightUpdate()
{
	static Matrix m;
	m = Matrix::Identity(4);
	m = world * view;
	transform = m * ortho;
	worldToProjection = view * ortho;
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

void Transform::ShadowHomogenize(Vertex& op, Vertex& re, int w, int h)
{
	re.coordinates.x = (op.coordinates.x + 1.0f) * w *0.5f;
	re.coordinates.y = (1.0f - op.coordinates.y) *h *0.5f;
	re.coordinates.z = op.coordinates.z;
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

void Transform::Set_Ortho(float w, float h, float near_z, float far_z)
{
	ortho = Matrix::ZeroMatrix(4);
	ortho[0][0] = float(2) / w;
	ortho[1][1] = float(2) / h;
	ortho[2][2] = float(1) / (far_z - near_z);
	ortho[3][3] = 1;
	ortho[3][2] = near_z / (near_z - far_z);
}

void Transform::Set_OrthoOffCenter(float l, float r, float b, float t, float zn, float zf)
{
	ortho = Matrix::ZeroMatrix(4);
	ortho[0][0] = float(2) / r - l;
	ortho[1][1] = float(2) / t - b;
	ortho[2][2] = float(1) / (zf - zn);
	ortho[3][3] = 1;
	ortho[3][0] = (l + r) / (l - r);
	ortho[3][1] = (t + b) / (t - b);
	ortho[3][2] = zn / (zn - zf);
}

void Transform::ScreenToWorld(Vertex& re, float rhw)
{
	re.coordinates.x = ((re.coordinates.x*2.f / width) - 1.0f) / rhw;
	re.coordinates.y = (1 - (re.coordinates.y*2.f / height)) / rhw;
	re.coordinates.z = re.coordinates.z / rhw;
	re.coordinates.w = 1 / rhw;
	re.local = inverTransform * re.coordinates;
	re.worldCoordinates = world * re.local;
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

float Light::DiffuseLightCos(Vector4f normal)
{
	Vector4f light_dir;
	light_dir = lightDir;
	light_dir.normalize();
	normal.normalize();
	float diffuse = normal * light_dir;
	diffuse = diffuse < 0 ? 0 : diffuse;
	diffuse = diffuse > 1 ? 1 : diffuse;
	return diffuse;
}

void Light::SetDiffuseLightDir(float x, float y, float z)
{
	lightDir.x = x;
	lightDir.y = y;
	lightDir.z = z;
}

////////////////
//渲染抽象设备//
////////////////

//初始化设备
Device::Device(int w, int h, void*fb, int sw, int sh,int threadNum)
{
	width = w;
	height = h;
	shadowWidth = sw;
	shadowHeight = sh;
	transform.Init(width, height);
	lightTransform.Init(sw, sh);
	threadpool.Init(threadNum);
	int need = sizeof(void*)*(height * 2 + 1024) + width * height * 8;
	char *ptr = (char*)malloc(need + 64);
	char *framebuf, *zbuf;
	int j;
	assert(ptr != NULL);
	framebuffer = (UINT32**)ptr;
	zbuffer = (float**)(ptr + sizeof(void*)*height);
	shadowDepthbuffer = std::vector<std::vector<float> >(sh, std::vector<float>(sw, 0.f));
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
	threadpool.start();
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
	float mm = shadowDepthbuffer[251][400];
	//将深度设置为足够大的默认值
	for (y = 0; y < height; y++)
	{
		float *dst = zbuffer[y];
		for (x = width; x > 0; dst++, x--) dst[0] = 65535.f;
	}
	mm = shadowDepthbuffer[251][400];
}

//清空阴影buffer
void Device::ClearShadowBuf()
{
	int x, y;

	//将深度设置为足够大的默认值
	for (y = 0; y < shadowHeight; y++)
	{
		for (x = 0; x < shadowWidth; x++) shadowDepthbuffer[y][x] = 65535.f;
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

//计算世界坐标转换为光照相机屏幕坐标
Vector3f Device::PointInLightSpace(Vector4f worldCoord)
{
	Vector3f result;
	Vector4f temp;
	temp = lightTransform.worldToProjection * worldCoord;
	result.x = (temp.x + 1.0f) * shadowWidth *0.5f;
	result.y = (1.0f - temp.y) *shadowHeight *0.5f;
	result.z = temp.z;
	return result;
}

//动态设置阴影相机
float tempMin, tempMax;
void Device::SetupShadowCamera(vector<Model> models)
{
	Vector4f up = { 0,1,0,1 };
	float max_x = -65535.f, min_x = 65535.f, max_y = -65535.f, min_y = 65535.f, max_z = -65535.f, min_z = 65535.f;
	Vector4f position;
	for (int i = 0; i < models.size(); i++)
	{
		Model model = models[i];
		lightTransform.world = Matrix::RotateMatrix(model.rotation.x, model.rotation.y, model.rotation.z, model.rotation.w);
		lightTransform.world = Matrix::TranslateMatrix(model.Position().x, model.Position().y, model.Position().z, lightTransform.world);
		for (int j = 0; j < model.vertices.size(); j++)
		{
			position = lightTransform.world*model.vertices[j].local;
			if (position.x > max_x)
			{
				max_x = position.x;
			}
			if (position.x < min_x)
			{
				min_x = position.x;
			}
			if (position.y > max_y)
			{
				max_y = position.y;
			}
			if (position.y < min_y)
			{
				min_y = position.y;
			}
			if (position.z > max_z)
			{
				max_z = position.z;
			}
			if (position.z < min_z)
			{
				min_z = position.z;
			}
		}
	}
	position.x = 0;
	position.y = 0;
	position.z = 0;

	Vector4f distance = diffuselight.lightDir * 10;
	shadowCamera.SetPosition(distance.x, distance.y, distance.z);
	shadowCamera.SetCamera(position, up);
	lightTransform.view = shadowCamera.view;
	Matrix m;
	max_x = -65535.f, min_x = 65535.f, max_y = -65535.f, min_y = 65535.f, max_z = -65535.f, min_z = 65535.f;
	for (int i = 0; i < models.size(); i++)
	{
		Model model = models[i];
		lightTransform.world = Matrix::RotateMatrix(model.rotation.x, model.rotation.y, model.rotation.z, model.rotation.w);
		lightTransform.world = Matrix::TranslateMatrix(model.Position().x, model.Position().y, model.Position().z, lightTransform.world);
		m = lightTransform.world * lightTransform.view;
		for (int j = 0; j < model.vertices.size(); j++)
		{
			position = m * model.vertices[j].local;
			if (position.x > max_x)
			{
				max_x = position.x;
			}
			if (position.x < min_x)
			{
				min_x = position.x;
			}
			if (position.y > max_y)
			{
				max_y = position.y;
			}
			if (position.y < min_y)
			{
				min_y = position.y;
			}
			if (position.z > max_z)
			{
				max_z = position.z;
			}
			if (position.z < min_z)
			{
				min_z = position.z;
			}
		}
	}

	position.x = (max_x + min_x) / float(2);
	position.y = (max_y + min_y) / float(2);
	position.z = (max_z + min_z) / float(2);

	if (firstTimeSetUpShadowCamera)
	{
		m = lightTransform.view.Inverse();
	}
	position = m * position;

	shadowCamera.SetPosition(position.x + distance.x, position.y + distance.y, position.z + distance.z);
	shadowCamera.SetCamera(position, up);
	lightTransform.view = shadowCamera.view;

	if (firstTimeSetUpShadowCamera)
	{
		max_shadowWidth = max_x - min_x;
		max_shadowHeight = max_y - min_y;
		firstTimeSetUpShadowCamera = false;
	}
	else
	{
		if (max_x - min_x > max_shadowWidth)
		{
			max_shadowWidth = max_x - min_x;
		}
		if (max_y - min_y > max_shadowHeight)
		{
			max_shadowHeight = max_y - min_y;
		}
	}
	float x = (float)4 / (float)3;
	lightTransform.Set_Ortho(max_shadowWidth, max_shadowHeight, min_z, max_z);
	cout << min_z << " " << max_z << endl;
	tempMin = min_z;
	tempMax = max_z;
}

void ThreadFunc(Device *device,ScanLineData& scanline,Vector4f pa, Vector4f pb, Vector4f pc, Vector4f pd, Texture texture)
{
	device->ProcessScanLineTexture(scanline, pa, pb, pc, pd, texture);
}

void func(Device *device) {
	
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

//扫线法（有纹理图）
void Device::ProcessScanLineTexture(ScanLineData scanline, Vector4f& pa, Vector4f& pb, Vector4f& pc, Vector4f& pd, Texture &tex)  
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

	////光照强度插值    高氏着色
	//float sn = INTERP(scanline.ndotla, scanline.ndotlb, gradient_s);
	//float en = INTERP(scanline.ndotlc, scanline.ndotld, gradient_e);

	//高光插值
	Vector4f world_s = Vector_Interp(scanline.world_a, scanline.world_b, gradient_s);
	Vector4f world_e = Vector_Interp(scanline.world_c, scanline.world_d, gradient_e);

	float srhw = INTERP(scanline.rhwa, scanline.rhwb, gradient_s);
	float erhw = INTERP(scanline.rhwc, scanline.rhwd, gradient_e);
	int y = scanline.currentY;
	UINT32 color;
	for (int x = sx; x <= ex; x++)
	{
		if (ex == sx) continue;
		float gradient = (x - sx) / (float)(ex - sx);
		float z = INTERP(z1, z2, gradient);
		float rhw = INTERP(srhw, erhw, gradient);

		Vertex re;
		re.coordinates.x = x;
		re.coordinates.y = scanline.currentY;
		re.coordinates.z = z;
		re.coordinates.w = 1.0f;
		transform.ScreenToWorld(re, rhw);            //将当前像素坐标逆变换为世界坐标，用于之后计算高光和阴影

		rhw = 1 / rhw;
		//纹理映射插值
		float u = INTERP(su, eu, gradient);
		float v = INTERP(sv, ev, gradient);

		//漫反射
		float diffuse = scanline.diffuse;

		//高光
		float specularPower = this->specularPower;

		Vector4f lightDir = speculaLight.position - re.worldCoordinates;
		float ndot = lightDir.normalize() * scanline.n;
		Vector4f reflectionVector = scanline.n * 2 * ndot - lightDir;
		Vector4f viewDir = my_camera.position - re.worldCoordinates;
		reflectionVector.normalize();
		viewDir.normalize();
		float specular = CMIDF(pow(reflectionVector*viewDir, specularPower), 0, 1);
		if (isnan(specular))
		{
			specular = 0;
		}

		u = u * rhw;
		v = v * rhw;

		z = (z + 1) / 2;
		z = z * rhw;
		Vector3f pixelInlight = PointInLightSpace(re.worldCoordinates);
		int lightX = pixelInlight.x;
		int lightY = pixelInlight.y;
		float zInlight = pixelInlight.z - 0.01f;
		Vector4i shadow = { 100,100,100,1 };
		Vector4i colorRGB;
		if (!tex.buf.empty())
		{
			colorRGB = tex.Map(u, v);
		}
		else colorRGB = shadow;
		Vector4i diffuseColor = colorRGB * diffuse * diffuselight.intensity;
		Vector4i ambientColor = colorRGB + ambientLight.color * ambientLight.intensity;
		if (lightX < shadowWidth && lightY < shadowHeight && lightX >= 0 && lightY >= 0)
		{
			float m = pixelInlight.z - shadowDepthbuffer[lightY][lightX];

			if (zInlight > shadowDepthbuffer[lightY][lightX])
			{
				diffuseColor = diffuseColor * 0.01f;
			}
		}
		Vector4i specularColor = speculaLight.color * specular;
		colorRGB = diffuseColor + ambientColor;// +specularColor;      //省略高光
		color = ConvertRGBTOUINT(colorRGB);
		PutPixel(x, scanline.currentY, z, color);
	}
}

//将深度渲染为深度图
void Device::ProcessScanLineToTexture(ScanLineData scanline, Vector4f& pa, Vector4f& pb, Vector4f& pc, Vector4f& pd)    
{
	float gradient_s = pa.y != pb.y ? (scanline.currentY - pa.y) / (pb.y - pa.y) : 1;
	float gradient_e = pc.y != pd.y ? (scanline.currentY - pc.y) / (pd.y - pc.y) : 1;

	int sx = INTERP(pa.x, pb.x, gradient_s);
	int ex = INTERP(pc.x, pd.x, gradient_e);

	//深度线性插值
	float z1 = INTERP(pa.z, pb.z, gradient_s);
	float z2 = INTERP(pc.z, pd.z, gradient_e);

	int y = scanline.currentY;
	UINT32 color;
	Vector4i shadow = { 255,255,255,1 };
	Vector4i colors;
	for (int x = sx; x <= ex; x++)
	{
		if (ex == sx) continue;
		float gradient = (x - sx) / (float)(ex - sx);
		float z = INTERP(z1, z2, gradient);

		colors = shadow * z;
		color = ConvertRGBTOUINT(colors);
		if (y >= shadowHeight - 5)
		{
			float m = 1;
		}
		if (((UINT32)x) < (UINT32)shadowWidth && ((UINT32)y) < (UINT32)shadowHeight)
		{
			if (z >= shadowDepthbuffer[y][x]) continue;
			shadowDepthbuffer[y][x] = z;
			//framebuffer[y][x] = color;        //画深度图
		}
	}
}

//线框渲染
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

//平面单色着色
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

//平面着色
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

//纹理着色
void Device::DrawTriangleTexture(Vertex A, Vertex B, Vertex C, Texture texture)
{
	if (face == 3)
	{
		float m = 1;
	}

	ScanLineData scanline;

	float temp = float(1) / float(3);
	//normal = (p0.normal + p1.normal + p2.normal) * temp;
	Vector4f edge1 = C.worldCoordinates - A.worldCoordinates;
	Vector4f edge2 = B.worldCoordinates - A.worldCoordinates;
	Vector4f n = edge1 ^ edge2;
	n.normalize();
	scanline.n = n;
	scanline.diffuse = diffuselight.DiffuseLightCos(n);         //计算漫反射在当前面的光照强度

	Vector4f pa = A.coordinates;
	Vector4f pb = B.coordinates;
	Vector4f pc = C.coordinates;

	//按y值大小按从小到大顺序排列
	if (pa.y > pb.y)
	{
		swap(pa, pb); swap(A, B);  //swap(specularA, specularB);
	}
	if (pa.y > pc.y)
	{
		swap(pa, pc); swap(A, C); //swap(specularA, specularC);
	}
	if (pb.y > pc.y)
	{
		swap(pb, pc); swap(B, C); // swap(specularB, specularC);
	}

	if (pa.y == pb.y)
	{
		if (pa.x < pb.x)
		{
			swap(pa, pb);
			swap(A, B);// swap(specularA, specularB);
		}
		for (int row = (int)pa.y; row <= (int)pc.y; row++)
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
			scanline.world_a = B.worldCoordinates;
			scanline.world_b = C.worldCoordinates;
			scanline.world_c = A.worldCoordinates;
			scanline.world_d = C.worldCoordinates;
			threadpool.append(std::bind(ThreadFunc,this, scanline, pb, pc, pa, pc, texture));
			//ProcessScanLineTexture(scanline, pb, pc, pa, pc, texture);  //单线程
		}
		bool hasFinished = false;
		while (hasFinished == false)
		{
			hasFinished = threadpool.finishTask();           //判断是否完成所有任务
		}
		return;
	}

	if (pc.y == pb.y)
	{
		if (pc.x < pb.x)
		{
			swap(pc, pb);
			swap(C, B);
			// swap(specularC, specularB);
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
			scanline.world_a = A.worldCoordinates;
			scanline.world_b = B.worldCoordinates;
			scanline.world_c = C.worldCoordinates;
			scanline.world_d = A.worldCoordinates;
			threadpool.append(std::bind(ThreadFunc, this, scanline, pa, pb, pc, pa, texture));
			//ProcessScanLineTexture(scanline, pa, pb, pc, pa, texture);
		}
		bool hasFinished = false;
		while (hasFinished == false)
		{
			hasFinished = threadpool.finishTask();
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
				scanline.world_a = A.worldCoordinates;
				scanline.world_b = C.worldCoordinates;
				scanline.world_c = B.worldCoordinates;
				scanline.world_d = A.worldCoordinates;
				threadpool.append(std::bind(ThreadFunc, this, scanline, pa, pc, pb, pa, texture));
				//ProcessScanLineTexture(scanline, pa, pc, pb, pa, texture);
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
				scanline.world_a = A.worldCoordinates;
				scanline.world_b = C.worldCoordinates;
				scanline.world_c = B.worldCoordinates;
				scanline.world_d = C.worldCoordinates;
				threadpool.append(std::bind(ThreadFunc, this, scanline, pa, pc, pb, pc, texture));
				//ProcessScanLineTexture(scanline, pa, pc, pb, pc, texture);
			}
		}
		bool hasFinished = false;
		while (hasFinished == false)
		{
			hasFinished = threadpool.finishTask();
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
				scanline.world_a = A.worldCoordinates;
				scanline.world_b = B.worldCoordinates;
				scanline.world_c = C.worldCoordinates;
				scanline.world_d = A.worldCoordinates;
				threadpool.append(std::bind(ThreadFunc, this, scanline, pa, pb, pc, pa, texture));
				//ProcessScanLineTexture(scanline, pa, pb, pc, pa, texture);
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
				scanline.world_a = B.worldCoordinates;
				scanline.world_b = C.worldCoordinates;
				scanline.world_c = A.worldCoordinates;
				scanline.world_d = C.worldCoordinates;
				threadpool.append(std::bind(ThreadFunc, this, scanline, pb, pc, pa, pc, texture));
				//ProcessScanLineTexture(scanline, pb, pc, pa, pc, texture);    
			}
		}
		bool hasFinished = false;
		while (hasFinished == false)
		{
			hasFinished = threadpool.finishTask();
		}

	}
}

//得到深度的texture
void Device::DrawTriangleToTexture(Vertex A, Vertex B, Vertex C)
{
	ScanLineData scanline;

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

	if (pa.y == pb.y)
	{
		if (pa.x < pb.x)
		{
			swap(pa, pb);
			swap(A, B);
		}
		for (int row = (int)pa.y; row <= (int)pc.y; row++)
		{
			scanline.currentY = row;
			ProcessScanLineToTexture(scanline, pb, pc, pa, pc);
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
			ProcessScanLineToTexture(scanline, pa, pb, pc, pa);
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
				ProcessScanLineToTexture(scanline, pa, pc, pb, pa);
			}
			else
			{
				ProcessScanLineToTexture(scanline, pa, pc, pb, pc);
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
				ProcessScanLineToTexture(scanline, pa, pb, pc, pa);
			}
			else
			{
				ProcessScanLineToTexture(scanline, pb, pc, pa, pc);
			}
		}
	}
}

void Device::RenderToShadowTexture(vector<Model> models)
{
	Vertex re2, re3, re4;

	for (int j = 0; j < models.size(); j++)
	{
		Model model = models[j];
		lightTransform.world = Matrix::RotateMatrix(model.rotation.x, model.rotation.y, model.rotation.z, model.rotation.w);
		lightTransform.world = Matrix::TranslateMatrix(model.Position().x, model.Position().y, model.Position().z, lightTransform.world);
		lightTransform.LightUpdate();
		for (int i = 0; i < model.nfaces(); i++)
		{
			lightTransform.Apply(model.vertices[model.faces[i][0][0]], re2, model.getUV(i, 0));
			lightTransform.ShadowHomogenize(re2, re2, shadowWidth, shadowHeight);
			lightTransform.Apply(model.vertices[model.faces[i][1][0]], re3, model.getUV(i, 1));
			lightTransform.ShadowHomogenize(re3, re3, shadowWidth, shadowHeight);
			lightTransform.Apply(model.vertices[model.faces[i][2][0]], re4, model.getUV(i, 2));
			lightTransform.ShadowHomogenize(re4, re4, shadowWidth, shadowHeight);
			DrawTriangleToTexture(re2, re3, re4);
		}
	}
}

void Device::Render(vector<Model> models, int op)
{
	transform.Update();
	lightTransform.LightUpdate();

	ClearShadowBuf();
	Vertex re2, re3, re4;
	int count_backface = 0;
	Clear(0);
	RenderToShadowTexture(models);
	//Clear(0);
	for (int j = 0; j < models.size(); j++)
	{
		modelNum = j;
		Model model = models[j];
		transform.world = Matrix::RotateMatrix(model.rotation.x, model.rotation.y, model.rotation.z, model.rotation.w);
		transform.world = Matrix::TranslateMatrix(model.Position().x, model.Position().y, model.Position().z, transform.world);
		transform.Update();
		for (int i = 0; i < model.nfaces(); i++)
		{
			re2 = model.vertices[model.faces[i][0][0]];
			transform.Apply(model.vertices[model.faces[i][0][0]], re2, model.getUV(i, 0));
			transform.Homogenize(re2, re2);
			transform.Apply(model.vertices[model.faces[i][1][0]], re3, model.getUV(i, 1));
			transform.Homogenize(re3, re3);
			transform.Apply(model.vertices[model.faces[i][2][0]], re4, model.getUV(i, 2));
			transform.Homogenize(re4, re4);
			if (!BackfaceCulling(re2, re3, re4, model.normals[i / 2]))
			{
				continue;
			}
			switch (op)
			{
				//case 0:DrawTriangleFrame(re2, re3, re4, color[i / 2]); break;
				//case 1:DrawTriangleFlat(re2, re3, re4, color[i / 2]); break;
			case 2:DrawTriangleFlat(re2, re3, re4); break;
			case 3:DrawTriangleTexture(re2, re3, re4, model.texture); break;
			}
		}
	}
}
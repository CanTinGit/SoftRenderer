#include <Windows.h>
#include <tchar.h>
#include "geometry.h"
#include "camera.h"
#include "mymath.h"
#include "model.h"
#include <vector>
#include "render.h"

using namespace std;
/////////////////////////
//创建windows窗口////////
/////////////////////////
int screen_width, screen_height, screen_exit = 0;
int screen_mx = 0, screen_my = 0, screen_mb = 0;
int screen_keys[512]; //当前键盘按下状态
static HWND screen_handle = NULL;  //窗口句柄,类似身份证号,通过此即可找到窗口类，窗口指针
static HDC screen_dc = NULL;
static HBITMAP screen_hb = NULL;
static HBITMAP screen_ob = NULL;   //老的Bitmap
unsigned char *screen_fb = NULL;   //Frame buffer
long screen_pitch = 0;

int Screen_Init(int width, int height, const TCHAR *title);
int Screen_Close(void);
void Screen_Dispatch(void);
void Screen_update(void);
void ShowFPS();

// win32 event handler
static LRESULT Screen_Events(HWND, UINT, WPARAM, LPARAM);

#ifdef _MSC_VER
#pragma comment(lib, "gdi32.lib")  //链接库
#pragma comment(lib, "user32.lib")
#endif

#define KEY_DOWN(vk_code)((GetAsyncKeyState(vk_code) & 0x8000) ? 1:0 )

//窗口初始化
int Screen_Init(int width, int height, const TCHAR *title)
{
	WNDCLASS window_screen = { CS_BYTEALIGNCLIENT,(WNDPROC)Screen_Events,0,0,0,
								NULL,NULL,NULL,NULL,_T("TinyRenderer") };
	BITMAPINFO bi = { {sizeof(BITMAPINFOHEADER),width,-height,1,32,BI_RGB,
					width*height * 4,0,0,0,0} };
	RECT rect = { 0,0,width,height };
	int wx, wy, sx, sy;
	LPVOID ptr;
	HDC hdc;

	Screen_Close();

	window_screen.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	window_screen.hInstance = GetModuleHandle(NULL);
	window_screen.hCursor = LoadCursor(NULL, IDC_ARROW);
	if (!RegisterClass(&window_screen)) return -1;

	screen_handle = CreateWindow(_T("TinyRenderer"), title,
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		0, 0, 0, 0, NULL, NULL, window_screen.hInstance, NULL);
	if (screen_handle == NULL) return -2;

	screen_exit = 0;
	hdc = GetDC(screen_handle);
	screen_dc = CreateCompatibleDC(hdc);
	ReleaseDC(screen_handle, hdc);

	screen_hb = CreateDIBSection(screen_dc, &bi, DIB_RGB_COLORS, &ptr, 0, 0);
	if (screen_hb == NULL) return -3;

	screen_ob = (HBITMAP)SelectObject(screen_dc, screen_hb);
	screen_fb = (unsigned char*)ptr;
	screen_width = width;
	screen_height = height;
	screen_pitch = width * 4;

	AdjustWindowRect(&rect, GetWindowLong(screen_handle, GWL_STYLE), 0);
	wx = rect.right - rect.left;
	wy = rect.bottom - rect.top;
	sx = (GetSystemMetrics(SM_CXSCREEN) - wx) / 2;
	sy = (GetSystemMetrics(SM_CYSCREEN) - wy) / 2;
	if (sy < 0) sy = 0;
	SetWindowPos(screen_handle, NULL, sx, sy, wx, wy, (SWP_NOCOPYBITS | SWP_NOZORDER | SWP_SHOWWINDOW));
	SetForegroundWindow(screen_handle);

	ShowWindow(screen_handle, SW_NORMAL);
	Screen_Dispatch();

	memset(screen_keys, 0, sizeof(int) * 512);
	memset(screen_fb, 0, width*height * 4);

	return 0;
}

int Screen_Close(void)
{
	if (screen_dc)
	{
		if (screen_ob)
		{
			SelectObject(screen_dc, screen_ob);
			screen_ob = NULL;
		}
		DeleteDC(screen_dc);
		screen_dc = NULL;
	}

	if (screen_hb)
	{
		DeleteObject(screen_hb);
		screen_hb = NULL;
	}

	if (screen_handle)
	{
		CloseWindow(screen_handle);
		screen_handle = NULL;
	}
	return 0;
}

// win32 event handler
// TODO: Click once
static LRESULT Screen_Events(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CLOSE: screen_exit = 1; break;
	case WM_KEYDOWN: screen_keys[wParam & 511] = 1; break;
	case WM_KEYUP: screen_keys[wParam & 511] = 0; break;
	default: return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

void Screen_Dispatch(void)
{
	MSG msg;
	while (1)
	{
		if (!PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) break;
		if (!GetMessage(&msg, NULL, 0, 0)) break;
		DispatchMessage(&msg);
	}
}
void Screen_update(void)
{
	HDC hdc = GetDC(screen_handle);
	BitBlt(hdc, 0, 0, screen_width, screen_height, screen_dc, 0, 0, SRCCOPY);
	ReleaseDC(screen_handle, hdc);
	Screen_Dispatch();
}

void ShowFPS()
{
	static float fps = 0;
	static int frameCount = 0;
	static float currentTime = 0.0f;
	static float lastTime = 0.0f;

	frameCount++;
	currentTime = timeGetTime()*0.001f;
	if (currentTime - lastTime > 1.0f)
	{
		fps = (float)frameCount / (currentTime - lastTime);
		lastTime = currentTime;
		frameCount = 0;
	}

	char strBuffer[20];
	HDC hdc = GetDC(screen_handle);
	sprintf_s(strBuffer, 20, "%0.3f", fps);
	TextOut(screen_dc, 0, 0, strBuffer, 6);
}

int main()
{
	//Mesh mesh(8, 12);
	//set_mesh_vertices_faces(mesh, all_verticesJson, facesJson);

	//mesh.Rotation = { 0, 0, 0, 0 };
	vector<Model> models;
	const TCHAR *title = _T("TinyRender");
	if (Screen_Init(800, 600, title))
		return -1;

	Model *model = new Model("Resources/cube.obj");
	static float delta = 0;
	model->SetRotation(0, 1, 0, 0.5f);
	model->SetPosition(0, -1, 0);
	model->texture.Load("Resources/cube.jpg");
	models.push_back(*model);

	Model *model2 = new Model("Resources/cube.obj");
	model2->SetRotation(0, 1, 0, 0.0f);
	model2->SetPosition(0, -2, 0);
	//model2->texture.Load("Resources/cube.jpg");
	models.push_back(*model2);
	Matrix m = Matrix::ScaleMatrix(5, 0.1f, 5);
	for (int i = 0; i < 8; i++)
	{
		models[1].vertices[i].local = m * models[1].vertices[i].local;
	}

	Vector4f look_at(0, 0, 0, 1), up = { 0,1,0,1 };
	Device my_device(screen_width, screen_height, screen_fb, 800, 600);

	my_device.diffuselight.SetDiffuseLightDir(1, 1, -1);
	my_device.diffuselight.SetIntensity(1.0f);
	my_device.ambientLight.SetColor(0, 0, 0);
	my_device.ambientLight.SetIntensity(0.05f);
	my_device.speculaLight.SetPosition(-1.f, 0, -1.0f);
	my_device.speculaLight.SetColor(255, 0, 0);
	my_device.specularPower = 5.f;

	//my_device.my_camera.SetPosition(100, 100, -100);
	//my_device.my_camera.SetCamera(look_at, up);
	//my_device.lightTransform.view = my_device.my_camera.view;

	//my_device.lightTransform.Set_Ortho(8, 6, 1, 500.0f);
	my_device.SetupShadowCamera(models);

	my_device.my_camera.SetPosition(2, 2, -2);
	my_device.my_camera.SetCamera(look_at, up);

	my_device.transform.view = my_device.my_camera.view;

	float aspect = float(800) / ((float)600);
	float fovy = PI * 0.5f;
	my_device.transform.Set_Perspective(fovy, aspect, 1.0f, 500.0f);
	my_device.transform.Update();

	my_device.Clear(0);
	float theta = 0;
	int op = 3;
	while (screen_exit == 0 && screen_keys[VK_ESCAPE] == 0)
	{
		Screen_Dispatch();
		my_device.Clear(0);
		my_device.my_camera.SetCamera(look_at, up);
		my_device.transform.view = my_device.my_camera.view;
		my_device.Render(models, op);
		//model->rotation.w += 0.01f;
		if (screen_keys[VK_UP]) my_device.my_camera.position.z += 0.01f;
		if (screen_keys[VK_DOWN]) my_device.my_camera.position.z -= 0.01f;

		if (screen_keys[VK_LEFT]) my_device.my_camera.position.x -= 0.01f;
		if (screen_keys[VK_RIGHT]) my_device.my_camera.position.x += 0.01f;

		if (screen_keys[VK_PRIOR]) {
			models[0].rotation.w -= 0.01f; cout << models[0].rotation.w << endl;
		}
		if (screen_keys[VK_NEXT])  models[0].rotation.w += 0.01f;

		if (screen_keys[VK_NUMPAD4]) model->rotation.w += 0.01f;
		if (screen_keys[VK_NUMPAD5]) model->rotation.w -= 0.01f;

		if (screen_keys[VK_NUMPAD1]) my_device.specularPower += 0.1f;	//E
		if (screen_keys[VK_NUMPAD2]) my_device.specularPower -= 0.1f;	//F

		if (screen_keys[VK_SPACE])
		{
			op++;
			op %= 4;
		}

		ShowFPS();
		Screen_update();
		Sleep(1);
	}
}
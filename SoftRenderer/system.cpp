#include<Windows.h>
#include <tchar.h>
#include "geometry.h"
#include "camera.h"
#include "mymath.h"
#include "model.h"
#include <vector>

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

	screen_hb = CreateDIBSection(screen_dc,&bi,DIB_RGB_COLORS,&ptr,0,0);
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
static LRESULT Screen_Events(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	switch (msg)
	{
	case WM_PAINT: {hdc = BeginPaint(hwnd, &ps); EndPaint(hwnd, &ps); return 0; }break;
		case WM_CLOSE: screen_exit = 1; break;
		case WM_KEYDOWN: screen_keys[wParam & 511] = 1; break;
		case WM_KEYUP: screen_keys[wParam && 511] = 0; break;
		case WM_DESTROY: {PostQuitMessage(0); return 0; } break;
		default: return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

void Screen_Dispatch(void) 
{
	MSG msg;
	while(1)
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

int main()
{
	const TCHAR *title = _T("TinyRender");
	if (Screen_Init(800, 600, title))
		return -1;

	Model model("obj/cube.obj");
	Camera my_camera(0, 0, -3);
	Camera my();
	

	//将模型坐标转换为世界坐标
	vector<Vector3f> model_world_coords;
	for (int i = 0; i < model.nverts(); i++)
	{
		Vector3f v = Translate(model.vert[i], model.SetPosition);        //根据模型中心,将其顶点平移至中心左右
		model_world_coords.push_back(v);
	}

	//包围球测试
	SphereTest(model, 100, 10);

	//背面剔除
	for (int i = 0; i < model.nfaces(); i++)
	{
		vector<int> face = model.face(i);
		vector<Vector3f> ver_face;   
		for (int j = 0; j < 3; j++)
		{
			ver_face.push_back(model.vert(face[j]));            //存储每个三角形的三个顶点,用于计算面法线
		}

		Vector3f n = (ver_face[2] - ver_face[0]) ^ (ver_face[1] - ver_face[0]);  //三角形两条边叉乘获得面法线
		//待实现: 面法线点乘相机观察向量来检测当前面是否可见

	}

	//将世界坐标转换为相机坐标
	vector<Vector3f> model_camera_coords;
	vector<Vector3f> model_camera_coords_justFinishTranslating;
	//顶点平移
	for (int i = 0; i < model.nverts(); i++)	
	{
		Vector3f v = Translate(model_world_coords[i], Vector3f(-my_camera.GetPosition().x, -my_camera.GetPosition().y, -my_camera.GetPosition().z));        //将相机平移到世界坐标原点后,同样要对所有模型进行相同的平移
		model_camera_coords_justFinishTranslating.push_back(v);
	}
	//待实现:顶点旋转

	//投影变换
	vector<Vector3f> viewPort;                 //转换后可忽视z坐标
	for (int i = 0; i < model_camera_coords.size(); i++)
	{	
		Vector3f v = TransformToViewPort(screen_width, screen_height, model_camera_coords[i]);
		viewPort.push_back(v);
	}

	//屏幕坐标变换
	vector<Vector2f> screen;                
	for (int i = 0; i < viewPort.size(); i++)
	{
		Vector2f v = TransformToScreen(viewPort[i],screen_width,screen_height);
		screen.push_back(v);
	}


	while (TRUE) {
		
		Screen_update();
		if (KEY_DOWN(VK_ESCAPE)) PostMessage(screen_handle, WM_DESTROY, 0, 0);
	}

}


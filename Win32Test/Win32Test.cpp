// Win32Test.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "Win32Test.h"

#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

#include <windows.h>





LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

LRESULT CALLBACK WndProc2(HWND, UINT, WPARAM, LPARAM);



static int wndCount = 0;



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,

	LPSTR lpCmdLine, int nCmdShow)

{

	WNDCLASS wndclass, wndclass2;

	HWND hwnd, hwnd2, hwnd3, hwnd4;

	MSG msg;





	wndclass.style = CS_HREDRAW | CS_VREDRAW;

	wndclass.lpfnWndProc = WndProc;

	wndclass.cbClsExtra = 0;

	wndclass.cbWndExtra = 0;

	wndclass.hInstance = hInstance;

	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);

	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);

	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);

	wndclass.lpszMenuName = NULL;

	wndclass.lpszClassName = L"HelloClass";





	wndclass2 = wndclass;

	wndclass2.lpszClassName = L"h2";

	wndclass2.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);

	wndclass2.lpfnWndProc = WndProc2;







	if (!RegisterClass(&wndclass)) return 1;

	if (!RegisterClass(&wndclass2)) return 1;







	hwnd = CreateWindowEx(0, L"HelloClass", L"h1", WS_OVERLAPPEDWINDOW,

		0, 0, 500, 500,

		NULL, NULL, hInstance, NULL);



	hwnd2 = CreateWindowEx(0, L"HelloClass", L"h2", WS_OVERLAPPEDWINDOW,

		500, 0, 500, 500,

		NULL, NULL, hInstance, NULL);



	hwnd3 = CreateWindowEx(0, L"h2", L"h3", WS_OVERLAPPEDWINDOW,

		0, 500, 500, 500,

		NULL, NULL, hInstance, NULL);



	hwnd4 = CreateWindowEx(0, L"h2", L"h4", WS_OVERLAPPEDWINDOW,

		500, 500, 500, 500,

		NULL, NULL, hInstance, NULL);





	ShowWindow(hwnd, nCmdShow);

	ShowWindow(hwnd2, nCmdShow);

	ShowWindow(hwnd3, nCmdShow);

	ShowWindow(hwnd4, nCmdShow);





	while (GetMessage(&msg, NULL, 0, 0) > 0) {

		TranslateMessage(&msg);

		DispatchMessage(&msg);

	}



	return msg.wParam;

}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message,

	WPARAM wParam, LPARAM lParam)

{

	HDC hdc;

	PAINTSTRUCT ps;

	LPCWSTR str = L"Hello, SDK";



	switch (message) {

	case WM_CREATE:

		wndCount++;

		return 0;



	case WM_LBUTTONDOWN:

		MessageBox(hwnd, L"마우스를클릭했습니다.", L"마우스메시지", MB_OK);

		return 0;



	case WM_PAINT:

		hdc = BeginPaint(hwnd, &ps);

		TextOut(hdc, 100, 100, str, lstrlen(str));

		EndPaint(hwnd, &ps);

		return 0;



	case WM_DESTROY:



		wndCount--;

		if (wndCount == 0) {

			PostQuitMessage(0);

		}

		return 0;

	}



	return DefWindowProc(hwnd, message, wParam, lParam);

}





LRESULT CALLBACK WndProc2(HWND hwnd, UINT message,

	WPARAM wParam, LPARAM lParam)

{

	HDC hdc;

	PAINTSTRUCT ps;

	LPCWSTR str = L"Hello, SDK";



	switch (message) {

	case WM_CREATE:

		wndCount++;

		return 0;



	case WM_LBUTTONDOWN:

		MessageBox(hwnd, L"마우스를클릭했습니다.", L"마우스메시지", MB_OK);

		return 0;



	case WM_PAINT:

		hdc = BeginPaint(hwnd, &ps);

		TextOut(hdc, 100, 100, str, lstrlen(str));

		EndPaint(hwnd, &ps);

		return 0;



	case WM_DESTROY:



		wndCount--;

		if (wndCount == 0) {

			PostQuitMessage(0);

		}

		return 0;

	}



	return DefWindowProc(hwnd, message, wParam, lParam);

}

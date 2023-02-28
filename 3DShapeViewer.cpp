#define _CRT_SECURE_NO_WARNINGS

#include "framework.h"
#include "3DShapeViewer.h"

using namespace std;

#define GLEW_STATIC
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <windows.h>
#include <conio.h>

// Forward declarations
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
bool InitWindow(HINSTANCE hInstance, int nCmdShow);
bool InitOpenGL();
void Render();

HWND hWnd;

// Global variables
EGLDisplay eglDisplay = nullptr;
EGLContext eglContext = nullptr;
EGLSurface eglSurface = nullptr;
GLuint programObject = 0;

void Render()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Draw a triangle
    GLfloat vertices[] = {
        0.0f,  0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f
    };
    GLuint vertexBuffer;
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDisableVertexAttribArray(0);

    // Swap buffers
    eglSwapBuffers(eglDisplay, eglSurface);
}

// Function to destroy the OpenGL ES context
void DestroyContext()
{
    eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(eglDisplay, eglContext);
    eglDestroySurface(eglDisplay, eglSurface);
    eglTerminate(eglDisplay);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
	{
		InitOpenGL();

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glEnable(GL_DEPTH_TEST);

		break;
	}

	case WM_SIZE:
	{
		glViewport(0, 0, LOWORD(lParam), HIWORD(lParam));
		break;
	}

	case WM_PAINT:
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Render();

		eglSwapBuffers(eglDisplay, eglSurface);

		ValidateRect(hWnd, nullptr);
		break;
	}

	case WM_DESTROY:
	{
		eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

		eglDestroySurface(eglDisplay, eglSurface);
		eglDestroyContext(eglDisplay, eglContext);
		eglTerminate(eglDisplay);

		PostQuitMessage(0);
		break;
	}

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    AllocConsole();
    OutputDebugStringW(L"Hello");

    cout << "TEST" << endl;

    // Initialize the window
    if (!InitWindow(hInstance, nCmdShow)) {
        return 1;
    }

    // Initialize OpenGL
    if (!InitOpenGL()) {
        return 1;
    }

    cout << "TEST" << endl;

    // Main message loop
    MSG msg = { 0 };
    while (msg.message != WM_QUIT) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    cout << "TEST" << endl;

    FreeConsole();

    return static_cast<int>(msg.wParam);
}

bool InitWindow(HINSTANCE hInstance, int nCmdShow)
{
    const wchar_t CLASS_NAME[] = L"OpenGLWin32App";
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClass(&wc);

    hWnd = CreateWindowW(
        CLASS_NAME,
        CLASS_NAME,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        nullptr, nullptr, hInstance, nullptr);
    if (!hWnd) {
        return false;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return true;
}

bool InitOpenGL()
{
    // Create an EGL display connection
    eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (eglDisplay == EGL_NO_DISPLAY) {
        return false;
    }

    // Initialize the EGL display connection
    EGLint majorVersion, minorVersion;
    if (!eglInitialize(eglDisplay, &majorVersion, &minorVersion)) {
        return false;
    }

    // Choose an EGL configuration
    EGLConfig eglConfig;
    EGLint numConfigs;
    const EGLint configAttribs[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 16,
        EGL_STENCIL_SIZE, 0,
        EGL_NONE
    };

    eglChooseConfig(eglDisplay, configAttribs, &eglConfig, 1, &numConfigs);
    eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, hWnd, NULL);
    eglContext = eglCreateContext(eglDisplay, eglConfig, EGL_NO_CONTEXT, NULL);
    EGLBoolean result = eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);

    return result;
}
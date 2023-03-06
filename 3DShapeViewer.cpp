#define _CRT_SECURE_NO_WARNINGS

#include "shapefil.h"
#include "framework.h"
#include "3DShapeViewer.h"
#include <locale>
#include <string>
#include <codecvt>
#include <iostream>
#include <vector>

using namespace std;

EGLint EGL_OPENGL_ES3_BIT_KHR = 0x0040;

// Window dimensions
const int WINDOW_WIDTH = 640;
const int WINDOW_HEIGHT = 480;

// OpenGL context and window handle
EGLDisplay eglDisplay;
EGLSurface eglSurface;
EGLContext eglContext;
HWND hWnd;
HINSTANCE hInst;
HANDLE hConsole;

// Vertex shader source code
const char* vertexShaderSource =
"#version 300 es\n"
"layout(location = 0) in vec4 a_position;\n"
"void main() {\n"
"  gl_Position = a_position;\n"
"}\n";

// Fragment shader source code
const char* fragmentShaderSource =
"#version 300 es\n"
"precision mediump float;\n"
"out vec4 fragColor;\n"
"void main() {\n"
"  fragColor = vec4(1.0, 1.0, 0.0, 1.0);\n"
"}\n";

// Vertex data
const GLfloat vertexData[] = {
    0.0f, 0.5f, 0.0f,
    -0.5f, -0.5f, 0.0f,
    0.5f, -0.5f, 0.0f
};

// Vertex buffer object and attribute location
GLuint vbo;
GLint positionAttr;

// Initialize OpenGL context and window
void initializeOpenGL()
{
    // Initialize EGL
    EGLint numConfigs;
    EGLint majorVersion;
    EGLint minorVersion;
    EGLConfig eglConfig;
    EGLint attribs[] = {
    EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
    EGL_RED_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_BLUE_SIZE, 8,
    EGL_ALPHA_SIZE, 8,
    EGL_DEPTH_SIZE, 24,
    EGL_STENCIL_SIZE, 8,
    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR, // OpenGL ES 3.0 컨텍스트 요청
    EGL_NONE
    };
    eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(eglDisplay, &majorVersion, &minorVersion);
    eglChooseConfig(eglDisplay, attribs, &eglConfig, 1, &numConfigs);
    eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, hWnd, NULL);
    EGLint contextAttribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 3,
        EGL_NONE
    };
    eglContext = eglCreateContext(eglDisplay, eglConfig, EGL_NO_CONTEXT, contextAttribs);
    eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);

    // Compile and link shaders
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    glUseProgram(program);

    // Create and bind vertex buffer object
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

    // Get position attribute location and enable vertex attribute array
    positionAttr = glGetAttribLocation(program, "a_position");
    glEnableVertexAttribArray(positionAttr);

    // Set vertex attribute pointer
    glVertexAttribPointer(positionAttr, 3, GL_FLOAT, GL_FALSE, 0, 0);
}

// Clean up OpenGL context and window
void cleanupOpenGL()
{
    // Delete vertex buffer object
    glDeleteBuffers(1, &vbo);
    // Delete program and shaders
    GLint program;
    glGetIntegerv(GL_CURRENT_PROGRAM, &program);
    GLuint vertexShader;
    glGetAttachedShaders(program, 1, NULL, &vertexShader);
    GLuint fragmentShader;
    glGetAttachedShaders(program, 1, NULL, &fragmentShader);
    glDetachShader(program, vertexShader);
    glDetachShader(program, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glDeleteProgram(program);
    // Terminate EGL
    eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(eglDisplay, eglContext);
    eglDestroySurface(eglDisplay, eglSurface);
    eglTerminate(eglDisplay);
}


SHPHandle hSHP;
SHPObject* psShape;
bool isShapeLoaded = false;
TCHAR szFileName[MAX_PATH];
int targetIdx = 0;

// Render OpenGL scene
void drawOpenGL()
{
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void openShapefile() {
    OPENFILENAME ofn;

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFilter = L"Shapefiles (*.shp)\0*.shp\0All Files (*.*)\0*.*\0";
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = L"shp";

	if (GetOpenFileName(&ofn) == NULL) return;

	//hSHP = SHPOpen(ConvertWideCharToChar(ofn.lpstrFile).c_str(), "rb");
	wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;
	string str = converter.to_bytes(szFileName);

	hSHP = SHPOpen(converter.to_bytes(szFileName).c_str(), "rb");

	if (hSHP == NULL) return;

	int maxIdx = 0;
	int maxVert = 0;
	int nShapeCount;
	SHPGetInfo(hSHP, &nShapeCount, NULL, NULL, NULL);

	vector<double> xAvr;
	vector<double> yAvr;

	double xMin = DBL_MAX;
	double xMax = DBL_MIN;
	double yMin = DBL_MAX;
	double yMax = DBL_MIN;

	for (size_t i = 0; i < nShapeCount; ++i) {
		SHPObject* psShape = SHPReadObject(hSHP, i);

		double x = 0;
		double y = 0;

		for (int i = 0; i < psShape->nVertices; i++) {
			x += psShape->padfX[i];
			y += psShape->padfY[i];
		}
		x /= psShape->nVertices;
		y /= psShape->nVertices;

		xMin = min(xMin, x);
		yMin = min(yMin, y);
		xMax = max(xMax, x);
		yMax = max(yMax, y);

		xAvr.push_back(x);
		yAvr.push_back(y);

		SHPDestroyObject(psShape);
	}

	double xRat = xMax - xMin;
	double yRat = yMax - yMin;
	double rat = xRat / yRat;

	psShape = SHPReadObject(hSHP, targetIdx);

	//std::cout << "nSHPType: " << psShape->nSHPType << std::endl;
	//std::cout << "nShapeId: " << psShape->nShapeId << std::endl;
	//std::cout << "nParts: " << psShape->nParts << std::endl;
	//std::cout << "nVertices: " << psShape->nVertices << std::endl;

	double dfx[4];
	dfx[0] = psShape->dfXMin; dfx[1] = psShape->dfXMax;
	dfx[2] = psShape->dfYMin; dfx[3] = psShape->dfYMax;

	isShapeLoaded = true;
}

void cleanupShapefile() {
    if (isShapeLoaded) SHPClose(hSHP);
}

std::string ConvertWideCharToChar(const wchar_t* wideCharString)
{
    int bufferSize = WideCharToMultiByte(CP_UTF8, 0, wideCharString, -1, nullptr, 0, nullptr, nullptr);

    std::string result(100, 0);
    WideCharToMultiByte(CP_UTF8, 0, wideCharString, -1, &result[0], bufferSize, nullptr, nullptr);

    return result;
}

// Window procedure
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        AllocConsole();
        hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        break;
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // 메뉴 선택을 구문 분석합니다:
        switch (wmId)
        {
        case IDM_OPEN:
            openShapefile();
			break;
		case IDM_EXIT:
			cleanupShapefile();
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
    }
    break;
    case WM_DESTROY:
        cleanupShapefile();
        PostQuitMessage(0);
        break;
    case WM_PAINT:
        glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        if (isShapeLoaded) drawOpenGL();
        eglSwapBuffers(eglDisplay, eglSurface);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // Register window class
    WNDCLASSEX wcex = {};
    wcex.cbSize = sizeof(wcex);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WindowProc;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDC_MY3DSHAPEVIEWER));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_MY3DSHAPEVIEWER);
    wcex.lpszClassName = L"OpenGLWindowClass";
    RegisterClassEx(&wcex);
    // Create window
    hWnd = CreateWindowEx(0, L"OpenGLWindowClass", L"OpenGL ES 3.0 Window", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH, WINDOW_HEIGHT,
        NULL, NULL, hInstance, NULL);
    hInst = hInstance;
    ShowWindow(hWnd, nCmdShow);

    // Initialize OpenGL
    initializeOpenGL();

    // Enter message loop
    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MY3DSHAPEVIEWER));
    MSG msg = {};

    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    // Clean up OpenGL
    cleanupOpenGL();

    FreeConsole();

    return static_cast<int>(msg.wParam);
}
#include "3DShapeViewer.h"

#include <stdarg.h>
#include <CommCtrl.h>

using namespace std;

EGLint EGL_OPENGL_ES3_BIT_KHR = 0x0040;

const int WINDOW_WIDTH = 1316;
const int WINDOW_HEIGHT = 1000;

EGLDisplay eglDisplay;
EGLSurface eglSurface;
EGLContext eglContext;
HWND hWnd;
HINSTANCE hInst;
TCHAR szFileName[MAX_PATH];

GLuint vbo;
GLuint program;

SHPHandle hSHP;
SHPObject* psShape;
bool isShapeLoaded = false;

float cameraX = 0.0f;
float cameraY = 0.0f;
const float delta = 0.02f;

vector<float> vertices;
vector<int> objectVertices;



bool checkShaderCompileStatus(GLuint shader)
{
	GLint compileStatus;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
	if (compileStatus == GL_FALSE)
	{
		GLint infoLogLength;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
		vector<char> infoLog(infoLogLength);
		glGetShaderInfoLog(shader, infoLogLength, nullptr, infoLog.data());
		cerr << "Shader compile error: " << infoLog.data() << endl;
		return false;
	}
	std::cout << "Shader compile complite" << endl;
	return true;
}

bool compileShader(GLuint shader, const char* source)
{
	glShaderSource(shader, 1, &source, nullptr);
	glCompileShader(shader);
	return checkShaderCompileStatus(shader);
}



bool initialize()
{
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
	EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
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


	string shaderSource = readShader("source.vert");
	const char* shaderCstr = shaderSource.c_str();

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	if (!compileShader(vertexShader, shaderCstr)) {
		return false;
	}

	shaderSource = readShader("source.frag");
	shaderCstr = shaderSource.c_str();

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	if (!compileShader(fragmentShader, shaderCstr)) {
		return false;
	}

	program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);
	glUseProgram(program);

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

	return true;
}

void render()
{
	glm::vec3 cameraPosition = glm::vec3(cameraX, cameraY, 1.0f);
	glm::vec3 cameraTarget = glm::vec3(cameraX, cameraY, 0.0f);
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

	glm::mat4 viewMatrix = glm::lookAt(cameraPosition, cameraTarget, cameraUp);

	glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE, glm::value_ptr(viewMatrix));

	glClear(GL_COLOR_BUFFER_BIT);
	for (int i = 0, startIndex = 0; i < objectVertices.size(); ++i) {
		glDrawArrays(GL_LINE_STRIP, startIndex, objectVertices[i]);
		startIndex += objectVertices[i];
	}
}

void cleanUp()
{
	glDeleteBuffers(1, &vbo);

	GLuint vertexShader;
	glGetAttachedShaders(program, 1, NULL, &vertexShader);
	GLuint fragmentShader;
	glGetAttachedShaders(program, 1, NULL, &fragmentShader);
	glDetachShader(program, vertexShader);
	glDetachShader(program, fragmentShader);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	glDeleteProgram(program);

	eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	eglDestroyContext(eglDisplay, eglContext);
	eglDestroySurface(eglDisplay, eglSurface);
	eglTerminate(eglDisplay);
}



string readShader(const string& filepath) {
	ifstream file(filepath);
	if (!file.is_open()) {
		return "";
	}

	string shader_code;
	string line;
	while (getline(file, line)) {
		shader_code += line + "\n";
	}

	return shader_code;
}

bool openShapefile() {
	OPENFILENAME ofn;

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFilter = L"Shapefiles (*.shp)\0*.shp\0All Files (*.*)\0*.*\0";
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = L"shp";

	if (GetOpenFileName(&ofn) == false) return false;

	//hSHP = SHPOpen(ConvertWideCharToChar(ofn.lpstrFile).c_str(), "rb");
	wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;
	hSHP = SHPOpen(converter.to_bytes(szFileName).c_str(), "rb");

	if (hSHP == nullptr) return false;

	float xMin = FLT_MAX;
	float xMax = FLT_MIN;
	float yMin = FLT_MAX;
	float yMax = FLT_MIN;

	readShapefile(xMin, xMax, yMin, yMax);

	float xDel = (xMax - xMin) / 2.0f;
	float yDel = (yMax - yMin) / 2.0f;

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

	glGetAttribLocation(program, "a_position");
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

	GLfloat min[] = { xMin, yMin };
	GLuint offsetLoc = glGetUniformLocation(program, "minimum");
	glUniform2fv(offsetLoc, 1, min);

	GLfloat del[] = { xDel, yDel };
	GLuint delLoc = glGetUniformLocation(program, "delta");
	glUniform2fv(delLoc, 1, del);

	isShapeLoaded = true;

	return true;
}


void closeShapefile() {
	if (isShapeLoaded) SHPClose(hSHP);
}

string ConvertWideCharToChar(const wchar_t* wideCharString)
{
	int bufferSize = WideCharToMultiByte(CP_UTF8, 0, wideCharString, -1, nullptr, 0, nullptr, nullptr);

	string result(100, 0);
	WideCharToMultiByte(CP_UTF8, 0, wideCharString, -1, &result[0], bufferSize, nullptr, nullptr);

	return result;
}



LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		switch (wmId)
		{
		case IDM_OPEN:
			openShapefile();
			break;
		case IDM_EXIT:
			closeShapefile();
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_LEFT:
			cameraX -= delta;
			break;
		case VK_RIGHT:
			cameraX += delta;
			break;
		case VK_UP:
			cameraY += delta;
			break;
		case VK_DOWN:
			cameraY -= delta;
			break;
		}
		break;
	case WM_DESTROY:
		closeShapefile();
		PostQuitMessage(0);
		break;
	case WM_PAINT:
		glClear(GL_COLOR_BUFFER_BIT);
		if (isShapeLoaded) render();
		eglSwapBuffers(eglDisplay, eglSurface);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

#include <bitset>

void readShapefile(float& xMin, float& xMax, float& yMin, float& yMax) {
	int nShapeCount;
	SHPGetInfo(hSHP, &nShapeCount, NULL, NULL, NULL);
	cout << hSHP->nFileSize << endl;
	for (size_t i = 0; i < 2; ++i) {
		printf("%0.16f\n", hSHP->adBoundsMin[i]);
	}
	for (size_t i = 0; i < 2; ++i) {
		printf("%0.16f\n", hSHP->adBoundsMax[i]);
	}
	cout << bitset<64>(hSHP->adBoundsMin[0]) << endl;
	cout << bitset<64>(hSHP->adBoundsMin[1]) << endl;
	cout << bitset<64>(hSHP->adBoundsMax[0]) << endl;
	cout << bitset<64>(hSHP->adBoundsMax[1]) << endl;

	RECT rt;
	GetClientRect(hWnd, &rt);
	int progressWidth = 440;
	int progressHeight = 30;

	HWND hWndProgress = CreateWindowEx(0,
		PROGRESS_CLASS, L"PROGRESS", WS_VISIBLE | WS_CHILD,
		(rt.right - progressWidth) / 2,
		(rt.bottom - progressHeight) / 2,
		progressWidth,
		progressHeight, hWnd, (HMENU)401, hInst, NULL);
	if (hWndProgress == NULL) {
		wchar_t* p_error_message;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
			GetLastError(), MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
			(LPTSTR)&p_error_message, 0, NULL);

		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
		std::wstring wstring = p_error_message;
		string str = converter.to_bytes(wstring);
		std::cout << GetLastError() << ": " << str << endl;
		LocalFree(p_error_message);
	}

	::SendMessage(hWndProgress, PBM_SETRANGE, 0, MAKELPARAM(0, 20));

	for (size_t i = 0; i < nShapeCount; ++i) {
		SHPObject* psShape = SHPReadObject(hSHP, i);
		objectVertices.push_back(psShape->nVertices);

		if (i == 0) {
			cout << psShape->nShapeId << endl;
			cout << psShape->nVertices << endl;
			cout << psShape->nParts << endl;
			printf("%0.16f, %0.16f\n", psShape->dfXMin, psShape->dfYMin);
			printf("%0.16f, %0.16f\n", psShape->dfXMax, psShape->dfYMax);
		}

		for (int v = 0; v < psShape->nVertices; v++) {
			float x = (float)(psShape->padfX[v]);
			float y = (float)(psShape->padfY[v]);

			if (i == 0) {
				printf("%0.16f, %0.16f\n", x, y);
			}

			xMin = min(xMin, x);
			yMin = min(yMin, y);
			xMax = max(xMax, x);
			yMax = max(yMax, y);

			vertices.push_back(x);
			vertices.push_back(y);
		}
		::SendMessage(hWndProgress, PBM_SETPOS, (int)(i * 10 / nShapeCount), 0);
		SHPDestroyObject(psShape);
	}

	DestroyWindow(hWndProgress);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	AllocConsole();
	freopen("CONOUT$", "w", stderr);
	freopen("CONOUT$", "w", stdout);

	INITCOMMONCONTROLSEX icex = { sizeof(INITCOMMONCONTROLSEX) };
	icex.dwICC = ICC_PROGRESS_CLASS;
	InitCommonControlsEx(&icex);

	WNDCLASSEX wcex = {};
	{
		wcex.cbSize = sizeof(wcex);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = WindowProc;
		wcex.hInstance = hInstance;
		wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDC_MY3DSHAPEVIEWER));
		wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_MY3DSHAPEVIEWER);
		wcex.lpszClassName = L"OpenGLWindowClass";
	}
	RegisterClassEx(&wcex);

	hWnd = CreateWindowEx(0, L"OpenGLWindowClass", L"3D Shape Viewer", WS_OVERLAPPEDWINDOW,
		300, 0, WINDOW_WIDTH, WINDOW_HEIGHT,
		NULL, NULL, hInstance, NULL);

	hInst = hInstance;

	ShowWindow(hWnd, nCmdShow);


	if (!initialize()) {
		return -1;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MY3DSHAPEVIEWER));
	MSG msg = {};

	glClear(GL_COLOR_BUFFER_BIT);

	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	cleanUp();

	FreeConsole();

	return static_cast<int>(msg.wParam);
}

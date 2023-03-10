#include "3DShapeViewer.h"

#include <stdarg.h>

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

vector<float> vertexData;



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
	cout << "Shader compile complite" << endl;
	return true;
}

bool compileShader(GLuint shader, const char* source)
{
	glShaderSource(shader, 1, &source, nullptr);
	glCompileShader(shader);
	return checkShaderCompileStatus(shader);
}



void initialize()
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
	compileShader(vertexShader, shaderCstr);

	shaderSource = readShader("source.frag");
	shaderCstr = shaderSource.c_str();

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	compileShader(fragmentShader, shaderCstr);


	program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);
	glUseProgram(program);

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
}

void render()
{
	glm::vec3 cameraPosition = glm::vec3(cameraX, cameraY, 1.0f);
	glm::vec3 cameraTarget = glm::vec3(cameraX, cameraY, 0.0f);
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

	glm::mat4 viewMatrix = glm::lookAt(cameraPosition, cameraTarget, cameraUp);

	glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE, glm::value_ptr(viewMatrix));

	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_POINTS, 0, vertexData.size() / 2);
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
	glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);

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

void readShapefile(float& xMin, float& xMax, float& yMin, float& yMax) {
	int nShapeCount;
	SHPGetInfo(hSHP, &nShapeCount, NULL, NULL, NULL);

	for (size_t i = 0; i < nShapeCount; ++i) {
		SHPObject* psShape = SHPReadObject(hSHP, i);

		for (int i = 0; i < psShape->nVertices; i++) {
			float x = (float)(psShape->padfX[i]);
			float y = (float)(psShape->padfY[i]);

			xMin = min(xMin, x);
			yMin = min(yMin, y);
			xMax = max(xMax, x);
			yMax = max(yMax, y);

			vertexData.push_back(x);
			vertexData.push_back(y);
		}

		SHPDestroyObject(psShape);
	}
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

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
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

	hWnd = CreateWindowEx(0, L"OpenGLWindowClass", L"OpenGL ES 3.0 Window", WS_OVERLAPPEDWINDOW,
		300, 0, WINDOW_WIDTH, WINDOW_HEIGHT,
		NULL, NULL, hInstance, NULL);
	hInst = hInstance;
	ShowWindow(hWnd, nCmdShow);

	AllocConsole();
	freopen("CONOUT$", "w", stderr);
	freopen("CONOUT$", "w", stdout);

	initialize();

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

	cleanUp();

	FreeConsole();

	return static_cast<int>(msg.wParam);
}
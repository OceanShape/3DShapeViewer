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

FILE* SHPFile;
bool isShapeLoaded = false;
int32_t recordCount = 0;

float cameraX = 0.0f;
float cameraY = 0.0f;
const float delta = 0.02f;

vector<float> vertices;
vector<int> objectVertices;

typedef unsigned char uchar;


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




void closeShapefile() {
	if (isShapeLoaded) fclose(SHPFile);
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

struct SHPHeader {
	int32_t fileCode;
	int32_t fileLen;
	int32_t version;
	int32_t SHPType;

	double Xmin;
	double Ymin;
	double Xmax;
	double Ymax;
	double Zmin;
	double Zmax;
	double Mmin;
	double Mmax;
};

struct SHPPoint {
	double x;
	double y;
};

struct SHPPolygon {
	double box[4];
	shared_ptr<vector<int32_t>> parts;
	shared_ptr<vector<SHPPoint>> points;
};

void memSwap(void* const data, size_t size) {
	uint8_t* start = (uint8_t*)data;
	uint8_t* end = (uint8_t*)data + size - 1;
	while (start < end) {
		uint8_t tmp = *start;
		*start = *end;
		*end = tmp;
		start++, end--;
	}
}

void readShapefileCustom(float& xMin, float& xMax, float& yMin, float& yMax) {
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


	SHPHeader shpHeaderData;

	FILE* fp = SHPFile;

	fseek(fp, 0L, SEEK_END);
	long fileSize = ftell(fp);
	rewind(fp);

	uchar* data = new uchar[fileSize];
	memset(data, 0, fileSize);
	fread(data, sizeof(uchar), fileSize, fp);

	uchar* offset = data;

	// File Code
	std::memcpy(&shpHeaderData.fileCode, offset, 4); offset += 4;
	memSwap(&shpHeaderData.fileCode, 4);

	// Unused
	offset += 20;

	// File Length
	std::memcpy(&shpHeaderData.fileLen, offset, 4);  offset += 4;
	memSwap(&shpHeaderData.fileLen, 4);

	// version
	std::memcpy(&shpHeaderData.version, offset, 4);  offset += 4;

	// Shape Type
	std::memcpy(&shpHeaderData.SHPType, offset, 4);  offset += 4;

	// Bounding Box
	std::memcpy(&shpHeaderData.Xmin, offset, 8); offset += 8;
	std::memcpy(&shpHeaderData.Ymin, offset, 8); offset += 8;
	std::memcpy(&shpHeaderData.Xmax, offset, 8); offset += 8;
	std::memcpy(&shpHeaderData.Ymax, offset, 8); offset += 8;
	std::memcpy(&shpHeaderData.Zmin, offset, 8); offset += 8;
	std::memcpy(&shpHeaderData.Zmax, offset, 8); offset += 8;
	std::memcpy(&shpHeaderData.Mmin, offset, 8); offset += 8;
	std::memcpy(&shpHeaderData.Mmax, offset, 8); offset += 8;

	SHPPoint* points = new SHPPoint[1000];

	while (offset < data + fileSize) {
		int32_t recordNum;
		int32_t contentLength;

		int32_t shapeType;
		double box[4];
		int32_t numParts;
		int32_t numPoints;
		int32_t parts;

		std::memcpy(&recordNum, offset, 4);  offset += 4;
		memSwap(&recordNum, 4);

		std::memcpy(&contentLength, offset, 4);  offset += 4;
		memSwap(&contentLength, 4);

		std::memcpy(&shapeType, offset, 4);  offset += 4;

		std::memcpy(box, offset, sizeof(double) * 4);  offset += sizeof(double) * 4;

		std::memcpy(&numParts, offset, 4);  offset += 4;
		std::memcpy(&numPoints, offset, 4);  offset += 4;

		std::memcpy(&parts, offset, sizeof(int32_t) * numParts);  offset += sizeof(int32_t) * numParts;

		std::memcpy(points, offset, sizeof(SHPPoint) * numPoints);

		offset += sizeof(double) * 2 * numPoints;

		objectVertices.push_back(numPoints);

		for (int p = 0; p < numPoints; p++) {
			float x = points[p].x;
			float y = points[p].y;

			xMin = std::min(xMin, x);
			yMin = std::min(yMin, y);
			xMax = std::max(xMax, x);
			yMax = std::max(yMax, y);

			vertices.push_back(x);
			vertices.push_back(y);
		}

		recordCount++;
	}

	delete[] points;
	delete[] data;

	DestroyWindow(hWndProgress);
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

	wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;
	SHPFile = fopen(converter.to_bytes(szFileName).c_str(), "rb");

	if (SHPFile == nullptr) return false;

	float xMin = FLT_MAX;
	float xMax = FLT_MIN;
	float yMin = FLT_MAX;
	float yMax = FLT_MIN;

	readShapefileCustom(xMin, xMax, yMin, yMax);

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

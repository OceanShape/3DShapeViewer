#include "3DShapeViewer.h"
#include "shapefile.h"
#include "shapedata.h"
#include "quadtree.h"

EGLint EGL_OPENGL_ES3_BIT_KHR = 0x0040;

const int WINDOW_POS_X = 500;
const int WINDOW_POS_Y = 0;
const int WINDOW_WIDTH = 913;
const int WINDOW_HEIGHT = 959;

EGLDisplay eglDisplay;
EGLSurface eglSurface;
EGLContext eglContext;
EGLConfig eglConfig;
EGLint contextAttribs[] = {
	EGL_CONTEXT_CLIENT_VERSION, 3,
	EGL_NONE
};

HWND hWnd;
HINSTANCE hInst;
TCHAR szFileName[MAX_PATH];

GLuint vao[2];
GLuint vbo[2];
GLuint program;

FILE* SHPFile;
bool isShapeLoaded = false;
int32_t recordCount = 0;
float aspectRatio = 1.0f;

float cameraX = 0.0f;
float cameraY = 0.0f;
const float delta = 0.02f;

shared_ptr<ObjectData> objectData;

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

	vector<float> allObjectVertices;
	vector<float> allObjectVertexCount;
	vector<float> allBorderPoints;

	objectData->addVertexAndPoint(allObjectVertices, allObjectVertexCount, allBorderPoints);

	glBindVertexArray(vao[0]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, allObjectVertices.size() * sizeof(float), allObjectVertices.data(), GL_STATIC_DRAW);
	for (int i = 0, startIndex = 0; i < allObjectVertexCount.size(); ++i) {
		glDrawArrays(GL_LINE_STRIP, startIndex, allObjectVertexCount[i]);
		startIndex += allObjectVertexCount[i];
	}

	glBindVertexArray(vao[1]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, allBorderPoints.size() * sizeof(float), allBorderPoints.data(), GL_STATIC_DRAW);
	for (int i = 0; i < allBorderPoints.size() / (3 * 5); ++i) {
		glDrawArrays(GL_LINE_STRIP, i * 5, 5);
	}
}

void cleanUp()
{
	glDeleteVertexArrays(2, vao);
	glDeleteBuffers(2, vbo);

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
	if (isShapeLoaded) {
		objectData.reset();
		fclose(SHPFile);
	}
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

bool readShapefile(float min[], float del[]) {
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

	// Reading header
	{
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
	}


	// Check Shape Type
	if (shpHeaderData.SHPType % 10 != 3 && shpHeaderData.SHPType % 10 != 5) {
		cout << "Unsupported data type" << endl;
		return false;
	}

	float xMin = shpHeaderData.Xmin;
	float yMin = shpHeaderData.Ymin;
	float xMax = shpHeaderData.Xmax;
	float yMax = shpHeaderData.Ymax;
	float zMin = shpHeaderData.Zmin;
	float zMax = shpHeaderData.Zmax;

	min[0] = xMin;
	min[1] = yMin;
	min[2] = zMin;

	del[0] = (xMax - xMin) / 2.0f;
	del[1] = (yMax - yMin) / 2.0f;
	del[2] = (zMax - zMin) / 2.0f;

	float yTop = (yMin + yMax) / 2 + del[0];
	float yBot = (yMin + yMax) / 2 - del[0];


	objectData = make_shared<ObjectData>(shpHeaderData.Xmin, shpHeaderData.Xmax, yBot, yTop);

	SHPPoint* points = new SHPPoint[1000];
	double* Zpoints = new double[1000]; // 13: PolyLineZ(ArcZ)
	int32_t* parts = new int32_t[1000];

	while (offset < data + fileSize) {
		uchar* startOffset = offset;

		bool hasZvalue = false;

		int32_t recordNum;
		int32_t contentLength;

		int32_t shapeType;
		double box[4];
		int32_t numParts;
		int32_t numPoints;
		double Zrange[2];   // min, max


		std::memcpy(&recordNum, offset, 4);  offset += 4;
		memSwap(&recordNum, 4);

		std::memcpy(&contentLength, offset, 4);  offset += 4;
		memSwap(&contentLength, 4);

		std::memcpy(&shapeType, offset, 4);  offset += 4;

		std::memcpy(box, offset, sizeof(double) * 4);  offset += sizeof(double) * 4;

		std::memcpy(&numParts, offset, 4);  offset += 4;
		std::memcpy(&numPoints, offset, 4);  offset += 4;

		std::memcpy(parts, offset, sizeof(int32_t) * numParts);	offset += sizeof(int32_t) * numParts;

		std::memcpy(points, offset, sizeof(SHPPoint) * numPoints);	offset += sizeof(SHPPoint) * numPoints;

		// Z point
		if (offset - startOffset < contentLength * 2) {
			hasZvalue = true;

			std::memcpy(Zrange, offset, sizeof(double) * 2);    offset += sizeof(double) * 2;
			std::memcpy(Zpoints, offset, sizeof(double) * numPoints);	offset += sizeof(double) * numPoints;
		}

		// M point(ignore)
		if (offset - startOffset < contentLength * 2) {
			offset += sizeof(double) * 2;
			offset += sizeof(double) * numPoints;
		}

		vector<float> objectVertices;

		for (int p = 0; p < numPoints; p++) {
			float x = points[p].x;
			float y = points[p].y;
			float z = (hasZvalue) ? Zpoints[p] : 0.0f;

			objectVertices.push_back(x);
			objectVertices.push_back(y);
			objectVertices.push_back(z);
		}

		bool trigger = false;
		if (recordCount == 20126) {
			trigger = true;
		}

		objectData->storeObject(objectVertices, trigger);

		recordCount++;
	}

	std::cout << "Total record count: " << recordCount << endl;

	delete[] Zpoints;
	delete[] parts;
	delete[] points;
	delete[] data;

	DestroyWindow(hWndProgress);

	return true;
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

	if (isShapeLoaded) {
		objectData.reset();
		recordCount = 0;
		cameraX = 0.0f;
		cameraY = 0.0f;
	}

	GLfloat min[3], del[3];
	if (readShapefile(min, del) == false) {
		return false;
	}

	glUniform1f(glGetUniformLocation(program, "aspect_ratio"), del[0] / del[1]);

	glUniform3fv(glGetUniformLocation(program, "minimum"), 1, min);

	glUniform3fv(glGetUniformLocation(program, "delta"), 1, del);


	glGenVertexArrays(2, vao);
	glGenBuffers(2, vbo);

	glBindVertexArray(vao[0]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(vao[1]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);


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
		WINDOW_POS_X, WINDOW_POS_Y, WINDOW_WIDTH, WINDOW_HEIGHT,
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

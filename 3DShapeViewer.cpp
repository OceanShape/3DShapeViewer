#include "3DShapeViewer.h"
#include "shapefile.h"
#include "shapedata.h"
#include "quadtree.h"
#include "object.h"

EGLint EGL_OPENGL_ES3_BIT_KHR = 0x0040;

const int WINDOW_POS_X = 500;
const int WINDOW_POS_Y = 0;
const int WINDOW_WIDTH = 913;
const int WINDOW_HEIGHT = 959;
const float CAMERA_START_Z = 3.0f;

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
GLuint ebo;
GLuint programs[2];
bool groundMode = false;

FILE* SHPFile;
bool isShapeLoaded = false;
int32_t recordCount = 0;
float aspectRatio = 1.0f;


float fov = 45.0f;
float cameraX = 0.0f;
float cameraY = 0.0f;
float cameraZ = CAMERA_START_Z;
float moveX = 0.0f;
float moveY = 0.0f;
float moveZ = 3.0f;
const float delta = 0.02f;
const float deltaZ = 0.1f;

float yaw = -90.0f;
float pitch = 0.0f;
float rotDel = 1.0f;

bool mouseClicked = false;
float lastX = 450.0f;
float lastY = 450.0f;

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 cameraRight = glm::vec3(1.0f, .0f, .0f);

shared_ptr<ObjectData> objectData;
std::vector<shared_ptr<Object>> objects;

int maxLevel = 0;
int currentLevel = 0;

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

	string shaderFileName[] = { "object.vert", "object.frag", "grid.vert", "grid.frag" };
	for (size_t i = 0; i < 2; ++i) {
		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
		if (!compileShader(vertexShader, readShader(shaderFileName[i * 2]).c_str())) {
			return false;
		}

		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		if (!compileShader(fragmentShader, readShader(shaderFileName[i * 2 + 1]).c_str())) {
			return false;
		}

		programs[i] = glCreateProgram();
		glAttachShader(programs[i], vertexShader);
		glAttachShader(programs[i], fragmentShader);
		glLinkProgram(programs[i]);
		glUseProgram(programs[i]);
	}

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

	return true;
}

void updateCameraVec() {
	glm::vec3 direction;
	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction.y = sin(glm::radians(pitch));
	direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(direction);
	cameraRight = glm::normalize(glm::cross(cameraFront, glm::vec3(.0f, 1.0f, .0f)));
	cameraUp = glm::normalize(glm::cross(cameraRight, cameraFront));
}

void render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// model
	glm::mat4 model = glm::mat4(1.0f);// glm::scale(glm::mat4(1.0f), { 1.0f, 1.0f, 0.0f });//glm::mat4(1.0f);

	// view
	glm::vec3 position = glm::vec3(cameraX, cameraY, cameraZ);
	glm::mat4 view = glm::lookAt(position, position + cameraFront, cameraUp);

	// projection
	glm::mat4 projection = glm::perspective(glm::radians(fov), 1.0f, 0.1f, 10.0f);

	for (int i = 0; i < 2; ++i) {
		glUseProgram(programs[i]);
		glUniformMatrix4fv(glGetUniformLocation(programs[i], "model"), 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(glGetUniformLocation(programs[i], "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(programs[i], "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	}

	vector<float> allObjectVertices;
	vector<float> allObjectVertexCount;
	vector<float> allBorderPoints;

	static int tester = 0;
	objectData->addVertexAndPoint(allObjectVertices, allObjectVertexCount, allBorderPoints, currentLevel, tester);



	// draw objects
	glUseProgram(programs[0]);
	glBindVertexArray(vao[0]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);

	objectData->renderObject(currentLevel);

	// draw grid
	glUseProgram(programs[1]);
	glBindVertexArray(vao[1]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);

	objectData->renderBorder(currentLevel);
}

void cleanUp()
{
	glDeleteVertexArrays(2, vao);
	glDeleteBuffers(2, vbo);
	glDeleteBuffers(1, &ebo);

	for (size_t i = 0; i < 2; ++i) {
		GLuint vertexShader, fragmentShader;
		glGetAttachedShaders(programs[i], 1, NULL, &vertexShader);
		glGetAttachedShaders(programs[i], 1, NULL, &fragmentShader);
		glDetachShader(programs[i], vertexShader);
		glDetachShader(programs[i], fragmentShader);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
		glDeleteProgram(programs[i]);
	}

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

void setCurrentLevel(int cameraZ) {
	if (0.0f <= cameraZ <= 3.0f) {
		float deltaLevel = 1.0f / (maxLevel + 1.0f);
		currentLevel = (int)((1.0f - cameraZ / CAMERA_START_Z) / deltaLevel);
	}
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wDel;
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
	case WM_MOUSEWHEEL:
		wDel = GET_WHEEL_DELTA_WPARAM(wParam) / 120;
		fov -= wDel * .1f;
		fov = (fov > 89.0f) ? 89.0f : (fov < 1.0f) ? 1.0f : fov;
		break;
	case WM_KEYDOWN:
		if (wParam == 'E' || wParam == 'e') {
			cameraZ -= deltaZ;
			setCurrentLevel(cameraZ);
		}
		else if (wParam == 'Q' || wParam == 'q') {
			cameraZ += deltaZ;
			setCurrentLevel(cameraZ);
		}
		else if (wParam == 'A' || wParam == 'a') {
			cameraX -= delta;
		}
		else if (wParam == 'D' || wParam == 'd') {
			cameraX += delta;
		}
		else if (wParam == 'W' || wParam == 'w') {
			cameraY += delta;
		}
		else if (wParam == 'S' || wParam == 's') {
			cameraY -= delta;
		}
		else if (wParam == 'J' || wParam == 'j') {
			yaw = (yaw < -175.0f) ? -175.0f : yaw - rotDel;
			updateCameraVec();
		}
		else if (wParam == 'L' || wParam == 'l') {
			yaw = (yaw > 5.0f) ? 5.0f: yaw + rotDel;
			updateCameraVec();
		}
		else if (wParam == 'I' || wParam == 'i') {
			pitch = (pitch > 85.0f) ? 85.0f : pitch + rotDel;
			updateCameraVec();
		}
		else if (wParam == 'K' || wParam == 'k') {
			pitch = (pitch < -85.0f) ? -85.0f : pitch - rotDel;
			updateCameraVec();
		}
		else if (wParam == 'G' || wParam == 'g') {
			groundMode = !groundMode;
		}
		else if ('0' <= wParam && wParam <= '9') {
			int num = wParam - '0';
			if (num > maxLevel) {
				num = maxLevel;
			}
			currentLevel = num;
		}
		break;
	case WM_MOUSEMOVE:
		if (mouseClicked) {
			
		}
		break;
	case WM_LBUTTONDOWN:
		mouseClicked = true;
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

bool readShapefile(float min[], float max[], float del[]) {
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

	std::fseek(fp, 0L, SEEK_END);
	long fileSize = ftell(fp);
	std::rewind(fp);

	uchar* data = new uchar[fileSize];
	std::memset(data, 0, fileSize);
	std::fread(data, sizeof(uchar), fileSize, fp);

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
		std::cout << "Unsupported data type" << endl;
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
	max[2] = zMax;

	del[0] = (xMax - xMin) / 2.0f;
	del[1] = (yMax - yMin) / 2.0f;
	del[2] = (zMax - zMin) / 2.0f;

	float yTop = (yMin + yMax) / 2 + del[0];
	float yBot = (yMin + yMax) / 2 - del[0];

	std::cout << "header Z min/max: " << zMin << "/" << zMax << endl;

	objectData = make_shared<ObjectData>(shpHeaderData.Xmin, shpHeaderData.Xmax, yBot, yTop);

	SHPPoint* points;
	double* Zpoints;
	int32_t* parts;
	vector<int> objectVertexCount;
	int allVertexCount = 0;

	//for (int i = 0; i < 2; ++i){
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

		parts = new int32_t[numParts];
		std::memcpy(parts, offset, sizeof(int32_t) * numParts);	offset += sizeof(int32_t) * numParts;

		points = new SHPPoint[numPoints];
		std::memcpy(points, offset, sizeof(SHPPoint) * numPoints);	offset += sizeof(SHPPoint) * numPoints;

		// Z point
		if (offset - startOffset < contentLength * 2) {
			Zpoints = new double[numPoints];
			hasZvalue = true;

			std::memcpy(Zrange, offset, sizeof(double) * 2);    offset += sizeof(double) * 2;
			std::memcpy(Zpoints, offset, sizeof(double) * numPoints);	offset += sizeof(double) * numPoints;
			delete[] Zpoints;
		}

		// M point(ignore)
		if (offset - startOffset < contentLength * 2) {
			offset += sizeof(double) * 2;
			offset += sizeof(double) * numPoints;
		}

		shared_ptr<Object> obj = make_shared<Object>(points, numPoints, parts, numParts);
		objects.push_back(obj);
		objectData->storeObject(obj, maxLevel);
		objectVertexCount.push_back(numPoints);
		allVertexCount += numPoints;

		delete[] parts;
		delete[] points;

		recordCount++;
	}

	std::cout << "Total record count: " << objects.size() << endl;
	objectData->allocateObjectMemory(objects.size(), allVertexCount, objectVertexCount.data());
	objectData->allocateGridMemory();

	delete[] data;

	DestroyWindow(hWndProgress);

	return true;
}

bool openShapefile() {
	OPENFILENAME ofn;

	std::ZeroMemory(&ofn, sizeof(ofn));
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

	
	GLfloat min[3], max[3], del[3];
	if (readShapefile(min, max, del) == false) {
		return false;
	}
	printf("del: %.10f\n", del[2]);
	printf("z position: 0/%f\n", (max[2] - min[2]) / del[2]);
	

	for (int i = 0; i < 2; ++i) {
		glUseProgram(programs[i]);
		glUniform1f(glGetUniformLocation(programs[i], "aspect_ratio"), del[0] / del[1]);
		glUniform3fv(glGetUniformLocation(programs[i], "minimum"), 1, min);
		glUniform3fv(glGetUniformLocation(programs[i], "delta"), 1, del);
	}


	glGenVertexArrays(2, vao);
	glGenBuffers(2, vbo);
	glGenBuffers(1, &ebo);

	glUseProgram(programs[0]);
	glBindVertexArray(vao[0]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

	glUseProgram(programs[1]);
	glBindVertexArray(vao[1]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

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

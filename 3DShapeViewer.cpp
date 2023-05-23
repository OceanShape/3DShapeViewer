#include "3DShapeViewer.h"

ShapeViewer::ShapeViewer() {
	camera = make_shared<Camera>(0.0f, 0.0f, 3.0f);
	g_shapeViewer = this;
}

bool ShapeViewer::initialize(HINSTANCE hInstance, int nCmdShow)
{
	hWnd = CreateWindowEx(0, L"OpenGLWindowClass", L"3D Shape Viewer", WS_OVERLAPPEDWINDOW,
		WINDOW_POS_X, WINDOW_POS_Y, WINDOW_WIDTH, WINDOW_HEIGHT,
		NULL, NULL, hInstance, NULL);

	hInst = hInstance;

	ShowWindow(hWnd, nCmdShow);

	EGLint numConfigs, majorVersion, minorVersion;
	EGLint attribs[] = {
	EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
	EGL_RED_SIZE, 8,
	EGL_GREEN_SIZE, 8,
	EGL_BLUE_SIZE, 8,
	EGL_ALPHA_SIZE, 8,
	EGL_DEPTH_SIZE, 24,
	EGL_STENCIL_SIZE, 8,
	EGL_RENDERABLE_TYPE, eglOptions.EGL_OPENGL_ES3_BIT_KHR,
	EGL_NONE
	};
	eglOptions.eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	eglInitialize(eglOptions.eglDisplay, &majorVersion, &minorVersion);
	eglChooseConfig(eglOptions.eglDisplay, attribs, &eglOptions.eglConfig, 1, &numConfigs);
	eglOptions.eglSurface = eglCreateWindowSurface(eglOptions.eglDisplay, eglOptions.eglConfig, hWnd, NULL);
	eglOptions.eglContext = eglCreateContext(eglOptions.eglDisplay, eglOptions.eglConfig, EGL_NO_CONTEXT, eglOptions.contextAttribs);
	eglMakeCurrent(eglOptions.eglDisplay, eglOptions.eglSurface, eglOptions.eglSurface, eglOptions.eglContext);

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


		GLuint program = glCreateProgram();
		glAttachShader(program, vertexShader);
		glAttachShader(program, fragmentShader);
		glLinkProgram(program);
		glUseProgram(program);
		renderOption.program[i] = program;
	}

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	
	return true;
}

bool ShapeViewer::isKeyPressed(char ch) {
	return (65 <= ch && ch <= 90) ? keyPressed[ch] : (97 <= ch && ch <= 122) ? keyPressed[ch - 32] : keyPressed[ch];
}

void ShapeViewer::update() {
	if (isShapeLoaded == false) return;

	if (isKeyPressed('A')) {
		camera->moveRight(-1);
	}
	else if (isKeyPressed('D')) {
		camera->moveRight(1);
	}
	else if (isKeyPressed('W')) {
		camera->moveUp(1);
	}
	else if (isKeyPressed('S')) {
		camera->moveUp(-1);
	}
	else if (isKeyPressed('Q')) {
		camera->moveForward(-1);
	}
	else if (isKeyPressed('E')) {
		camera->moveForward(1);
	}
	else if (isKeyPressed('J')) {
		camera->updateKey(0, 1, 0);
	}
	else if (isKeyPressed('L')) {
		camera->updateKey(0, -1, 0);
	}
	else if (isKeyPressed('I')) {
		camera->updateKey(1, 0, 0);
	}
	else if (isKeyPressed('K')) {
		camera->updateKey(-1, 0, 0);
	}
	else if (isKeyPressed('O')) {
		camera->updateKey(0, 0, -1);
	}
	else if (isKeyPressed('U')) {
		camera->updateKey(0, 0, 1);
	}
	else if (isKeyPressed('G')) {
		drawGrid = !drawGrid;
	}

	for (int i = 9; i >= 0; i--) {
		if (isKeyPressed('0' + i)) {
			camera->setLevel(i);
			break;
		}
	}
}

void ShapeViewer::render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (isShapeLoaded == true) {
		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 view = camera->getView();
		glm::mat4 projection = camera->getProj();

		for (int i = 0; i < 2; ++i) {
			glUseProgram(renderOption.program[i]);
			glUniformMatrix4fv(glGetUniformLocation(renderOption.program[i], "model"), 1, GL_FALSE, glm::value_ptr(model));
			glUniformMatrix4fv(glGetUniformLocation(renderOption.program[i], "view"), 1, GL_FALSE, glm::value_ptr(view));
			glUniformMatrix4fv(glGetUniformLocation(renderOption.program[i], "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		}

		objectData->render(camera->currentLevel, camera->boundaryX, camera->boundaryY);
	}

	eglSwapBuffers(eglOptions.eglDisplay, eglOptions.eglSurface);
	return;
}

void ShapeViewer::cleanUp()
{
	glDeleteVertexArrays(2, renderOption.vao);
	glDeleteBuffers(2, renderOption.vbo);
	glDeleteBuffers(1, &renderOption.ebo);

	for (size_t i = 0; i < 2; ++i) {
		GLuint shaders[2];
		glGetAttachedShaders(renderOption.program[i], 2, NULL, shaders);
		for (size_t j = 0; j < 2; ++j) {
			glDetachShader(renderOption.program[i], shaders[j]);
			glDeleteShader(shaders[j]);
		}
		glDeleteProgram(renderOption.program[i]);
	}

	eglMakeCurrent(eglOptions.eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	eglDestroyContext(eglOptions.eglDisplay, eglOptions.eglContext);
	eglDestroySurface(eglOptions.eglDisplay, eglOptions.eglSurface);
	eglTerminate(eglOptions.eglDisplay);
}

void ShapeViewer::closeShapefile() {
	if (isShapeLoaded) {
		//objectData.reset();
		fclose(SHPFile);
	}
}

bool ShapeViewer::readShapefile() {
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

	Object::type = shpHeaderData.SHPType;

	float xMin = shpHeaderData.Xmin;
	float yMin = shpHeaderData.Ymin;
	float xMax = shpHeaderData.Xmax;
	float yMax = shpHeaderData.Ymax;
	float zMin = shpHeaderData.Zmin;
	float zMax = shpHeaderData.Zmax;

	minTotal[0] = xMin; minTotal[1] = yMin; minTotal[2] = zMin;
	maxTotal[0] = xMax; maxTotal[1] = yMax; maxTotal[2] = zMax;
	camera->minTotal[0] = xMin; camera->minTotal[1] = yMin; camera->minTotal[2] = zMin;
	camera->maxTotal[0] = xMax; camera->maxTotal[1] = yMax; camera->maxTotal[2] = zMax;

	std::cout << "header Z min/max: " << zMin << "/" << zMax << endl;

	delTotal[0] = (xMax - xMin) / 2.0f;
	delTotal[1] = (yMax - yMin) / 2.0f;
	delTotal[2] = (zMax - zMin) / 2.0f;
	camera->delTotal[0] = (xMax - xMin) / 2.0f;
	camera->delTotal[1] = (yMax - yMin) / 2.0f;
	camera->delTotal[2] = (zMax - zMin) / 2.0f;
	

	{
		float yTop = (yMin + yMax) / 2 + delTotal[0];
		float yBot = (yMin + yMax) / 2 - delTotal[0];

		camera->setBoundary(xMin, xMax, yBot, yTop);

		float min[2] = { shpHeaderData.Xmin, yBot };
		float max[2] = { shpHeaderData.Xmax , yTop };
		objectData = make_shared<ObjectData>(min, max);
	}

	int maxLevel = 0;
	SHPPoint* points;
	double* Zpoints;
	int32_t* parts;

	//for (int i = 0; i < 1; ++i){
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
		objectData->store(obj, maxLevel);

		delete[] parts;
		delete[] points;

		recordCount++;
	}

	std::cout << "Total record count: " << objects.size() << endl;
	std::cout << "max level: " << maxLevel << endl;
	camera->maxLevel = maxLevel;

	delete[] data;

	DestroyWindow(hWndProgress);

	return true;
}

bool ShapeViewer::openShapefile() {
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
	}

	if (readShapefile() == false) {
		return false;
	}
	printf("del: %.10f\n", delTotal[2]);
	printf("z position: 0/%f\n", (maxTotal[2] - minTotal[2]) / delTotal[2]);
	

	for (int i = 0; i < 2; ++i) {
		glUseProgram(renderOption.program[i]);
		glUniform1f(glGetUniformLocation(renderOption.program[i], "aspect_ratio"), delTotal[0] / delTotal[1]);
		glUniform3fv(glGetUniformLocation(renderOption.program[i], "minimum"), 1, minTotal);
		glUniform3fv(glGetUniformLocation(renderOption.program[i], "delta"), 1, delTotal);
	}


	glGenVertexArrays(2, renderOption.vao);
	glGenBuffers(2, renderOption.vbo);
	glGenBuffers(1, &renderOption.ebo);

	glUseProgram(renderOption.program[0]);
	glBindVertexArray(renderOption.vao[0]);
	glBindBuffer(GL_ARRAY_BUFFER, renderOption.vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderOption.ebo);

	glUseProgram(renderOption.program[1]);
	glBindVertexArray(renderOption.vao[1]);
	glBindBuffer(GL_ARRAY_BUFFER, renderOption.vbo[1]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderOption.ebo);

	objectData->setRenderOpiton(renderOption);

	isShapeLoaded = true;

	return true;
}

LRESULT ShapeViewer::msgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wDel;
	RECT rt;
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
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_LBUTTONDOWN:
		GetClientRect(hWnd, &rt);
		std::cout << GET_X_LPARAM(lParam) << "," << rt.bottom - rt.top  - GET_Y_LPARAM(lParam) << std::endl;
	break;
	case WM_MOUSEWHEEL:
		camera->updateZoom(GET_WHEEL_DELTA_WPARAM(wParam) / 120);
		break;
	case WM_KEYDOWN:
		keyPressed[wParam] = true;
		break;
	case WM_KEYUP:
		keyPressed[wParam] = false;
		break;
	case WM_DESTROY:
		closeShapefile();
		::PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return g_shapeViewer->msgProc(hWnd, message, wParam, lParam);
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


	ShapeViewer shapeViewer;

	if (!shapeViewer.initialize(hInstance, nCmdShow)) {
		return -1;
	}
	
	MSG msg = {};
	while (WM_QUIT != msg.message) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			shapeViewer.update();
			shapeViewer.render();
		}
	}

	shapeViewer.cleanUp();

	FreeConsole();

	return static_cast<int>(msg.wParam);
}

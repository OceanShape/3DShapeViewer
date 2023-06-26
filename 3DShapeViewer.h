#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include "framework.h"
#include "resource.h"
#include "shapefile.h"
#include "shapedata.h"
#include "quadtree.h"
#include "object.h"
#include "camera.h"
#include "utility.h"

#include <Commdlg.h>
#include <Windows.h>
#include <Windowsx.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <iostream>
#include <locale>
#include <string>
#include <codecvt>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <cstdint>
#include <stdarg.h>
#include <CommCtrl.h>

class ShapeViewer {
public:
	HWND hWnd;
	HINSTANCE hInst;
	TCHAR szFileName[MAX_PATH];
	bool keyPressed[256] = { false, };
	EGLOptions eglOptions;

	const int WINDOW_POS_X = 500;
	const int WINDOW_POS_Y = 0;
	const int WINDOW_WIDTH = 913;
	const int WINDOW_HEIGHT = 956;
	const float CAMERA_START_Z = 3.0f;

	RenderOption renderOption;
	bool drawGrid = false;

	FILE* SHPFile;
	bool isShapeLoaded = false;
	int recordCount = 0;
	float aspectRatio = 1.0f;

	glm::mat4 modelMat = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

	shared_ptr<Camera> camera;

	GLfloat minTotal[3]{ FLT_MIN, FLT_MIN, FLT_MIN };
	GLfloat maxTotal[3]{ FLT_MAX, FLT_MAX, FLT_MAX };
	GLfloat delTotal[3];

	float lastX = 450.0f;
	float lastY = 450.0f;
	int startMouseX = 0, startMouseY = 0;
	int mouseX = 0, mouseY = 0;

	shared_ptr<ObjectData> objectData;
	std::vector<shared_ptr<Object>> objects;

	bool objectPicked = false;
	bool getStatus = false;
	bool isFPS = true;
	bool mouseClicked = false;

	ShapeViewer();

	LRESULT msgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	void status();

	bool initialize(HINSTANCE hInstance, int nCmdShow);
	void render();
	void cleanUp();

	bool openShapefile();
	bool readShapefile();
	void closeShapefile();

	bool isKeyPressed(char ch);
	void update();
};

ShapeViewer* g_shapeViewer;
typedef unsigned char uchar;
using namespace std;

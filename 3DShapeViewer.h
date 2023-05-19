#pragma once
#define _CRT_SECURE_NO_WARNINGS

// #include "shapefil.h"
#include "framework.h"
#include "resource.h"

#include <Commdlg.h>
#include <Windows.h>
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

using namespace std;

class ShapeViewer {
public:
	LRESULT msgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
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
				DestroyWindow(hWnd);
				break;
			default:
				return DefWindowProc(hWnd, message, wParam, lParam);
			}
		}
		break;
		//case WM_MOUSEWHEEL:
		//	wDel = GET_WHEEL_DELTA_WPARAM(wParam) / 120;
		//	fov -= wDel * .1f;
		//	fov = (fov > 89.0f) ? 89.0f : (fov < 5.0f) ? 5.0f : fov;
		//	std::cout << "fov: " << fov << endl;
		//	break;
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
};

bool checkShaderCompileStatus(GLuint shader);
bool compileShader(GLuint shader, const char* source);

bool initialize();
void render();
void cleanUp();

string readShader(const string& filepath);
bool openShapefile();

bool readShapefile();
void closeShapefile();


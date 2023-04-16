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

bool checkShaderCompileStatus(GLuint shader);
bool compileShader(GLuint shader, const char* source);

bool initialize();
void render();
void cleanUp();

string readShader(const string& filepath);
bool openShapefile();

bool readShapefile(float min[], float max[], float del[]);
void closeShapefile();

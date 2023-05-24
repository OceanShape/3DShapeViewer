#pragma once
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>

// options
struct EGLOptions {
	EGLint EGL_OPENGL_ES3_BIT_KHR = 0x0040;
	EGLDisplay eglDisplay;
	EGLSurface eglSurface;
	EGLContext eglContext;
	EGLConfig eglConfig;
	EGLint contextAttribs[3] = {
		EGL_CONTEXT_CLIENT_VERSION, 3,
		EGL_NONE
	};
};

struct RenderOption {
	GLuint program[2];
	GLuint vao[2];
	GLuint vbo[2];
	GLuint ebo;
};

// shader compile functions
bool checkShaderCompileStatus(GLuint shader)
{
	GLint compileStatus;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
	if (compileStatus == GL_FALSE)
	{
		GLint infoLogLength;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
		std::vector<char> infoLog(infoLogLength);
		glGetShaderInfoLog(shader, infoLogLength, nullptr, infoLog.data());
		std::cerr << "Shader compile error: " << infoLog.data() << std::endl;
		return false;
	}
	return true;
}

bool compileShader(GLuint shader, const char* source)
{
	glShaderSource(shader, 1, &source, nullptr);
	glCompileShader(shader);
	return checkShaderCompileStatus(shader);
}

std::string readShader(const std::string& filepath) {
	std::ifstream file(filepath);
	if (!file.is_open()) {
		return "";
	}

	std::string shader_code;
	std::string line;
	while (getline(file, line)) {
		shader_code += line + "\n";
	}

	return shader_code;
}

// memory swap
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

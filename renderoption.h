#pragma once
#include "object.h"

struct RenderOption {
	GLuint program[2];
	GLuint vao[2];
	GLuint vbo[2];
	GLuint ebo;
};
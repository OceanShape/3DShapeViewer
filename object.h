#pragma once
#include <iostream>
#include <string>
#include <EGL/egl.h>
#include <GLES3/gl3.h>

struct Vertex {
	float x;
	float y;
	float z;
};

struct SHPPoint {
	float x;
	float y;
};

class Object {
private:
	Vertex* vertices = nullptr;
	GLuint* indices = nullptr;
	int vertexCount = 0;


public:
	//float min[3]{ DBL_MAX, DBL_MAX, DBL_MAX };
	//float max[3]{ DBL_MIN, DBL_MIN, DBL_MIN };
	float min[3]{ FLT_MAX, FLT_MAX, FLT_MAX };
	float max[3]{ FLT_MIN, FLT_MIN, FLT_MIN };

	Object(int _vertexCount, SHPPoint* _vertices) : vertexCount(_vertexCount) {
		vertices = new Vertex[vertexCount];
		indices = new GLuint[vertexCount];

		for (size_t i = 0; i < vertexCount; ++i) {
			float x = _vertices[i].x;
			float y = _vertices[i].y;
			float z = 0.0f;

			min[0] = std::min(min[0], x);
			max[0] = std::max(max[0], x);
			min[1] = std::min(min[1], y);
			max[1] = std::max(max[1], y);

			vertices[i] = { x, y, z };
		}

		setIndex();
	}

	// Set index with triangulation
	void setIndex() {
		for (size_t i = 0; i < vertexCount; ++i) {
			indices[i] = i;
		}
	}

	void render() {
		glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(float), vertices, GL_STATIC_DRAW);
		glDrawArrays(GL_LINE_STRIP, 0, vertexCount);

		/*glBufferData(GL_ELEMENT_ARRAY_BUFFER, vertexCount * sizeof(unsigned int), indices, GL_STATIC_DRAW);
		glDrawElements(GL_LINE_LOOP, vertexCount, GL_UNSIGNED_INT, 0);*/
	}

	~Object() {
		delete[] vertices;
		delete[] indices;
	}
};

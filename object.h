#pragma once
#include <string>
#include <EGL/egl.h>
#include <GLES3/gl3.h>

class Object {
	double* vertices = nullptr;
	GLuint* indices = nullptr;
	int vertexCount = 0;

	Object(int _vertexCount) : vertexCount(_vertexCount) {
		vertices = new double[vertexCount];
		indices = new GLuint[vertexCount];
	}

	void inputVertices(const double* const _vertices) {
		std::memcpy(vertices, _vertices, vertexCount * sizeof(double));
	}

	// Set index with triangulation
	void setIndex() {
		for (size_t i = 0; i < vertexCount; ++i) {
			indices[i] = i;
		}
	}

	void render() {
		glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(float), vertices, GL_STATIC_DRAW);
		//glDrawArrays(GL_LINE_STRIP, 0, vertexCount);

		glBufferData(GL_ELEMENT_ARRAY_BUFFER, vertexCount * sizeof(unsigned int), indices, GL_STATIC_DRAW);
		glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_INT, 0);
	}

	~Object() {
		delete[] vertices;
		delete[] indices;
	}
};

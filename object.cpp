#include "object.h"

class Object {
	double* vertices = nullptr;
	int* indices = nullptr;
	int vertexCount = 0;

	Object(int _vertexCount): vertexCount(_vertexCount) {
		vertices = new double[vertexCount];
		indices = new int[vertexCount];
	}

	void inputVertices(const double* const _vertices) {
		std::memcpy(vertices, _vertices, vertexCount * sizeof(double));
	}

	// Set index with triangulation
	void setIndex() {

	}

	~Object() {
		delete[] vertices;
		delete[] indices;
	}
};
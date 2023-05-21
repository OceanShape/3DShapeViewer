#pragma once
#include <iostream>
#include <string>
#include <EGL/egl.h>
#include <GLES3/gl3.h>

#include "quadtree.h"
#include "triangulation.h"

struct SHPPoint {
	double x;
	double y;
};

struct VertexFLT {
	float x, y, z;
};

class Object {
public:
	Triangulation::Vertex* verticesDBL = nullptr;
	VertexFLT* vertices = nullptr;
	GLuint* indices = nullptr;
	int indexCount = 0;
	int32_t* partVertexCounts = nullptr;
	int32_t* partStartIndex = nullptr;
	int vertexCount = 0;
	int partCount = 0;
	static int type;
	static GLuint program;

public:
	double min[3]{ DBL_MAX, DBL_MAX, DBL_MAX };
	double max[3]{ DBL_MIN, DBL_MIN, DBL_MIN };

	Object(SHPPoint* _vertices, int _vertexCount, int32_t* _parts, int _partCount) : vertexCount(_vertexCount), partCount(_partCount) {
		verticesDBL = new Triangulation::Vertex[vertexCount];
		vertices = new VertexFLT[vertexCount * 2];
		partVertexCounts = new int32_t[partCount];
		partStartIndex = new int32_t[partCount];

		for (size_t i = 0; i < vertexCount; ++i) {
			double x = _vertices[i].x;
			double y = _vertices[i].y;
			double z = 0.01f;

			min[0] = min(min[0], x);
			max[0] = max(max[0], x);
			min[1] = min(min[1], y);
			max[1] = max(max[1], y);

			verticesDBL[i] = { x, y, z };
			vertices[i] = { (float)x, (float)y, (float)z };
		}

		std::memcpy(vertices + vertexCount, vertices, vertexCount * sizeof(VertexFLT));

		for (size_t i = vertexCount; i < vertexCount * 2; ++i) {
			vertices[i].z = .0f;
		}

		for (size_t i = 0; i < partCount; ++i) {
			if (i == partCount - 1) {
				partVertexCounts[i] = vertexCount - _parts[i];
				break;
			}
			partVertexCounts[i] = _parts[i + 1] - _parts[i];
		}

		std::memcpy(partStartIndex, _parts, sizeof(int32_t) * partCount);

		if (partCount > 1) {
			for (int i = 0; i < partCount; ++i) {
				cout << _parts[i] << " ";
			}
			cout << endl;
		}

		setIndex();
	}

	// Set index with triangulation
	void setIndex() {
		std::vector<Triangulation::Triangle> triangulation;
		Triangulate(triangulation, verticesDBL, vertexCount, partVertexCounts, partCount);

		indexCount = triangulation.size() * 3 + (vertexCount - 1) * 6 + (vertexCount - 1) * 2 * 3;
		indices = new GLuint[indexCount];

		for (int i = 0; i < triangulation.size(); i++) {
			indices[i * 3] = triangulation[i].a;
			indices[i * 3 + 1] = triangulation[i].b;
			indices[i * 3 + 2] = triangulation[i].c;
		}


		int startIdx = triangulation.size() * 3;
		for (int i = 0; i < vertexCount - 1; ++i) {
			indices[startIdx + i * 6] = i;
			indices[startIdx + i * 6 + 1] = i + 1;
			indices[startIdx + i * 6 + 2] = vertexCount + i;

			indices[startIdx + i * 6 + 3] = i + 1;
			indices[startIdx + i * 6 + 4] = vertexCount + i + 1;
			indices[startIdx + i * 6 + 5] = vertexCount + i;
		}

		startIdx = triangulation.size() * 3 + (vertexCount - 1) * 6;
		for (int i = 0; i < vertexCount - 1; ++i) {
			indices[startIdx + i * 6] = i;
			indices[startIdx + i * 6 + 1] = i + 1;
			
			indices[startIdx + i * 6 + 2] = i;
			indices[startIdx + i * 6 + 3] = vertexCount + i;
			
			indices[startIdx + i * 6 + 4] = vertexCount + i;
			indices[startIdx + i * 6 + 5] = vertexCount + i + 1;
		}

	}

	void render() {
		glBufferData(GL_ARRAY_BUFFER, vertexCount * 2 * 3 * sizeof(float), vertices, GL_STATIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(GLuint), indices, GL_STATIC_DRAW);

		glUniform1i(glGetUniformLocation(program, "color"), 0);

		if (true) {
			glDrawElements(GL_TRIANGLES, indexCount - (vertexCount - 1) * 6, GL_UNSIGNED_INT, 0);
		}
		else {
			for (size_t pos = 0; pos < indexCount; pos += 3) {
				glDrawElements(GL_LINE_LOOP, 3, GL_UNSIGNED_INT, (const void*)(pos * sizeof(GLuint)));
			}
		}

		glUniform1i(glGetUniformLocation(program, "color"), 1);

		for (size_t pos = indexCount - (vertexCount - 1) * 6; pos < indexCount; pos += 2) {
			glDrawElements(GL_LINE_LOOP, 2, GL_UNSIGNED_INT, (const void*)(pos * sizeof(GLuint)));
		}
	}

	~Object() {
		delete[] verticesDBL;
		delete[] vertices;
		delete[] indices;
		delete[] partVertexCounts;
		delete[] partStartIndex;
	}
};

int Object::type = 0;
GLuint Object::program = 0;

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

class Object {
public:
	Triangulation::Vertex* vertices = nullptr;
	GLuint* indices = nullptr;
	int indexCount = 0;
	int32_t* partVertexCounts = nullptr;
	int32_t* partStartIndex = nullptr;
	int vertexCount = 0;
	int partCount = 0;

	static int type;

public:
	float min[3]{ FLT_MAX, FLT_MAX, FLT_MAX };
	float max[3]{ FLT_MIN, FLT_MIN, FLT_MIN };
	//double min[3]{ DBL_MAX, DBL_MAX, DBL_MAX };
	//double max[3]{ DBL_MIN, DBL_MIN, DBL_MIN };

	Object(SHPPoint* _vertices, int _vertexCount, int32_t* _parts, int _partCount) : vertexCount(_vertexCount), partCount(_partCount) {
		vertices = new Triangulation::Vertex[vertexCount];
		indices = new GLuint[vertexCount];
		partVertexCounts = new int32_t[partCount];
		partStartIndex = new int32_t[partCount];

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
		if (partCount == 1) { // type == 5 && partCount == 1
			// loop를 고려해서, 맨 마지막 점은 빼고 넣을 것
			//for (size_t i = 0; i < partVertexCounts[0]; ++i) {

			//}

			std::vector<Triangulation::Triangle> triangulation;
			Triangulate(triangulation, vertices, partVertexCounts[0]);

			delete[] indices;
			indexCount = triangulation.size() * 3;
			indices = new GLuint[triangulation.size() * 3];

			for (int i = 0; i < triangulation.size(); i++) {
				indices[i * 3] = triangulation[i].a;
				indices[i * 3 + 1] = triangulation[i].b;
				indices[i * 3 + 2] = triangulation[i].c;
			}
			std::cout << triangulation.size();
		}
		else {
			for (size_t i = 0; i < vertexCount; ++i) {
				indices[i] = i;
			}
		}
	}

	void render() {
		if (partCount == 1) { // type == 5
			glBufferData(GL_ARRAY_BUFFER, vertexCount * 3 * sizeof(float), vertices, GL_STATIC_DRAW);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(GLuint), indices, GL_STATIC_DRAW);
			if (partCount > 1) {
				glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
			}
			else {
				for (size_t pos = 0; pos < indexCount; pos += 3) {
					glDrawElements(GL_LINE_LOOP, 3, GL_UNSIGNED_INT, (const void*)(pos * sizeof(GLuint)));
				}
			}
		}
		else {
			glBufferData(GL_ARRAY_BUFFER, vertexCount * 3 * sizeof(float), vertices, GL_STATIC_DRAW);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, vertexCount * sizeof(GLuint), indices, GL_STATIC_DRAW);
			for (size_t currentPart = 0, pos = 0; currentPart < partCount; ++currentPart) {
				glDrawElements(GL_LINE_LOOP, partVertexCounts[currentPart], GL_UNSIGNED_INT, (const void*)(pos * sizeof(GLuint)));
				pos += partVertexCounts[currentPart];
			}

			//for (size_t currentPart = 0, pos = 0; currentPart < partCount; ++currentPart) {
			//	glDrawArrays(GL_LINE_STRIP, pos, partVertexCounts[currentPart]);
			//	pos += partVertexCounts[currentPart];
			//}
		}
	}

	~Object() {
		delete[] vertices;
		delete[] indices;
		delete[] partVertexCounts;
		delete[] partStartIndex;
	}
};

int Object::type = 0;

#pragma once
#include <iostream>
#include <string>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <glm/glm.hpp>

#include "quadtree.h"
#include "triangulation.h"
#include "frustum.h"

struct SHPPoint {
	double x;
	double y;
};

typedef glm::vec3 Vertex;

class Object {
public:
	const int ID;

	Triangulation::Vertex* verticesDBL = nullptr;
	Vertex* vertices = nullptr;
	GLuint* indices = nullptr;
	int indexCount = 0;
	int32_t* partVertexCounts = nullptr;
	int32_t* partStartIndex = nullptr;
	int vertexCount = 0;
	int partCount = 0;
	int triangleCount = 0;
	glm::vec3 pickedPoint{};

	static int type;
	static GLuint program;
	static bool objectSelected;

	const float objectColor[4] = { 1.0f, 0.5f, 0.2f, 1.0f };
	const float objectSideColor[4] = { 1.0f, .0f, .0f, 1.0f };
	const float tmp[8][4] = {
		{255.f / 256, 89.f / 255, 116.f / 255, 1.0f},
		{35.f / 255, 145.f / 255, 256.f / 256, 1.0f},
		{60.f / 256, 179.f / 256, 113.f / 256, 1.0f},
		{238.f / 256, 130.f / 256, 238.f / 256, 1.0f},
		{255.f / 256, 166.f / 256, 0.f / 256, 1.0f},
		{106.f / 256, 90.f / 256, 205.f / 256, 1.0f},
		{ 1.0f, 0.5f, 0.2f, 1.0f } ,
		{0.f, 134.f / 256, 68.f / 256},
	};

	const float selectedColor[4] = { 1.0f, 1.0f, 0.0f, 1.0f };
	glm::vec3 center;
	float radius = 0.0f;

public:
	double min[3]{ DBL_MAX, DBL_MAX, DBL_MAX };
	double max[3]{ DBL_MIN, DBL_MIN, DBL_MIN };

	Object(int _id, SHPPoint* _vertices, int _vertexCount, int32_t* _parts, int _partCount) : ID(_id), vertexCount(_vertexCount), partCount(_partCount) {
		verticesDBL = new Triangulation::Vertex[vertexCount];
		vertices = new Vertex[vertexCount * 2];
		partVertexCounts = new int32_t[partCount];
		partStartIndex = new int32_t[partCount];

		for (size_t i = 0; i < vertexCount; ++i) {
			double x = _vertices[i].x;
			double y = _vertices[i].y;
			double z = 10.0f;

			min[0] = std::min(min[0], x);
			max[0] = std::max(max[0], x);
			min[1] = std::min(min[1], y);
			max[1] = std::max(max[1], y);
			//min[2] = std::min(min[2], z);
			//max[2] = std::max(max[2], z);

			verticesDBL[i] = { x, y, z };
			vertices[i] = { (float)x, (float)y, (float)z };
		}
		min[2] = .0f; max[2] = 10.0f;

		std::memcpy(vertices + vertexCount, vertices, vertexCount * sizeof(Vertex));

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

		center = glm::vec3(0);
		for (size_t i = vertexCount; i < vertexCount * 2; ++i) {
			center += vertices[i];
		}
		center /= vertexCount; center.z = 0.0025f;

		radius = .0f;
		for (size_t i = vertexCount; i < vertexCount * 2; ++i) {
			radius = std::max(glm::length(center - vertices[i]), radius);
		}

		setIndex();
	}

	// Set index with triangulation
	void setIndex() {
		std::vector<Triangulation::Triangle> triangulation;
		Triangulate(triangulation, verticesDBL, vertexCount, partVertexCounts, partCount);

		indexCount = triangulation.size() * 3 + (vertexCount - 1) * 6 + (vertexCount - 1) * 2 * 3;
		indices = new GLuint[indexCount];
		triangleCount = triangulation.size();

		for (int i = 0; i < triangulation.size(); i++) {
			indices[i * 3] = triangulation[i].a;
			indices[i * 3 + 1] = triangulation[i].b;
			indices[i * 3 + 2] = triangulation[i].c;
		}

		// reverse vertex order for picking
		int startIdx = triangulation.size() * 3;
		for (int i = 0; i < vertexCount - 1; ++i) {
			indices[startIdx + i * 6] = vertexCount + i;
			indices[startIdx + i * 6 + 1] = i + 1;
			indices[startIdx + i * 6 + 2] = i;

			indices[startIdx + i * 6 + 3] = vertexCount + i;
			indices[startIdx + i * 6 + 4] = vertexCount + i + 1;
			indices[startIdx + i * 6 + 5] = i + 1;
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

	void render(bool isSelected, int level) {
		glBufferData(GL_ARRAY_BUFFER, vertexCount * 2 * 3 * sizeof(float), vertices, GL_STATIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(GLuint), indices, GL_STATIC_DRAW);

		glUniform4fv(glGetUniformLocation(program, "color"), 1, isSelected ? selectedColor : &(tmp[level][0]));

		if (true) {
			glDrawElements(GL_TRIANGLES, indexCount - (vertexCount - 1) * 6, GL_UNSIGNED_INT, 0);
		}
		else {
			for (size_t pos = 0; pos < indexCount - (vertexCount - 1) * 6; pos += 3) {
				glDrawElements(GL_LINE_LOOP, 3, GL_UNSIGNED_INT, (const void*)(pos * sizeof(GLuint)));
			}
		}

		glUniform4fv(glGetUniformLocation(program, "color"), 1, objectSideColor);

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
bool Object::objectSelected = false;

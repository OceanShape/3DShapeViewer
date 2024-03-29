#pragma once
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#include "utility.h"

// Frustum culling option
enum class FRUSTUM_CULLING {
	_OUT,
	_COMPLETE,
	_PARTIAL
};

struct Ray {
	glm::vec3 orig;
	glm::vec3 dir;
};

glm::vec3 getNormal(const glm::vec3 v[]) {
	return glm::normalize(glm::cross(v[2] - v[0], v[1] - v[0]));
};

bool isVertexInsideTriangle(const glm::vec3 tri[], const glm::vec3& p) {
	glm::vec3 normal = getNormal(tri);
	glm::vec3 m = cross(p - tri[0], tri[1] - tri[0]);
	glm::vec3 n = cross(p - tri[1], tri[2] - tri[1]);
	glm::vec3 o = cross(p - tri[2], tri[0] - tri[2]);

	return glm::dot(m, normal) > 0 && glm::dot(n, normal) > 0 && glm::dot(o, normal) > 0;
};

bool isRayIntersecTriangle(const Ray& ray, glm::vec3& inter, const glm::vec3 tri[]) {
	glm::vec3 normal = getNormal(tri);

	float dotTmp = glm::dot(normal, ray.dir);
	if (std::abs(dotTmp) < 0.0001f) return false;

	float d = -glm::dot(normal, tri[0]);

	inter = ray.orig - (glm::dot(normal, ray.orig) + d) * ray.dir / dotTmp;
	return isVertexInsideTriangle(tri, inter);
};

struct Plane {
	//[1][2]
	//[0][3]
	glm::vec3 vertices[4]{};
	glm::vec3 normal{};
	float d = .0f;

	Plane() {};
	//점은 시계방향으로 순서대로 들어온다고 가정
	//즉, 들어오는 점만으로 평면의 방정식을 정의할 수 있음
	Plane(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec3 v3) {
		update(v0, v1, v2, v3);
	}

	Plane(const glm::vec3 v[]) {
		update(v[0], v[1], v[2], v[3]);
	}

	bool getIntersecPoint(Ray ray, glm::vec3& answer) {
		float dotTmp = glm::dot(normal, ray.dir);
		if (std::abs(dotTmp) < 1e-10f) return false;
		answer = ray.orig - (glm::dot(normal, ray.orig) + d) * ray.dir / dotTmp;
		return isVertexInsidePlane(answer);
	}

	bool isVertexInsideTriangle(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& p) {
		glm::vec3 m = cross(p - v0, v1 - v0);
		glm::vec3 n = cross(p - v1, v2 - v1);
		glm::vec3 o = cross(p - v2, v0 - v2);

		return glm::dot(m, normal) > 0 && glm::dot(n, normal) > 0 && glm::dot(n, normal);
	}

	bool isVertexInsidePlane(const glm::vec3& p) {
		return isVertexInsideTriangle(vertices[0], vertices[1], vertices[2], p) 
			|| isVertexInsideTriangle(vertices[0], vertices[2], vertices[3], p);
	}

	void update(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec3 v3) {
		vertices[0] = v0, vertices[1] = v1, vertices[2] = v2, vertices[3] = v3;
		normal = getNormal();
		d = -glm::dot(getNormal(), vertices[0]);
	}

	glm::vec3 getNormal() {
		return glm::normalize(glm::cross(vertices[2] - vertices[0], vertices[1] - vertices[0]));
	}

	float getDistance(glm::vec3 point, bool justCheckFront = true) {
		return (glm::dot(getNormal(), point) + d) / (justCheckFront ? (1.0f) : glm::length(normal));
	}
};

struct Frustum {
	//near
	//[1][2]
	//[0][3]
	//far
	//[5][6]
	//[4][7]
	glm::vec3 vertices[8]{};
	float vertexFLT[24]{};

	Plane planes[6];
	// nearPlane
	// farPlane
	// leftPlane
	// rightPlane
	// topPlane
	// bottomPlane

	float nearZ;
	float farZ;
	float fov;
	glm::vec3 eyePos;
	
	static GLuint indices[];
	static float frustumColor[][4];

	static GLuint program;

	static glm::mat4 invViewProj;

	Frustum() {};

	Frustum(glm::vec3 direction, glm::vec3 up, glm::vec3 right, glm::vec3 _eyePos, float _nearZ, float _farZ, float _fov, glm::mat4 _invViewProj) : nearZ(_nearZ), farZ(_farZ), fov(_fov) {
		update(direction, up, right, _eyePos, fov, _invViewProj);
	};

	void setRenderOption(RenderOption option) {
		program = option.program[2];
	}

	bool inside(glm::vec3 v) {
		// nearPlane, farPlane, leftPlane, rightPlane, topPlane, bottomPlane
		bool t0, t1, t2, t3, t4, t5;
		t0 = planes[0].getDistance(v) < 0;
		t1 = true;
		t2 = planes[2].getDistance(v) < 0;
		t3 = planes[3].getDistance(v) < 0;
		t4 = planes[4].getDistance(v) < 0;
		t5 = planes[5].getDistance(v) < 0;
		return t2 && t3 && t4 && t5;
	}

	// Is this sphere inside the frustum?
	bool inSphere(glm::vec3 center, float radius) {
		for (size_t i = 2; i < 6; ++i) {
			if (planes[i].getDistance(center, false) > radius) return false;
		}
		return true;
	}

	void render(const float& ndcX, const float& ndcY, const Ray& ray) {
		//glBufferData(GL_ARRAY_BUFFER, 8 * 3 * sizeof(float), vertexFLT, GL_STATIC_DRAW);
		//glBufferData(GL_ELEMENT_ARRAY_BUFFER, 8 * 3 * sizeof(GLuint), indices, GL_STATIC_DRAW);

		//for (size_t pos = 0; pos < 8 * 3; pos += 3) {
		//	glUniform4fv(glGetUniformLocation(program, "color"), 1, frustumColor[pos % 2]);
		//	glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (const void*)(pos * sizeof(GLuint)));
		//}

		//glBufferData(GL_ARRAY_BUFFER, 8 * 3 * sizeof(float), vertexFLT, GL_STATIC_DRAW);
		//glBufferData(GL_ELEMENT_ARRAY_BUFFER, 8 * 3 * sizeof(GLuint), indices, GL_STATIC_DRAW);

		//for (size_t pos = 0; pos < 8 * 3; pos += 3) {
		//	glUniform4fv(glGetUniformLocation(program, "color"), 1, frustumColor[pos % 2]);
		//	glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (const void*)(pos * sizeof(GLuint)));
		//}
	}

	void update(glm::vec3 direction, glm::vec3 up, glm::vec3 right, glm::vec3 _eyePos, float fov, glm::mat4 viewProj) {
		float t = tan(glm::radians(fov / 2)); // fov default: 90.0f
		float val = nearZ * t;
		glm::vec3 eyePos = _eyePos / CONTRACT_RATE;
		glm::vec3* const v = vertices;

		v[0] = (-up - right) * val + direction * nearZ;
		v[1] = (+up - right) * val + direction * nearZ;
		v[2] = (+up + right) * val + direction * nearZ;
		v[3] = (-up + right) * val + direction * nearZ;

		val = farZ * t;
		v[4] = (-up - right) * val + direction * farZ;
		v[5] = (+up - right) * val + direction * farZ;
		v[6] = (+up + right) * val + direction * farZ;
		v[7] = (-up + right) * val + direction * farZ;

		for (int i = 0; i < 8; ++i) {
			v[i] += _eyePos;
			vertexFLT[i * 3] = v[i].x;
			vertexFLT[i * 3 + 1] = v[i].y;
			vertexFLT[i * 3 + 2] = v[i].z;
			// 판정은 축소한 채 그대로이기 때문에, 오브젝트가 보이지 않음
			// 하지만, 렌더링을 위한 버텍스는 그대로이므로 frustum이 잘 보인다
		}

		planes[0].update(v[0], v[1], v[2], v[3]);
		planes[1].update(v[4], v[7], v[6], v[5]);
		planes[2].update(v[4], v[5], v[1], v[0]);
		planes[3].update(v[3], v[2], v[6], v[7]);
		planes[4].update(v[1], v[5], v[6], v[2]);
		planes[5].update(v[0], v[3], v[7], v[4]);
		//std::cout << std::endl;
	}
};

GLuint Frustum::program = 0;
GLuint Frustum::indices[8 * 3] = {
	1, 0, 4,
	1, 4, 5,
	2, 1, 5,
	2, 5, 6,
	2, 3, 6,
	3, 7, 6,
	0, 3, 7,
	0, 7, 4,
};
float Frustum::frustumColor[][4] = { {1, 0, 1, 1}, {0, 1, 1, 1} };

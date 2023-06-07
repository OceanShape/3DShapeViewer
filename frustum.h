#pragma once
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

/*
struct Ray {
	glm::vec3 start;
	glm::vec3 dir;
};

	bool isIntersectTriangle(const glm::vec3& orig, const glm::vec3& dir,
		const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2) {

		glm::vec3 faceNormal = glm::normalize(glm::cross(vertices[2] - vertices[0], vertices[1] - vertices[0]));
		if (abs(glm::dot(dir, faceNormal)) < 1e-2f) return false;

		float t = (dot(vertices[0], faceNormal) - dot(orig, faceNormal)) / dot(dir, faceNormal);

		if (t < .0f) return false;

		glm::vec3 hitPoint = orig + t * dir;

		const glm::vec3 n0 = glm::normalize(glm::cross(vertices[1] - vertices[2], hitPoint - vertices[2]));
		const glm::vec3 n1 = glm::normalize(glm::cross(vertices[2] - vertices[0], hitPoint - vertices[0]));
		const glm::vec3 n2 = glm::normalize(glm::cross(vertices[0] - vertices[1], hitPoint - vertices[1]));

		if (dot(n0, faceNormal) < .0f || dot(n1, faceNormal) < .0f || dot(n2, faceNormal) < .0f) return false;

		return true;
	}

	bool isIntersect(const Ray& ray) {
		return isIntersectTriangle(ray.start, ray.dir, vertices[0], vertices[1], vertices[2]) && isIntersectTriangle(ray.start, ray.dir, vertices[0], vertices[2], vertices[3]);
	}
*/

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

	void update(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec3 v3) {
		vertices[0] = v0, vertices[1] = v1, vertices[2] = v2, vertices[3] = v3;
		normal = getNormal();
		d = -glm::dot(getNormal(), vertices[0]);
	}

	glm::vec3 getNormal() {
		return glm::normalize(glm::cross(vertices[1] - vertices[0], vertices[2] - vertices[0]));
	}

	float getDistance(glm::vec3 point, bool justCheckFront = true) {
		return (glm::dot(getNormal(), point) + d) / (justCheckFront ? (1.0f) : glm::length(normal));
	}

	bool isPointFront(glm::vec3 point) {
		return getDistance(point) > .0f;
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

	static GLuint indices[];
	static float frustumColor[][4];

	static GLuint program;

	Frustum() {};

	Frustum(glm::vec3 direction, glm::vec3 up, glm::vec3 right, glm::vec3 eyePos, float _nearZ, float _farZ, float _fov, glm::mat4 viewProj) : nearZ(_nearZ), farZ(_farZ), fov(_fov) {
		update(direction, up, right, eyePos, fov);
	}

	void setRenderOption(RenderOption option) {
		program = option.program[2];
	}

	// nearPlane, farPlane, leftPlane, rightPlane, topPlane, bottomPlane
	bool inside(glm::vec3 v) {
		bool t0, t1, t2, t3, t4, t5;
		t0 = planes[0].isPointFront(v);
		t1 = true;//planes[1].isPointFront(v); // always true
		t2 = planes[2].isPointFront(v);
		t3 = planes[3].isPointFront(v);
		t4 = planes[4].isPointFront(v);
		t5 = planes[5].isPointFront(v);
		return t0 && t1 && t2 && t3 && t4 && t5;
	}

	bool inSphere(glm::vec3 center, float radius) {
		bool res = true;
		std::cout << radius << ": " << std::endl;
		for (size_t i = 2; i < 6; ++i) {
			float distance = glm::abs(planes[i].getDistance(center, false));
			bool r = distance > radius;
			std::cout << r << " ";
			if (distance > radius) res = false;
		}
		std::cout << std::endl;
		return res; 
	}

	void render() {
		glBufferData(GL_ARRAY_BUFFER, 8 * 3 * sizeof(float), vertexFLT, GL_STATIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 8 * 3 * sizeof(GLuint), indices, GL_STATIC_DRAW);

		for (size_t pos = 0; pos < 8 * 3; pos += 3) {
			glUniform4fv(glGetUniformLocation(program, "color"), 1, frustumColor[pos % 2]);
			glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (const void*)(pos * sizeof(GLuint)));
		}
	}

	void update(glm::vec3 direction, glm::vec3 up, glm::vec3 right, glm::vec3 eyePos, float fov) {

		float t = tan(glm::radians(fov / 2));
		float val = nearZ * t;
		vertices[0] = eyePos + (-up - right) * val + direction * nearZ;
		vertices[1] = eyePos + (+up - right) * val + direction * nearZ;
		vertices[2] = eyePos + (+up + right) * val + direction * nearZ;
		vertices[3] = eyePos + (-up + right) * val + direction * nearZ;

		val = farZ * t;
		vertices[4] = eyePos + (-up - right) * val + direction * farZ;
		vertices[5] = eyePos + (+up - right) * val + direction * farZ;
		vertices[6] = eyePos + (+up + right) * val + direction * farZ;
		vertices[7] = eyePos + (-up + right) * val + direction * farZ;

		for (int i = 0; i < 8; ++i) {
			vertexFLT[i * 3] = vertices[i].x;
			vertexFLT[i * 3 + 1] = vertices[i].y;
			vertexFLT[i * 3 + 2] = vertices[i].z;
		}

		auto v = vertices;

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

#pragma once
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

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
		if (std::abs(dotTmp) < 0.0001f) return false;
		answer = ray.orig - (glm::dot(normal, ray.orig) + d) * ray.dir / dotTmp;
		return true;
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

	bool inside(const std::shared_ptr<Object> obj) {
		// sphere check

		// each point


		return true;
	}

	bool inSphere(glm::vec3 center, float radius) {
		for (size_t i = 2; i < 6; ++i) {
			if (planes[i].getDistance(center, false) > radius) return false;
		}
		return true;
	}

	void render(float ndcX, float ndcY, Ray ray) {
		glBufferData(GL_ARRAY_BUFFER, 8 * 3 * sizeof(float), vertexFLT, GL_STATIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 8 * 3 * sizeof(GLuint), indices, GL_STATIC_DRAW);

		for (size_t pos = 0; pos < 8 * 3; pos += 3) {
			glUniform4fv(glGetUniformLocation(program, "color"), 1, frustumColor[pos % 2]);
			glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (const void*)(pos * sizeof(GLuint)));
		}

		glm::vec3 v[4] = { {-1, -1, 0}, {-1, 1, 0}, {1, 1, 0}, {1, -1, 0} };
		Plane plane(v);
		Ray r{ glm::vec3(0, 0, 3), glm::vec3(.162, .162, -.97) };
		//std::cout << "inter: " << to_string(plane.getIntersecPoint(r)) << std::endl;
		glm::vec3 res;
		plane.getIntersecPoint(r, res);

		//float line[] = { .75, .75, 0, res.x, res.y, 0 };
		float line[] = { ray.orig.x, ray.orig.y, ray.orig.z - 1, ray.dir.x, ray.dir.y, ray.dir.z };
		//std::cout << "[" << ndcX << "," << ndcY << "]" << std::endl;
		//line[3] = ndcX; line[4] = ndcY;
		glBufferData(GL_ARRAY_BUFFER, 2 * 3 * sizeof(float), line, GL_STATIC_DRAW);
		glDrawArrays(GL_LINE_STRIP, 0, 2);
	}

	void update(glm::vec3 direction, glm::vec3 up, glm::vec3 right, glm::vec3 eyePos, float fov) {

		float t = tan(glm::radians(fov / 2)); // fov default: 90.0f
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

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

	Plane() {};
	//점은 시계방향으로 순서대로 들어온다고 가정
	//즉, 들어오는 점만으로 평면의 방정식을 정의할 수 있음
	Plane(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec3 v3) {
		update(v0, v1, v2, v3);
	}

	void update(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec3 v3) {
		vertices[0] = v0, vertices[1] = v1, vertices[2] = v2, vertices[3] = v3;
		std::cout << glm::to_string(getNormal()) << std::endl;
	}

	glm::vec3 getNormal() {
		auto t = glm::normalize(glm::cross(vertices[1] - vertices[0], vertices[2] - vertices[0]));
		return t;
	}

	bool isPointFront(glm::vec3 v) {
		float d = -glm::dot(getNormal(), vertices[0]);
		return glm::dot(getNormal(), v) + d > .0f;
	}
};

struct PlaneCustom {
	glm::vec3 normal = { 0, 1.0f, 0 };
	float distance = 0;

	PlaneCustom() {};
	PlaneCustom(const glm::vec3& p1, const glm::vec3& _normal) : normal(glm::normalize(_normal)), distance(glm::dot(normal, p1)) {}

	bool getSignedDistanceToPlane(const glm::vec3& point) const
	{
		return (glm::dot(normal, point) - distance) > 0;
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
	
	Plane nearPlane, farPlane, leftPlane, rightPlane, topPlane, bottomPlane;

	float nearZ;
	float farZ;

	Frustum() {};

	Frustum(glm::vec3 direction, glm::vec3 up, glm::vec3 right, glm::vec3 eyePos, float _nearZ, float _farZ, glm::mat4 viewProj) : nearZ(_nearZ), farZ(_farZ) {
		update(direction, up, right, eyePos, viewProj);
	}

	bool inside(glm::vec3 v) {
		bool t0, t1, t2, t3, t4, t5;
		t0 = nearPlane.isPointFront(v);
		t1 = farPlane.isPointFront(v);
		t2 = leftPlane.isPointFront(v);
		t3 = rightPlane.isPointFront(v);
		t4 = topPlane.isPointFront(v);
		t5 = bottomPlane.isPointFront(v);
		//std::cout << t0 << t1 << t2 << t3 << t4 << t5 << std::endl;
		return t0 && t1 && t2 && t3 && t4 && t5;
	}

	void render() {
		//glBufferData(GL_ARRAY_BUFFER, vertexCount * 2 * 3 * sizeof(float), vertices, GL_STATIC_DRAW);
		//glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(GLuint), indices, GL_STATIC_DRAW);

		//glUniform4fv(glGetUniformLocation(program, "color"), 1, objectColor);
	}

	//bool inside(glm::vec3 v) {
	//	bool t0, t1, t2, t3, t4, t5;
	//	t0 = nearPlane.getSignedDistanceToPlane(v);
	//	t1 = farPlane.getSignedDistanceToPlane(v);
	//	t2 = leftPlane.getSignedDistanceToPlane(v);
	//	t3 = rightPlane.getSignedDistanceToPlane(v);
	//	t4 = topPlane.getSignedDistanceToPlane(v);
	//	t5 = bottomPlane.getSignedDistanceToPlane(v);
	//	//std::cout << t0 << t1 << t2 << t3 << t4 << t5 << std::endl;
	//	return t0 && t1 && t2 && t3 && t4 && t5;
	//}

	void update(glm::vec3 direction, glm::vec3 up, glm::vec3 right, glm::vec3 eyePos, glm::mat4 viewProj) {
		auto inv = glm::inverse(viewProj);

		float val = nearZ;
		vertices[0] = eyePos + (-up - right) * val + direction * nearZ;
		vertices[1] = eyePos + (+up - right) * val + direction * nearZ;
		vertices[2] = eyePos + (+up + right) * val + direction * nearZ;
		vertices[3] = eyePos + (-up + right) * val + direction * nearZ;

		val = 1.0f;
		vertices[4] = eyePos + (-up - right) * val * farZ + direction * farZ;
		vertices[5] = eyePos + (+up - right) * val * farZ + direction * farZ;
		vertices[6] = eyePos + (+up + right) * val * farZ + direction * farZ;
		vertices[7] = eyePos + (-up + right) * val * farZ + direction * farZ;

		auto v = vertices;

		nearPlane.update(v[0], v[1], v[2], v[3]);
		farPlane.update(v[4], v[7], v[6], v[5]);
		leftPlane.update(v[4], v[5], v[1], v[0]);
		rightPlane.update(v[3], v[2], v[6], v[7]);
		topPlane.update(v[1], v[5], v[6], v[2]);
		bottomPlane.update(v[0], v[3], v[7], v[4]);
		std::cout << std::endl;


		const float halfVSide = farZ * tanf(1 * .5f);
		const float halfHSide = halfVSide * 1.0f;
		const glm::vec3 frontMultFar = farZ * direction;

		//nearPlane = { eyePos + nearZ * direction, direction };
		//farPlane = { eyePos + frontMultFar, -direction };
		//rightPlane = { eyePos, glm::cross(frontMultFar - right * halfHSide, up) };
		//leftPlane = { eyePos, glm::cross(up, frontMultFar + right * halfHSide) };
		//topPlane = { eyePos, glm::cross(right, frontMultFar - up * halfVSide) };
		//bottomPlane = { eyePos, glm::cross(frontMultFar + up * halfVSide, right) };
	}
};

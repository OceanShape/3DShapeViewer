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
	//[1][2]
	//[0][3]
	glm::vec3 nearVertices[4]{};
	glm::vec3 farVertices[4]{};
	
	PlaneCustom nearPlane, farPlane, leftPlane, rightPlane, topPlane, bottomPlane;

	float nearZ;
	float farZ;

	Frustum() {};

	Frustum(glm::vec3 direction, glm::vec3 up, glm::vec3 right, glm::vec3 eyePos, float _nearZ, float _farZ, glm::mat4 viewProj) : nearZ(_nearZ), farZ(_farZ) {
		update(direction, up, right, eyePos, viewProj);
	}

	//bool inside(glm::vec3 v) {
	//	bool t0, t1, t2, t3, t4, t5;
	//	t0 = nearPlane.isPointFront(v);
	//	t1 = farPlane.isPointFront(v);
	//	t2 = leftPlane.isPointFront(v);
	//	t3 = rightPlane.isPointFront(v);
	//	t4 = topPlane.isPointFront(v);
	//	t5 = bottomPlane.isPointFront(v);
	//	//std::cout << t0 << t1 << t2 << t3 << t4 << t5 << std::endl;
	//	return t0 && t1 && t2 && t3 && t4 && t5;
	//}

	void render() {

	}

	bool inside(glm::vec3 v) {
		bool t0, t1, t2, t3, t4, t5;
		t0 = nearPlane.getSignedDistanceToPlane(v);
		t1 = farPlane.getSignedDistanceToPlane(v);
		t2 = leftPlane.getSignedDistanceToPlane(v);
		t3 = rightPlane.getSignedDistanceToPlane(v);
		t4 = topPlane.getSignedDistanceToPlane(v);
		t5 = bottomPlane.getSignedDistanceToPlane(v);
		//std::cout << t0 << t1 << t2 << t3 << t4 << t5 << std::endl;
		return t0 && t1 && t2 && t3 && t4 && t5;
	}

	void update(glm::vec3 direction, glm::vec3 up, glm::vec3 right, glm::vec3 eyePos, glm::mat4 viewProj) {
		auto inv = glm::inverse(viewProj);

		//nearVertices[0] = glm::vec3(-1.0f, -1.0f, .0f);
		//nearVertices[1] = glm::vec3(-1.0f, 1.0f, .0f);
		//nearVertices[2] = glm::vec3(1.0f, 1.0f, .0f);
		//nearVertices[3] = glm::vec3(1.0f, -1.0f, .0f);

		//farVertices[0] = glm::vec3(-1.0f, -1.0f, 1.0f);
		//farVertices[1] = glm::vec3(-1.0f, 1.0f, 1.0f);
		//farVertices[2] = glm::vec3(1.0f, 1.0f, 1.0f);
		//farVertices[3] = glm::vec3(1.0f, -1.0f, 1.0f);

		//for (int i = 0; i < 4; ++i) {
		//	nearVertices[i] = inv * glm::vec4(nearVertices[i], 1.0f);
		//}
		//for (int i = 0; i < 4; ++i) {
		//	farVertices[i] = inv * glm::vec4(farVertices[i], 1.0f);
		//}
		//-------------------
		//float val = nearZ;
		//nearVertices[0] = eyePos + (-up - right) * val + direction * nearZ;
		//nearVertices[1] = eyePos + (+up - right) * val + direction * nearZ;
		//nearVertices[2] = eyePos + (+up + right) * val + direction * nearZ;
		//nearVertices[3] = eyePos + (-up + right) * val + direction * nearZ;

		//val = 1.0f;
		//farVertices[0] = eyePos + (-up - right) * val * farZ + direction * farZ;
		//farVertices[1] = eyePos + (+up - right) * val * farZ + direction * farZ;
		//farVertices[2] = eyePos + (+up + right) * val * farZ + direction * farZ;
		//farVertices[3] = eyePos + (-up + right) * val * farZ + direction * farZ;

		//auto nv = nearVertices;
		//auto fv = farVertices;

		//nearPlane.update(nv[0], nv[1], nv[2], nv[3]);
		//farPlane.update(fv[0], fv[3], fv[2], fv[1]);
		//leftPlane.update(fv[0], fv[1], nv[1], nv[0]);
		//rightPlane.update(nv[3], nv[2], fv[2], fv[3]);
		//topPlane.update(nv[1], fv[1], fv[2], nv[2]);
		//bottomPlane.update(nv[0], nv[3], fv[3], fv[0]);
		//std::cout << std::endl;


		//for (int i = 0; i < 4; ++i) {
		//	std::cout << glm::to_string(nearVertices[i]) << std::endl;
		//}
		//for (int i = 0; i < 4; ++i) {
		//	std::cout << glm::to_string(farVertices[i]) << std::endl;
		//}
		//std::cout << std::endl;

		const float halfVSide = farZ * tanf(1 * .5f);
		const float halfHSide = halfVSide * 1.0f;
		const glm::vec3 frontMultFar = farZ * direction;

		nearPlane = { eyePos + nearZ * direction, direction };
		farPlane = { eyePos + frontMultFar, -direction };
		rightPlane = { eyePos, glm::cross(frontMultFar - right * halfHSide, up) };
		leftPlane = { eyePos, glm::cross(up, frontMultFar + right * halfHSide) };
		topPlane = { eyePos, glm::cross(right, frontMultFar - up * halfVSide) };
		bottomPlane = { eyePos, glm::cross(frontMultFar + up * halfVSide, right) };
	}

	void printFrustumVertex() {
		std::cout << std::endl;
		for (int i = 0; i < 4; ++i) {
			std::cout << to_string(nearVertices[i]) << std::endl;
		}
		std::cout << "-----" << std::endl;
		for (int i = 0; i < 4; ++i) {
			std::cout << to_string(farVertices[i]) << std::endl;
		}
	}
};

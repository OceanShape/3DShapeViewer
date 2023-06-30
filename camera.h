#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/constants.hpp>

#include "frustum.h"

class Camera {

public:
	int currentLevel = 0;
	int maxLevel = 0;

	float startHeight = .0f;

	GLfloat minTotal[3]{ FLT_MIN, FLT_MIN, FLT_MIN };
	GLfloat maxTotal[3]{ FLT_MAX, FLT_MAX, FLT_MAX };
	GLfloat delTotal[3]{};

	shared_ptr<Frustum> frustum;
	bool frustumCaptured = false;

	glm::mat4 invModelMat;

	Ray ray{};

private:
	RECT rt;

	glm::vec3 position = glm::vec3(0.0f, 3.0f, 3.0f);
	glm::vec3 direction = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 right = glm::vec3(1.0f, .0f, .0f);

	float pitch = .0f;	// x-axis
	float yaw = .0f;	// y-axis
	float roll = .0f;	// z-axis
	float rotDel = 1.0f;

	float delta = 0.05f;
	float deltaZ = 0.1f;

	// projection option
	float fov = 90.0f;
	float nearZ = 0.01f;
	float farZ = 1000.0f;
	float aspect = 1.0f;
	 
	float ndcX = .0f;
	float ndcY = .0f;

public:
	Camera(float posX, float posY, float posZ, glm::mat4 _modelMat) : startHeight(posZ) {
		//position = glm::vec3(1144064.31195f, 1688191.9375f, 900.0f);
		
		/*position = glm::vec3(0, 200.0f, 0);
		up = glm::vec3(0, 0, -1);
		right = glm::vec3(1, 0, 0);
		direction = glm::vec3(0, -1, 0);*/
		
		up = glm::vec3(0, 1, 0);
		right = glm::vec3(1, 0, 0);
		direction = glm::vec3(0, 0, -1);
		position = glm::vec3(0, 0, 1);

		/*invModelMat = glm::inverse(_modelMat);
		position = glm::vec3(posX, posY, posZ);
		*/
		frustum = make_shared<Frustum>(direction, up, right, position, nearZ, farZ, fov, invModelMat);
	}

	void setRect(RECT _rt) {
		rt = _rt;
	}

	glm::mat4 getView() { return glm::lookAt(position, position + direction, up); };
	glm::mat4 getProj() { return glm::perspective(glm::radians(fov), aspect, nearZ, farZ); };
	glm::vec3 getEyePos() { return position; };

	void moveUp(float dt) { position += up * delta * dt; updateMove(); };
	void moveForward(float dt) { position += direction * deltaZ * dt; updateMove(); };
	void moveRight(float dt) { position += right * delta * dt; updateMove(); };

	void setAspectRatio(float _aspect) { aspect = _aspect; };

	void setLevel(int lev) {
		currentLevel = (lev > maxLevel) ? maxLevel : (0 > lev) ? 0 : lev;
	}

	void capture() {
		if (frustumCaptured) return; else frustumCaptured = true;
		std::cout << "captured" << std::endl;
	}

	void uncapture() {
		if (frustumCaptured == false) return; else frustumCaptured = false;
		std::cout << "uncapture" << std::endl;
	}

	void renderFrustum() {
		//auto t1 = nearZ * 1.001f;
		//auto t2 = nearZ * 1.001f;
		//auto t3 = nearZ * 1.001f;
		//test.orig = position + direction * t1;

		//std::cout << ndcX << "," << ndcY << std::endl;
		//test.dir = position + (up * t2) * ndcY + (right * t3) * ndcX + direction * t1;
		//std::cout << to_string(test.dir) << std::endl;

		frustum->render(ndcX, ndcY, ray);
	}

	void updateMove() {
		position.y = (position.y < .05f) ? .05f : position.y;

		// update level
		if (0.0f < position.z && position.z <= startHeight) {
			glm::vec3 res;
			Plane p({ {1, 1, 0}, {1, -1, 0}, {-1, -1, 0}, {-1, 1, 0} });
			p.getIntersecPoint(ray, res);
			float len = glm::length(res - ray.orig);
			len = (len > 3.0f) ? 3.0f : (len < .0f) ? .0f : len;

			float deltaLevel = startHeight / (maxLevel + 1.0f);
			setLevel((3.0f - len) / deltaLevel);
		}

		// update frustum
		if (frustumCaptured == false) {
			frustum->update(direction, up, right, position, fov);
			updateRay();
		}
	}

	void updateRay() {
		auto inv = glm::inverse(glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)));
		ray.orig = inv * glm::vec4{ position, .0f };
		ray.dir = inv * glm::vec4{ glm::normalize(ndcX * .01f * right + ndcY * .01f * up + direction * nearZ), 1.0f };
	}

	Ray getPickingRay(bool isFPS, float currentNdcX, float currentNdcY) {
		if (isFPS) return ray;

		auto inv = glm::inverse(glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)));
		Ray pr{};
		pr.orig = inv * glm::vec4{ position, .0f };
		pr.dir = inv * glm::vec4{ glm::normalize((currentNdcX) * .01f * right + (currentNdcY) * .01f * up + direction * nearZ), 1.0f };
		return pr;
	}

	void getNdc(float& _ndcX, float& _ndcY) {
		_ndcX = ndcX;
		_ndcY = ndcY;
	}

	void updateRotate(float _ndcX, float _ndcY) {
		std::cout << to_string(direction) << std::endl;

		ndcX = _ndcX; ndcY = (_ndcY > .899f) ? .899f : (_ndcY < -.899f) ? -.899f : _ndcY;

		float h_pi = glm::half_pi<float>();
		yaw = ndcX * h_pi;
		pitch = ndcY * h_pi;

		glm::quat qX = glm::angleAxis(pitch, glm::vec3(1, 0, 0));
		glm::quat qY = glm::angleAxis(yaw, glm::vec3(0, -1, 0));
		qX = glm::normalize(qX);
		qY = glm::normalize(qY);

		direction = glm::normalize(qY * qX * glm::vec3(0.0f, 0.0f, -1.0f));
		up = glm::normalize(qY * qX * glm::vec3(0.0f, 1.0f, 0.0f));
		right = glm::normalize(glm::cross(direction, up));

		if (frustumCaptured == false) {
			frustum->update(direction, up, right, position, fov);
			updateRay();
		}
	}
};


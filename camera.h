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

	float startZ = .0f;

	GLfloat minTotal[3]{ FLT_MIN, FLT_MIN, FLT_MIN };
	GLfloat maxTotal[3]{ FLT_MAX, FLT_MAX, FLT_MAX };
	GLfloat delTotal[3]{};

	shared_ptr<Frustum> frustum;
	bool frustumCaptured = false;

	Ray ray{};

	Camera(float posX, float posY, float posZ) : startZ(posZ) {
		position = glm::vec3(posX, posY, posZ);
		frustum = make_shared<Frustum>(direction, up, right, position, nearZ, farZ, fov, getProj() * getView());
	}

	glm::mat4 getView() { return glm::lookAt(position, position + direction, up); };
	glm::mat4 getProj() { return glm::perspective(glm::radians(fov), aspect, nearZ, farZ); };
	glm::vec3 getEyePos() { return position; };

	void updateMouse(float ndcX, float ndcY);
	void updateZoom(float dt);

	void moveUp(float dt) { position += up * delta * dt; update(); };
	void moveForward(float dt) { position += direction * deltaZ * dt; update(); };
	void moveRight(float dt) { position += right * delta * dt; update(); };

	void setAspectRatio(float _aspect) { aspect = _aspect; };

	void setLevel(int lev) {
		currentLevel = (lev > maxLevel) ? maxLevel : (0 > lev) ? 0 : lev;
	}

	void update();

	void capture() {
		if (frustumCaptured) return; else frustumCaptured = true;
		std::cout << "captured" << std::endl;
	}

	void uncapture() {
		if (frustumCaptured == false) return; else frustumCaptured = false;
		std::cout << "uncapture" << std::endl;
	}

	void updateRay(float _ndcX, float _ndcY) {
		ray.orig = position;
		ray.dir = glm::normalize(right * _ndcX / 30.0f + up * _ndcY / 30.0f + direction * nearZ - position);
		ndcX = _ndcX; ndcY = _ndcY;
		//std::cout << "RAY: " << to_string(ray.dir) << std::endl;
	}

	void renderFrustum() {
		Ray test;

		//auto t1 = nearZ * 1.001f;
		//auto t2 = nearZ * 1.001f;
		//auto t3 = nearZ * 1.001f;
		//test.orig = position + direction * t1;

		//std::cout << ndcX << "," << ndcY << std::endl;
		//test.dir = position + (up * t2) * ndcY + (right * t3) * ndcX + direction * t1;
		//std::cout << to_string(test.dir) << std::endl;

		test.orig = position;
		test.dir = glm::normalize(ndcX * .01f * right + ndcY * .01f * up + direction * nearZ);

		frustum->render(ndcX, ndcY, test);
	}

private:
	glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f);
	glm::vec3 direction = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 right = glm::vec3(1.0f, .0f, .0f);

	float pitch	= .0f;	// x-axis
	float yaw	= .0f;	// y-axis
	float roll	= .0f;	// z-axis
	float rotDel = 1.0f;

	float delta = 0.05f;
	float deltaZ = 0.1f;

	// projection option
	float fov = 90.0f;
	float nearZ = 0.01f;
	float farZ = 50.0f;
	float aspect = 1.0f;

	float ndcX = .0f;
	float ndcY = .0f;
};


void Camera::update() {
	// update level
	if (0.0f < position.z && position.z <= startZ) {
		float deltaLevel = startZ / (maxLevel + 1.0f);
		setLevel((3.0f - position.z) / deltaLevel);
		std::cout << "current level: [" << currentLevel << "]" << endl;
	}

	// update frustum
	if (frustumCaptured == false) {
		frustum->update(direction, up, right, position, fov);
		updateRay(ndcX, ndcY);
	}
}

void Camera::updateMouse(float _ndcX, float _ndcY) {
	
	yaw = _ndcX * glm::pi<float>() * 2;
	pitch = _ndcY * glm::pi<float>() / 2;

	glm::quat qX = glm::angleAxis(pitch, glm::vec3(1, 0, 0));
	glm::quat qY = glm::angleAxis(yaw, glm::vec3(0, -1, 0));
	qX = glm::normalize(qX);
	qY = glm::normalize(qY);

	direction = glm::normalize(qY * qX * glm::vec3(0.0f, 0.0f, -1.0f));
	up = glm::normalize(qY * qX * glm::vec3(0.0f, 1.0f, 0.0f));
	right = glm::normalize(glm::cross(direction, up));
	
	if (frustumCaptured == false) {
		frustum->update(direction, up, right, position, fov);
		updateRay(_ndcX, _ndcY);
	}
}

void Camera::updateZoom(float dt) {
	fov -= rotDel * dt;
	fov = (fov > 179.0f) ? 179.0f : (fov < 5.0f) ? 5.0f : fov;
	if (frustumCaptured == false) {
		frustum->update(direction, up, right, position, fov);
	}
	std::cout << fov << std::endl;
}

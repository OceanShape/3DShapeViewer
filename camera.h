#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Camera {

public:
	int currentLevel = 0;
	int maxLevel = 0;

	float startZ = .0f;

	float boundaryX[2] = {};
	float boundaryY[2] = {};

	GLfloat minTotal[3]{ FLT_MIN, FLT_MIN, FLT_MIN };
	GLfloat maxTotal[3]{ FLT_MAX, FLT_MAX, FLT_MAX };
	GLfloat delTotal[3]{};

	Camera(float posX, float posY, float posZ) : startZ(posZ) {
		position = glm::vec3(posX, posY, posZ);
	}

	glm::mat4 getView() { return glm::lookAt(position, position + direction, up); };
	glm::mat4 getProj() { return glm::perspective(glm::radians(fov), aspect, nearZ, farZ); };
	glm::vec3 getEyePos() { return position; };

	void updateMouse(float mouseNdcX, float mouseNdcY);
	void updateKey(float rotX, float rotY, float rotZ);

	void moveUp(float dt) { position += up * delta * dt; updateLevelAndBoundary(); };
	void moveForward(float dt) { position += direction * deltaZ * dt; updateLevelAndBoundary(); };
	void moveRight(float dt) { position += right * delta * dt; updateLevelAndBoundary(); };

	void setAspectRatio(float _aspect) { aspect = _aspect; };

	void setLevel(int lev) {
		currentLevel = (lev > maxLevel) ? maxLevel : lev;
	}

	void setBoundary(float xMin, float xMax, float yMin, float yMax) {
		boundaryX[0] = xMin, boundaryX[1] = xMax;
		boundaryY[0] = yMin, boundaryY[1] = yMax;
	}

	void updateLevelAndBoundary();

private:
	glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f);
	glm::vec3 direction = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 right = glm::vec3(1.0f, .0f, .0f);

	float pitch = .0f;	// x-axis
	float yaw = .0f;	// y-axis
	float roll = .0f;	// z-axis
	float rotDel = 1.0f;

	float delta = 0.01f;
	float deltaZ = 0.1f;

	// projection option
	float fov = 45.0f;
	float nearZ = 0.01f;
	float farZ = 10.0f;
	float aspect = 1.0f;
};

void Camera::updateLevelAndBoundary() {
	if (0.0f < position.z <= startZ) {
		float deltaLevel = startZ / (maxLevel + 1.0f);
		currentLevel = (int)((3.0f - position.z) / deltaLevel);
		std::cout << "current level: [" << currentLevel << "]" << endl;

		// update boundary
		float centerX = (maxTotal[0] + minTotal[0]) / 2 + delTotal[0] * position.x;
		float centerY = (maxTotal[1] + minTotal[1]) / 2 + delTotal[0] * position.y;
		float delBoundary = delTotal[0] * position.z * (tan(fov / 2)) / 1.1f;
		setBoundary(centerX - delBoundary, centerX + delBoundary, centerY - delBoundary, centerY + delBoundary);
	}
}

void Camera::updateMouse(float mouseNdcX, float mouseNdcY) {

}

void Camera::updateKey(float rotX, float rotY, float rotZ) {
	pitch += rotX * rotDel;
	yaw += rotY * rotDel;
	roll += rotZ * rotDel;

	glm::quat qX = glm::angleAxis(glm::radians(pitch), glm::vec3(1, 0, 0));
	glm::quat qY = glm::angleAxis(glm::radians(yaw), glm::vec3(0, 1, 0));
	glm::quat qZ = glm::angleAxis(glm::radians(roll), glm::vec3(0, 0, 1));
	qX = glm::normalize(qX);
	qY = glm::normalize(qY);
	qZ = glm::normalize(qZ);

	direction = glm::normalize(qZ * qY * qX * glm::vec3(0.0f, 0.0f, -1.0f));
	up = glm::normalize(qZ * qY * qX * glm::vec3(0.0f, 1.0f, 0.0f));
	right = glm::normalize(glm::cross(direction, up));

	updateLevelAndBoundary();
}

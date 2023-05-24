#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/constants.hpp>

class Camera {

public:
	int currentLevel = 0;
	int maxLevel = 0;

	float startZ = .0f;

	GLfloat minTotal[3]{ FLT_MIN, FLT_MIN, FLT_MIN };
	GLfloat maxTotal[3]{ FLT_MAX, FLT_MAX, FLT_MAX };
	GLfloat delTotal[3]{};

	Camera(float posX, float posY, float posZ) : startZ(posZ) {
		position = glm::vec3(posX, posY, posZ);
	}

	glm::mat4 getView() { return glm::lookAt(position, position + direction, up); };
	glm::mat4 getProj() { return glm::perspective(glm::radians(fov), aspect, nearZ, farZ); };
	glm::vec3 getEyePos() { return position; };

	void updateMouse(float ndcX, float ndcY);
	void updateZoom(float dt);

	void moveUp(float dt) { position += up * delta * dt; updateLevelAndBoundary(); };
	void moveForward(float dt) { position += direction * delta * dt; updateLevelAndBoundary(); };
	void moveRight(float dt) { position += right * delta * dt; updateLevelAndBoundary(); };

	void setAspectRatio(float _aspect) { aspect = _aspect; };

	void setLevel(int lev) {
		currentLevel = (lev > maxLevel) ? maxLevel : lev;
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

void Camera::updateMouse(float ndcX, float ndcY) {
	yaw = ndcX * glm::pi<float>() * 2;
	pitch = ndcY * glm::pi<float>() / 2;

	glm::quat qX = glm::angleAxis(pitch, glm::vec3(1, 0, 0));
	glm::quat qY = glm::angleAxis(yaw, glm::vec3(0, -1, 0));
	qX = glm::normalize(qX);
	qY = glm::normalize(qY);

	direction = glm::normalize(qY * qX * glm::vec3(0.0f, 0.0f, -1.0f));
	up = glm::normalize(qY * qX * glm::vec3(0.0f, 1.0f, 0.0f));
	right = glm::normalize(glm::cross(direction, up));
}

void Camera::updateZoom(float dt) {
	fov -= rotDel * dt;
	fov = (fov > 89.0f) ? 89.0f : (fov < 5.0f) ? 5.0f : fov;
	std::cout << fov << std::endl;
}

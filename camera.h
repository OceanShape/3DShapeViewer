#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

class Camera {

public:
	Camera(float posX, float posY, float posZ) {
		position = vec3(posX, posY, posZ);
	}

	mat4 getView() { return lookAt(position, position + direction, up); };
	mat4 getProj() { return perspective(radians(projFov), aspect, nearZ, farZ); };
	vec3 getEyePos() { return position; };

	void updateMouse(float mouseNdcX, float mouseNdcY);
	void updateKey(float rotX, float rotY, float rotZ);
	void moveForward(float dt) { position += direction * delta * dt; };
	void moveRight(float dt) { position += right * delta * dt; };
	void setAspectRatio(float _aspect) { aspect = _aspect; };

private:
	vec3 position = vec3(0.0f, 0.0f, 3.0f);
	vec3 direction = vec3(0.0f, 0.0f, -1.0f);
	vec3 up = vec3(0.0f, 1.0f, 0.0f);
	vec3 right = vec3(1.0f, .0f, .0f);

	float pitch = .0f;
	float yaw = .0f;
	float roll = .0f;

	float delta = 0.01f;

	// projection option
	float projFov = 45.0f;
	float nearZ = 0.01f;
	float farZ = 10.0f;
	float aspect = 1.0f;
};

void Camera::updateMouse(float mouseNdcX, float mouseNdcY) {

}

void Camera::updateKey(float rotX, float rotY, float rotZ) {

}

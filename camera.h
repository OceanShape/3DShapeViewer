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

	Ray ray{};

	glm::vec3 interPoint = glm::vec3(.0f);
	bool isLButtonFirstDown = true;

private:
	RECT rt;

	glm::vec3 upF = glm::vec3(0, 0, 1.0f);
	glm::vec3 rightF = glm::vec3(1.0f, 0, 0);
	glm::vec3 directionF = glm::vec3(0, 1.0f, 0);
	glm::vec3 positionF = glm::vec3(.0f);

	glm::vec3 up = upF;
	glm::vec3 right = rightF;
	glm::vec3 direction = directionF;
	glm::vec3 position = positionF;

	float pitch = .0f;	// x-axis
	float yaw = .0f;	// y-axis
	float roll = .0f;	// z-axis
	float rotDel = 1.0f;

	float delta = 200.0f;

	// projection option
	float fov = 90.0f;
	float nearZ = 1.0f;
	float farZ = 30000.0f;
	float aspect = 1.0f;

	float ndcX = .0f;
	float ndcY = .0f;

public:
	Camera(float posX, float posY, float posZ, glm::mat4 _modelMat) : startHeight(posZ) {
		/*invModelMat = glm::inverse(_modelMat);
		position = glm::vec3(posX, posY, posZ);
		*/
		frustum = make_shared<Frustum>(direction, up, right, position, nearZ, farZ, fov, glm::inverse(getProj() * getView()));
	}

	void setPos() {
		position = glm::vec3((minTotal[0] + maxTotal[0]) / 2.0f, minTotal[1], (maxTotal[0] - minTotal[0]) / (2.0f * 10.0f));
	}

	void setRect(RECT _rt) {
		rt = _rt;
	}

	glm::mat4 getView() { return glm::lookAt(position, position + direction, up); };
	glm::mat4 getProj() { return glm::perspective(glm::radians(fov), aspect, nearZ, farZ); };
	glm::vec3 getEyePos() { return position; };

	void moveUp(float dt) { position += up * delta * dt; updateMove(); };
	void moveForward(float dt) { position += direction * delta * dt; updateMove(); };
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
		//float t1 = nearZ * 1.001f;
		//float t2 = nearZ * 1.001f;
		//float t3 = nearZ * 1.001f;
		//test.orig = position + direction * t1;

		//std::cout << ndcX << "," << ndcY << std::endl;
		//test.dir = position + (up * t2) * ndcY + (right * t3) * ndcX + direction * t1;
		//std::cout << to_string(test.dir) << std::endl;
		//if (frustumCaptured == true) 
		frustum->render(ndcX, ndcY, ray);
	}

	void resetCamera() {
		up = upF;
		right = rightF;
		direction = directionF;
		frustum->update(direction, up, right, position, fov, getProj() * getView());
		updateRay(ndcX, ndcY);
	}

	void updateMove() {
		position.y = (position.y < .05f) ? .05f : position.y;

		// update level
		//if (0.0f < position.z && position.z <= startHeight) {
		//	std::cout << "T" << std::endl;
		//	glm::vec3 res;
		//	Plane p({ {1, 1, 0}, {1, -1, 0}, {-1, -1, 0}, {-1, 1, 0} });
		//	p.getIntersecPoint(ray, res);
		//	float len = glm::length(res - ray.orig);
		//	len = (len > 3.0f) ? 3.0f : (len < .0f) ? .0f : len;

		//	float deltaLevel = startHeight / (maxLevel + 1.0f);
		//	setLevel((3.0f - len) / deltaLevel);
		//}

		// update frustum
		if (frustumCaptured == false) {
			frustum->update(direction, up, right, position, fov, getProj() * getView());
			updateRay(ndcX, ndcY);
		}
	}

	void getNdc(float& _ndcX, float& _ndcY) {
		_ndcX = ndcX;
		_ndcY = ndcY;
	}

	void updateRotate(float _ndcX, float _ndcY) {
		ndcX = _ndcX;
		ndcY = _ndcY;

		/*ndcX = _ndcX; ndcY = (_ndcY > .899f) ? .899f : (_ndcY < -.899f) ? -.899f : _ndcY;*/

		float h_pi = glm::half_pi<float>();
		yaw = ndcX * h_pi;
		pitch = ndcY * h_pi;

		glm::quat qX = glm::angleAxis(yaw, glm::vec3(0, 0, -1.0f));
		qX = glm::normalize(qX);
		glm::quat qY = glm::angleAxis(pitch, qX * glm::vec3(1.0f, 0, 0));
		qY = glm::normalize(qY);

		direction = glm::normalize(qY * qX * directionF);
		up = glm::normalize(qY * qX * upF);
		right = glm::normalize(glm::cross(direction, up));

		if (frustumCaptured == false) {
			frustum->update(direction, up, right, position, fov, getProj() * getView());
			updateRay(ndcX, ndcY);
		}
	}

	// rotate, move 둘 다 ndc는 변하지 않음
	// 현재 frustum과 ndc에서 ray 업데이트하는 함수
	void updateRay(float ndcX, float ndcY) {
		ray.orig = glm::vec4{ position, .0f };
		ray.dir = glm::vec4{ glm::normalize(ndcX * 1.0f * right + ndcY * 1.0f * up + direction * nearZ), 1.0f };
	}

	Ray calculateRay(float ndcX, float ndcY) {
		Ray r;
		r.orig = glm::vec4{ position, .0f };
		r.dir = glm::vec4{ glm::normalize(ndcX * 1.0f * right + ndcY * 1.0f * up + direction * nearZ), 1.0f };
		return r;
	}

	void updateRotateTPS(float _ndcX, float _ndcY, float _ndcFPSX, float _ndcFPSY) {

		float delX = ndcX - _ndcX;
		float delY = ndcY - _ndcY;

		float h_pi = glm::half_pi<float>();
		float thetaX = delX * h_pi;
		float thetaY = delY * h_pi;
		
		
		// .0f면 처음 클릭한 상태
		if (isLButtonFirstDown == true) {
			Plane p({ {minTotal[0] / 10.0f, minTotal[1] / 10.0f, 0}, {minTotal[0] / 10.0f, maxTotal[1] / 10.0f, 0},
		  {maxTotal[0] / 10.0f, maxTotal[1] / 10.0f, 0}, {maxTotal[0] / 10.0f, minTotal[1] / 10.0f, 0} });
			Ray rayCont = calculateRay(_ndcFPSX, _ndcFPSY);
			rayCont.orig = ray.orig / 10.0f;

			bool ans = p.getIntersecPoint(rayCont, interPoint);
			interPoint *= 10.0f;
			std::cout << to_string(interPoint * 10.0f) << std::endl;
			if (ans == false) {
				updateRay(ndcX, ndcY);
				interPoint = glm::vec3(.0f);
				return;
			}
			isLButtonFirstDown = false;
		}

		// rotate
		{
			glm::mat4 rotZ = glm::rotate(glm::mat4(1.0f), thetaX, glm::vec3(0.0f, 0.0f, -1.0f));

			right = glm::normalize(rotZ * glm::vec4(right, 0.0f));
		
			glm::vec3 pos = position;
			pos.x -= interPoint.x; pos.y -= interPoint.y;
			pos = rotZ * glm::vec4(pos, 1.0f);

			glm::mat4 rotX = glm::rotate(glm::mat4(1.0f), thetaY, right);

			pos = rotX * glm::vec4(pos, 1.0f);
			pos.x += interPoint.x; pos.y += interPoint.y;
			position = pos;

			direction = glm::normalize(rotX * rotZ * glm::vec4(direction, 0.0f));
			up = glm::normalize(glm::cross(right, direction));
		}

		if (frustumCaptured == false) {
			frustum->update(direction, up, right, position, fov, getProj() * getView());
			updateRay(ndcX, ndcY);
		}

		ndcX = _ndcX;
		ndcY = _ndcY;
	};
};

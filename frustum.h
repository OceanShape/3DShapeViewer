#pragma once
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

struct Frustum {
	//[3][0]
	//[2][1]
	glm::vec3 planeNear[4]{};
	glm::vec3 planeFar[4]{};
	Frustum(glm::vec3 direction, glm::vec3 up, glm::vec3 right, glm::vec3 eyePos, float nearZ, float farZ) {
		update(direction, up, right, eyePos, nearZ, farZ);
	}

	void update(glm::vec3 direction, glm::vec3 up, glm::vec3 right, glm::vec3 eyePos, float nearZ, float farZ) {
		planeNear[0] = eyePos + up + right + direction * nearZ;
		planeNear[1] = eyePos - up + right + direction * nearZ;
		planeNear[2] = eyePos - up - right + direction * nearZ;
		planeNear[3] = eyePos + up - right + direction * nearZ;

	//	planeFar[0] = eyePos + up + right + direction * farZ;
	//	planeFar[1] = eyePos - up + right + direction * farZ;
	//	planeFar[2] = eyePos - up - right + direction * farZ;
	//	planeFar[3] = eyePos + up - right + direction * farZ;
	}
};
#pragma once
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

struct Frustum {
	//[1][2]
	//[0][3]
	glm::vec3 planeNear[4]{};
	glm::vec3 planeFar[4]{};
	

	float nearZ;
	float farZ;
	Frustum(glm::vec3 direction, glm::vec3 up, glm::vec3 right, glm::vec3 eyePos, float _nearZ, float _farZ) : nearZ(_nearZ), farZ(_farZ) {
		update(direction, up, right, eyePos);
	}

	void update(glm::vec3 direction, glm::vec3 up, glm::vec3 right, glm::vec3 eyePos) {
		planeNear[0] = eyePos - up - right + direction * nearZ; 
		planeNear[1] = eyePos + up - right + direction * nearZ;
		planeNear[2] = eyePos + up + right + direction * nearZ;
		planeNear[3] = eyePos - up + right + direction * nearZ; 

		planeFar[0] = eyePos + (-up - right) * farZ / nearZ + direction * farZ;
		planeFar[1] = eyePos + (+up - right) * farZ / nearZ + direction * farZ;
		planeFar[2] = eyePos + (+up + right) * farZ / nearZ + direction * farZ; 
		planeFar[3] = eyePos + (-up + right) * farZ / nearZ + direction * farZ; 
	}

	void printFrustumVertex() {
		std::cout << std::endl;
		for (int i = 0; i < 4; ++i) {
			std::cout << to_string(planeNear[i]) << std::endl;
		}
		std::cout << "-----" << std::endl;
		for (int i = 0; i < 4; ++i) {
			std::cout << to_string(planeFar[i]) << std::endl;
		}
	}
};

#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include <cstdlib>

#include "shapedata.h"
#include "object.h"
#include "utility.h"
#include "frustum.h"
#include "camera.h"

using namespace std;

class QuadtreeNode {
	friend class ObjectData;

	static int nodeCount;
	static int selectLevel;

	static RenderOption renderOption;
	static shared_ptr<Frustum> frustum;
	static vector<shared_ptr<QuadtreeNode>> nodeList;
	static vector<shared_ptr<Object>> selectedObjectList;
	static vector<int> selectedObjectListLevel;
	static Ray ray;
	static shared_ptr<Object> selectedObject;
	static vector<int> objectCount;
	static int renderObjectCount;

	bool culled;

	const int level;
	static const float halfRadiusRatio;
	const float radiusWorld;
	static float boundaryX[2];
	static float boundaryY[2];

	vector<shared_ptr<Object>> objects;

	vector<float> sphereVertices;
	vector<GLuint> sphereIndices;

	float Xmin, Xmax, Ymin, Ymax;
	float Xmid, Ymid;
	glm::vec3 centerWorld;
	// min max buffer for objects
	float _min[2] = { FLT_MAX, FLT_MAX };
	float _max[2] = { FLT_MIN, FLT_MIN };
	glm::vec3 box[4];

	shared_ptr<QuadtreeNode> nodes[4] = { nullptr, nullptr, nullptr, nullptr };
public:
	QuadtreeNode(const int& _level, const float& _Xmin, const float& _Xmax, const float& _Ymin, const float& _Ymax) : level(_level), radiusWorld(halfRadiusRatio / (1 << _level)), Xmin(_Xmin), Xmax(_Xmax), Ymin(_Ymin), Ymax(_Ymax), Xmid((_Xmin + _Xmax) / 2), Ymid((_Ymin + _Ymax) / 2) {
		centerWorld = modelToWorldPos(glm::vec3(Xmid, Ymid, .0f));
		++nodeCount;
		culled = true;

		float half = 1 / (1 << level);
		float x = centerWorld.x;
		float y = centerWorld.y;
		box[0] = glm::vec3{ x - half, y - half, .0f };
		box[1] = glm::vec3{ x - half, y + half, .0f };
		box[2] = glm::vec3{ x + half, y + half, .0f };
		box[3] = glm::vec3{ x + half, y - half, .0f };

		if (level <= 0) makeSphere(centerWorld, radiusWorld);
	}

private:
	bool isLeft(float x) {
		return Xmin <= x && x <= Xmid;
	}
	bool isRight(float x) {
		return Xmid <= x && x <= Xmax;
	}
	bool isUp(float y) {
		return Ymid <= y && y <= Ymax;
	}
	bool isDown(float y) {
		return Ymin <= y && y <= Ymid;
	}

	void store(const shared_ptr<Object> obj, int level, int& maxLevel) {
		if (Xmax - Xmin < 128) {
			objects.push_back(obj);
			objectCount[level]++;
			maxLevel = max(maxLevel, level);
			return;
		}

		float x = obj->center.x, y = obj->center.y;
		if (isLeft(x) && isUp(y)) {
			if (nodes[0] == nullptr) {
				nodes[0] = make_shared<QuadtreeNode>(level + 1, Xmin, Xmid, Ymid, Ymax);
				qtNode::nodeList.push_back(nodes[0]);
			}
			nodes[0]->store(obj, level + 1, maxLevel);
		}
		else if (isRight(x) && isUp(y)) {
			if (nodes[1] == nullptr) {
				nodes[1] = make_shared<QuadtreeNode>(level + 1, Xmid, Xmax, Ymid, Ymax);
				qtNode::nodeList.push_back(nodes[1]);
			}
			nodes[1]->store(obj, level + 1, maxLevel);
		}
		else if (isLeft(x) && isDown(y)) {
			if (nodes[2] == nullptr) {
				nodes[2] = make_shared<QuadtreeNode>(level + 1, Xmin, Xmid, Ymin, Ymid);
				qtNode::nodeList.push_back(nodes[2]);
			}
			nodes[2]->store(obj, level + 1, maxLevel);
		}
		else if (isRight(x) && isDown(y)) {
			if (nodes[3] == nullptr) {
				nodes[3] = make_shared<QuadtreeNode>(level + 1, Xmid, Xmax, Ymin, Ymid);
				qtNode::nodeList.push_back(nodes[3]);
			}
			nodes[3]->store(obj, level + 1, maxLevel);
		}
		else {
			objects.push_back(obj);
			objectCount[level]++;
			maxLevel = max(maxLevel, level);
		}
	}

	glm::vec3 modelToWorldPos(const glm::vec3& v) {
		return glm::vec3(
			2 * (v.x - boundaryX[0]) / (boundaryX[1] - boundaryX[0]) - 1,
			2 * (v.y - boundaryY[0]) / (boundaryY[1] - boundaryY[0]) - 1,
			v.z
		);
	}

	float modelToWorldLen(const float& len) {
		return len * 2.0f / (boundaryX[1] - boundaryX[0]);
	}
	
	FRUSTUM_CULLING insideFrustum() {
		auto cullingState = FRUSTUM_CULLING::_OUT;
		int count = 0;
		for (size_t i = 0; i < 4; ++i) if (frustum->inside(box[i])) count++;

		cullingState = (count == 4) ? FRUSTUM_CULLING::_COMPLETE : (count > 0) ? FRUSTUM_CULLING::_PARTIAL : frustum->inSphere(centerWorld, radiusWorld) ? FRUSTUM_CULLING::_PARTIAL : FRUSTUM_CULLING::_OUT;

		return cullingState;
	}

	void update() {
		if (level > selectLevel) {
			culled = true;
		}
		else {
			// Frustum culling
			// 피킹할 경우, 해당 오브젝트 색상을 변경해야 하기 때문
			// 노드의 오브젝트를 확인할 때에도, 나중에 확인
			auto cullingState = insideFrustum();

			switch (cullingState) {
			case FRUSTUM_CULLING::_COMPLETE:
				culled = false; break;
			case FRUSTUM_CULLING::_PARTIAL:
				culled = false; break;
			case FRUSTUM_CULLING::_OUT:
				culled = true; break;
			}

			if (glm::length(centerWorld - ray.orig) > .6f) culled = true;
		}

		for (auto n : nodes) {
			if (n != nullptr) n->update();
		}
	}

	void render() {

		for (auto n : nodes) {
			if (n != nullptr) n->render();
		}

		if (culled) return;
		drawBorder();
		drawObject();

		// draw selected objects
		if (level == 0) {
			float min = FLT_MAX;
			shared_ptr<Object> pickedObj;
			for (auto obj : selectedObjectList) {
				float tmp = glm::length(ray.orig - modelToWorldPos(obj->center));
				if (min > tmp) {
					min = tmp; pickedObj = obj;
				}
			}

			for (size_t i = 0; i < selectedObjectList.size(); ++i) {
				if (pickedObj == selectedObjectList[i]) selectedObject = selectedObjectList[i];
				selectedObjectList[i]->render(pickedObj == selectedObjectList[i], selectedObjectListLevel[i]);
			}
		}
	}

	void drawObject() {
		glUseProgram(renderOption.program[0]);
		glBindVertexArray(renderOption.vao[0]);
		glBindBuffer(GL_ARRAY_BUFFER, renderOption.vbo[0]);


		for (auto obj : objects) {
			auto cenW = modelToWorldPos(obj->center);
			auto radW = modelToWorldLen(obj->radius);
			
			if (frustum->inSphere(cenW, radW)
				&& glm::length(ray.orig - cenW) < .6f) {

				renderObjectCount++;

				auto d = glm::length(glm::cross(ray.dir, ray.orig - cenW)) / glm::length(ray.dir);

				if (d < radW && isDetected(obj)) {
					selectedObjectList.push_back(obj);
					selectedObjectListLevel.push_back(level);
				}
				else obj->render(false, level);
			}
		}
	}

	bool isDetected(const shared_ptr<Object> obj) {
		glm::vec3 inter;
		glm::vec3 triVertices[3];
		auto v = obj->vertices;
		auto idx = obj->indices;
		for (size_t i = 0; i < obj->triangleCount; ++i) {
			triVertices[0] = modelToWorldPos(v[idx[i * 3]]);
			triVertices[1] = modelToWorldPos(v[idx[i * 3 + 1]]);
			triVertices[2] = modelToWorldPos(v[idx[i * 3 + 2]]);
			if (isRayIntersecTriangle(ray, inter, triVertices)) return true;
		}

		int startIdx = obj->triangleCount * 3;
		for (int i = 0; i < obj->vertexCount - 1; ++i) {
			triVertices[0] = modelToWorldPos(v[idx[startIdx + i * 6]]);
			triVertices[1] = modelToWorldPos(v[idx[startIdx + i * 6 + 1]]);
			triVertices[2] = modelToWorldPos(v[idx[startIdx + i * 6 + 2]]);
			if (isRayIntersecTriangle(ray, inter, triVertices)) return true;

			triVertices[0] = modelToWorldPos(v[idx[startIdx + i * 6 + 3]]);
			triVertices[1] = modelToWorldPos(v[idx[startIdx + i * 6 + 4]]);
			triVertices[2] = modelToWorldPos(v[idx[startIdx + i * 6 + 5]]);
			if (isRayIntersecTriangle(ray, inter, triVertices)) return true;
		}

		return false;
	}

	void makeSphere(const glm::vec3& cw, const float& rw) {
		/*sphereVertices.reserve(1323);
		sphereIndices.reserve(2400);*/

		int numSlices = 20;
		int numStacks = 20;
		const float dTheta = glm::pi<float>() * 2 / float(numSlices);
		const float dPhi = glm::pi<float>() / float(numStacks);

		for (int j = 0; j <= numStacks; j++) {

			glm::mat4 transformX = glm::rotate(glm::mat4(1.0), dPhi * float(j), glm::vec3(1, 0, 0));
			glm::vec3 stackStartPoint = transformX * glm::vec4(0, -rw, 0, 1);

			for (int i = 0; i <= numSlices; i++) {
				glm::mat4 transformY = glm::rotate(glm::mat4(1.0), dTheta * float(i), glm::vec3(0, 1, 0));
				glm::vec3 res = transformY * glm::vec4(stackStartPoint, 1.0f);
				sphereVertices.push_back(res.x + cw.x);
				sphereVertices.push_back(res.y + cw.y);
				sphereVertices.push_back(res.z + cw.z);
			}
		}

		for (int j = 0; j < numStacks; j++) {
			const int offset = (numSlices + 1) * j;
			for (int i = 0; i < numSlices; i++) {

				sphereIndices.push_back(offset + i);
				sphereIndices.push_back(offset + i + numSlices + 1);
				sphereIndices.push_back(offset + i + 1 + numSlices + 1);

				sphereIndices.push_back(offset + i);
				sphereIndices.push_back(offset + i + 1 + numSlices + 1);
				sphereIndices.push_back(offset + i + 1);
			}
		}
	}

	void drawSphere() {

		glUseProgram(renderOption.program[2]);
		glBindVertexArray(renderOption.vao[2]);
		glBindBuffer(GL_ARRAY_BUFFER, renderOption.vbo[2]);

		const float color[4] = { 0.5f, 1.0f, 0.5f, .5f };

		glUniform4fv(glGetUniformLocation(renderOption.program[2], "color"), 1, color);

		glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(float), sphereVertices.data(), GL_STATIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(GLuint), sphereIndices.data(), GL_STATIC_DRAW);
		glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0);
	}

	void drawBorder() {
		glUseProgram(renderOption.program[1]);
		glBindVertexArray(renderOption.vao[1]);
		glBindBuffer(GL_ARRAY_BUFFER, renderOption.vbo[1]);

		float border[] = { Xmin, Ymin, 0.0f, Xmin, Ymax, 0.0f, Xmax, Ymax, 0.0f, Xmax, Ymin, 0.0f };
		glBufferData(GL_ARRAY_BUFFER, 4 * 3 * sizeof(float), border, GL_STATIC_DRAW);
		glDrawArrays(GL_LINE_LOOP, 0, 4);
	}
} typedef qtNode;

int qtNode::nodeCount = 0;
int qtNode::selectLevel = 0;
float qtNode::boundaryX[]{};
float qtNode::boundaryY[]{};
const float qtNode::halfRadiusRatio = 1.414213562f;
RenderOption qtNode::renderOption = { 0, };
shared_ptr<Frustum> qtNode::frustum = nullptr;
vector<shared_ptr<QuadtreeNode>> qtNode::nodeList;
vector<shared_ptr<Object>> qtNode::selectedObjectList;
vector<int> qtNode::selectedObjectListLevel;
shared_ptr<Object> qtNode::selectedObject;
vector<int> qtNode::objectCount(10);
int qtNode::renderObjectCount = 0;
Ray qtNode::ray;


class ObjectData {
	
	static int selectObject;
	shared_ptr<qtNode> root;

public:
	ObjectData(float min[], float max[]) {
		qtNode::boundaryX[0] = min[0], qtNode::boundaryX[1] = max[0];
		qtNode::boundaryY[0] = min[1], qtNode::boundaryY[1] = max[1];
		root = make_shared<qtNode>(0, min[0], max[0], min[1], max[1]);
		qtNode::nodeList.push_back(root);
	}

	void setRenderOpiton(const RenderOption& renderOption) {
		std::copy(&renderOption, &renderOption + 1, &qtNode::renderOption);
		Object::program = renderOption.program[0];
	}

	void store(const shared_ptr<Object> obj, int& maxLevel) {
		root->store(obj, 0, maxLevel);
	}

	void update(int selectLevel, shared_ptr<Frustum> frustum, Ray ray) {
		qtNode::frustum = frustum;
		qtNode::selectLevel = selectLevel;
		qtNode::ray = ray;
		root->update();
		root->culled = false;
	}

	int getObjectCount(int i) {
		return qtNode::objectCount[i];
	}

	void render(int selectLevel) {
		qtNode::selectLevel = selectLevel;
		qtNode::selectedObject = nullptr;
		qtNode::renderObjectCount = 0;
		root->render();
		qtNode::selectedObjectList.clear();
		qtNode::selectedObjectListLevel.clear();
	}

	int getRenderedObject() {
		return qtNode::renderObjectCount;
	}

	shared_ptr<Object> getSelectedObject() {
		return qtNode::selectedObject;
	}
};

int ObjectData::selectObject = 0;

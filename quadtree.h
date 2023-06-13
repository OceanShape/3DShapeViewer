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

	shared_ptr<QuadtreeNode> nodes[4] = { nullptr, nullptr, nullptr, nullptr };
public:
	QuadtreeNode(const int& _level, const float& _Xmin, const float& _Xmax, const float& _Ymin, const float& _Ymax) : level(_level), radiusWorld(halfRadiusRatio / (1 << _level)), Xmin(_Xmin), Xmax(_Xmax), Ymin(_Ymin), Ymax(_Ymax), Xmid((_Xmin + _Xmax) / 2), Ymid((_Ymin + _Ymax) / 2) {
		centerWorld = modelToWorldPos(Xmid, Ymid);
		++nodeCount;
		if (level <= 0) makeSphere(centerWorld, radiusWorld);
	}

private:
	bool isLeft() {
		return Xmin <= _min[0] && Xmin <= _max[0] && _min[0] <= Xmid && _max[0] <= Xmid;
	}
	bool isRight() {
		return Xmid <= _min[0] && Xmid <= _max[0] && _min[0] <= Xmax && _max[0] <= Xmax;
	}
	bool isUp() {
		return Ymid <= _min[1] && Ymid <= _max[1] && _min[1] <= Ymax && _max[1] <= Ymax;
	}
	bool isDown() {
		return Ymin <= _min[1] && Ymin <= _max[1] && _min[1] <= Ymid && _max[1] <= Ymid;
	}

	void store(const shared_ptr<Object> obj, int level, int& maxLevel) {
		if (Xmax - Xmin < 120) {
			objects.push_back(obj);
			maxLevel = max(maxLevel, level);
			return;
		}

		_min[0] = obj->min[0], _min[1] = obj->min[1], _max[0] = obj->max[0], _max[1] = obj->max[1];
		if (isLeft() && isUp()) {
			if (nodes[0] == nullptr) {
				nodes[0] = make_shared<QuadtreeNode>(level + 1, Xmin, Xmid, Ymid, Ymax);
				qtNode::nodeList.push_back(nodes[0]);
			}
			nodes[0]->store(obj, level + 1, maxLevel);
		}
		else if (isRight() && isUp()) {
			if (nodes[1] == nullptr) {
				nodes[1] = make_shared<QuadtreeNode>(level + 1, Xmid, Xmax, Ymid, Ymax);
				qtNode::nodeList.push_back(nodes[1]);
			}
			nodes[1]->store(obj, level + 1, maxLevel);
		}
		else if (isLeft() && isDown()) {
			if (nodes[2] == nullptr) {
				nodes[2] = make_shared<QuadtreeNode>(level + 1, Xmin, Xmid, Ymin, Ymid);
				qtNode::nodeList.push_back(nodes[2]);
			}
			nodes[2]->store(obj, level + 1, maxLevel);
		}
		else if (isRight() && isDown()) {
			if (nodes[3] == nullptr) {
				nodes[3] = make_shared<QuadtreeNode>(level + 1, Xmid, Xmax, Ymin, Ymid);
				qtNode::nodeList.push_back(nodes[3]);
			}
			nodes[3]->store(obj, level + 1, maxLevel);
		}
		else {
			objects.push_back(obj);
			maxLevel = max(maxLevel, level);
		}
	}

	/* for object
	void AABBOX(glm::vec3 vertices[]) {
		vertices[0] = glm::vec3{ Xmin, Ymin, .0f };
		vertices[1] = glm::vec3{ Xmin, Ymax, .0f };
		vertices[2] = glm::vec3{ Xmax, Ymax, .0f };
		vertices[3] = glm::vec3{ Xmax, Ymin, .0f };

		vertices[4] = glm::vec3{ Xmin, Ymin, 1.0f };
		vertices[5] = glm::vec3{ Xmin, Ymax, 1.0f };
		vertices[6] = glm::vec3{ Xmax, Ymax, 1.0f };
		vertices[7] = glm::vec3{ Xmax, Ymin, 1.0f };
	}
	*/

	glm::vec3 modelToWorldPos(float x, float y) {
		float res[3];
		res[0] = 2 * (x - boundaryX[0]) / (boundaryX[1] - boundaryX[0]) - 1;
		res[1] = 2 * (y - boundaryY[0]) / (boundaryY[1] - boundaryY[0]) - 1;
		res[2] = 0.005f;

		return glm::vec3(res[0], res[1], res[2]);
	}

	float modelToWorldLen(float len) {
		return len / (boundaryX[1] - boundaryX[0]);
	}

	void getBorderVertex(glm::vec3* const box) {
		float half = 1 / (1 << level);
		float x = centerWorld.x;
		float y = centerWorld.y;
		box[0] = glm::vec3{ x - half, y - half, .0f };
		box[1] = glm::vec3{ x - half, y + half, .0f };
		box[2] = glm::vec3{ x + half, y + half, .0f };
		box[3] = glm::vec3{ x + half, y - half, .0f };
	}
	 
	FRUSTUM_CULLING insideFrustum() {
		auto cullingState = FRUSTUM_CULLING::_OUT;

		glm::vec3 box[4];
		getBorderVertex(box);
		int count = 0;
		for (size_t i = 0; i < 4; ++i) if (frustum->inside(box[i])) count++;

		cullingState = (count == 4) ? FRUSTUM_CULLING::_COMPLETE : (count > 0) ? FRUSTUM_CULLING::_PARTIAL : frustum->inSphere(centerWorld, radiusWorld) ? FRUSTUM_CULLING::_PARTIAL : FRUSTUM_CULLING::_OUT;

		return cullingState;
	}

	void update(int level) {
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
		}

		for (auto n : nodes) {
			if (n != nullptr) n->update(level + 1);
		}
	}

	void render(int level) {
		if (culled) return;
		//if (level == 0) drawSphere();

		for (auto n : nodes) {
			if (n != nullptr) n->render(level + 1);
		}

		drawBorder();
		drawObject();
	}

	void drawObject() {
		glUseProgram(renderOption.program[0]);
		glBindVertexArray(renderOption.vao[0]);
		glBindBuffer(GL_ARRAY_BUFFER, renderOption.vbo[0]);

		int count = 0;

		for (auto obj : objects) {
			if (frustum->inSphere(modelToWorldPos(obj->center.x, obj->center.y), modelToWorldLen(obj->radius))) obj->render(count);
			count++;
		}
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

	void update(int selectLevel, shared_ptr<Frustum> frustum) {
		qtNode::frustum = frustum;
		qtNode::selectLevel = selectLevel;
		root->update(0);
	}

	void render(int selectLevel) {
		qtNode::selectLevel = selectLevel;
		root->render(0);
	}
};

int ObjectData::selectObject = 0;

#pragma once
#include <iostream>
#include <vector>
#include <memory>

#include "shapedata.h"
#include "object.h"

using namespace std;

// MeshCollection(prototype)
class MeshCollection {
public:
	float* allObjectVertices;
	int* allObjectIndices;
	int* allObjectVertexCount;
	int currentObjectVertexIndex = 0;

	float* allGridVertices;
	int* allGridIndices;
	int currentGridVertexIndex = 0;

	MeshCollection(int allObjectCount, int allGridCount) {
		allObjectVertices = new float[allObjectCount * 3];
		allObjectIndices = new int[allObjectCount * 3];
		allObjectVertexCount = new int[allObjectCount];

		allGridVertices = new float[allGridCount * 4];
		allGridIndices = new int[allGridCount * 4];
	}

	~MeshCollection() {
		delete[] allObjectVertices;
		delete[] allObjectIndices;
		delete[] allObjectVertexCount;
		delete[] allGridVertices;
		delete[] allGridIndices;
	}

	void addObjectVertices(shared_ptr<Object> obj) {
		for (int i = 0; i < obj->vertexCount; ++i) {
			allObjectVertices[currentObjectVertexIndex] = obj->vertices[i].x;
			allObjectVertices[currentObjectVertexIndex + 1] = obj->vertices[i].y;
			allObjectVertices[currentObjectVertexIndex + 2] = obj->vertices[i].z;

			allObjectIndices;
			currentObjectVertexIndex += 3;
		}

	}

	void addGridVertices(const float& Xmin, const float& Xmax, const float& Ymin, const float& Ymax) {
		allGridVertices;
		allGridIndices;
		currentGridVertexIndex++;
	}
};

//static MeshCollection ms;

struct QuadtreeNode {
	vector<float> objectVertices; // deprecate
	vector<int> objectVertexCounts; // deprecate
	vector<shared_ptr<Object>> objects;

	float Xmin, Xmax, Ymin, Ymax;
	float Xmid, Ymid;
	// min max buffer for objects
	float _min[2] = { FLT_MAX, FLT_MAX };
	float _max[2] = { FLT_MIN, FLT_MIN };

	shared_ptr<QuadtreeNode> nodes[4] = { nullptr, nullptr, nullptr, nullptr };

	QuadtreeNode(const float& _Xmin, const float& _Xmax, const float& _Ymin, const float& _Ymax) : Xmin(_Xmin), Xmax(_Xmax), Ymin(_Ymin), Ymax(_Ymax), Xmid((_Xmin + _Xmax) / 2), Ymid((_Ymin + _Ymax) / 2) {
	}

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
			maxLevel = std::max(maxLevel, level);
			return;
		}

		_min[0] = obj->min[0], _min[1] = obj->min[1], _max[0] = obj->max[0], _max[1] = obj->max[1];
		if (isLeft() && isUp()) {
			if (nodes[0] == nullptr) {
				nodes[0] = make_shared<QuadtreeNode>(Xmin, Xmid, Ymid, Ymax);
			}
			nodes[0]->store(obj, level + 1, maxLevel);
		}
		else if (isRight() && isUp()) {
			if (nodes[1] == nullptr) {
				nodes[1] = make_shared<QuadtreeNode>(Xmid, Xmax, Ymid, Ymax);
			}
			nodes[1]->store(obj, level + 1, maxLevel);
		}
		else if (isLeft() && isDown()) {
			if (nodes[2] == nullptr) {
				nodes[2] = make_shared<QuadtreeNode>(Xmin, Xmid, Ymin, Ymid);
			}
			nodes[2]->store(obj, level + 1, maxLevel);
		}
		else if (isRight() && isDown()) {
			if (nodes[3] == nullptr) {
				nodes[3] = make_shared<QuadtreeNode>(Xmid, Xmax, Ymin, Ymid);
			}
			nodes[3]->store(obj, level + 1, maxLevel);
		}
		else {
			objects.push_back(obj);
		}
	}

	// deprecate
	void addVertexAndPoint(vector<float>& allObjectVertices, vector<float>& allObjectVertexCount, vector<float>& allBorderPoints, int level, int selectLevel, int& count) {
		if (level > selectLevel) {
			return;
		}

		for (auto n : nodes) {
			if (n != nullptr) {
				n->addVertexAndPoint(allObjectVertices, allObjectVertexCount, allBorderPoints, level + 1, selectLevel, count);
			}
		}

		allObjectVertices.insert(allObjectVertices.end(), objectVertices.begin(), objectVertices.end());
		allObjectVertexCount.insert(allObjectVertexCount.end(), objectVertexCounts.begin(), objectVertexCounts.end());
		vector<float> border = { Xmin, Ymin, Xmin, Ymax, Xmax, Ymax, Xmax, Ymin };
		allBorderPoints.insert(allBorderPoints.end(), border.begin(), border.end());
	}

	void renderObject(int level, int selectLevel) {
		if (level > selectLevel) {
			return;
		}

		for (auto n : nodes) {
			if (n != nullptr) {
				n->renderObject(level + 1, selectLevel);
			}
		}

		for (auto obj : objects) {
			obj->render();
		}
	}

	void renderBorder(int level, int selectLevel) {
		if (level > selectLevel) {
			return;
		}

		for (auto n : nodes) {
			if (n != nullptr) {
				n->renderBorder(level + 1, selectLevel);
			}
		}

		GLuint indices[] = { 0, 1, 2, 3 };
		float border[] = { Xmin, Ymin, 0.0f, Xmin, Ymax, 0.0f, Xmax, Ymax, 0.0f, Xmax, Ymin, 0.0f };
		glBufferData(GL_ARRAY_BUFFER, 4 * 3 * sizeof(float), border, GL_STATIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 * sizeof(GLuint), indices, GL_STATIC_DRAW);

		glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_INT, 0);
	}
} typedef qtNode;

struct ObjectData {

	shared_ptr<qtNode> root;

	ObjectData(float Xmin, float Xmax, float Ymin, float Ymax) {
		root = make_shared<qtNode>(Xmin, Xmax, Ymin, Ymax);
	}

	void storeObject(const shared_ptr<Object> obj, int& maxLevel) {
		root->store(obj, 0, maxLevel);
	}

	void addVertexAndPoint(vector<float>& allObjectVertices, vector<float>& allObjectVertexCount, vector<float>& allBorderPoints, int selectLevel, int& count) {
		root->addVertexAndPoint(allObjectVertices, allObjectVertexCount, allBorderPoints, 0, selectLevel, count);
	}

	void renderObject(int selectLevel) {
		root->renderObject(0, selectLevel);
	}

	void renderBorder(int selectLevel) {
		root->renderBorder(0, selectLevel);
	}
};

#pragma once
#include <iostream>
#include <vector>
#include <memory>

#include "shapedata.h"
#include "object.h"

using namespace std;

// MeshCollection(prototype)
class MeshCollectionMemory {
public:
	float* objectVertices;
	int32_t* objectVertexCount;
	int currentObjectCount = 0;
	int currentVertexCount = 0;
	int allVertexCount = 0;

	float* allGridVertices;
	int borderCount = 0;

	~MeshCollectionMemory() {
		delete[] objectVertices;
		delete[] objectVertexCount;
		delete[] allGridVertices;
	}

	void clearMemory() {
		currentObjectCount = 0;
		currentVertexCount = 0;
		borderCount = 0;
	}

	// set allObjectVertices / allObjectIndices
	void allocateObjectMemory(int _allObjectCount, int _allVertexCount, int32_t _vertexCount[]) {
		allVertexCount = _allVertexCount;
		objectVertices = new float[_allVertexCount * 3];
		objectVertexCount = new int32_t[_allObjectCount];
		std::memcpy(objectVertexCount, _vertexCount, sizeof(int32_t) * _allObjectCount);
	}

	void allocateGridMemory(int nodeCount) {
		allGridVertices = new float[nodeCount * 3 * 4];
	}
};

class QuadtreeNode {
	friend class ObjectData;

	static int nodeCount;

	vector<shared_ptr<Object>> objects;

	float Xmin, Xmax, Ymin, Ymax;
	float Xmid, Ymid;
	// min max buffer for objects
	float _min[2] = { FLT_MAX, FLT_MAX };
	float _max[2] = { FLT_MIN, FLT_MIN };

	shared_ptr<QuadtreeNode> nodes[4] = { nullptr, nullptr, nullptr, nullptr };
public:
	QuadtreeNode(const float& _Xmin, const float& _Xmax, const float& _Ymin, const float& _Ymax) : Xmin(_Xmin), Xmax(_Xmax), Ymin(_Ymin), Ymax(_Ymax), Xmid((_Xmin + _Xmax) / 2), Ymid((_Ymin + _Ymax) / 2) {
		++nodeCount;
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
			maxLevel = std::max(maxLevel, level);
		}
	}

	void render(int level, int selectLevel, float boundaryX[], float boundaryY[]) {
		if (level > selectLevel) {
			return;
		}

		for (auto n : nodes) {
			if (n != nullptr) {
				n->render(level + 1, selectLevel, boundaryX, boundaryY);
			}
		}

		float border[] = { Xmin, Ymin, 0.0f, Xmin, Ymax, 0.0f, Xmax, Ymax, 0.0f, Xmax, Ymin, 0.0f };
		glBufferData(GL_ARRAY_BUFFER, 4 * 3 * sizeof(float), border, GL_STATIC_DRAW);
		glDrawArrays(GL_LINE_LOOP, 0, 4);

		for (auto obj : objects) {
			// check x boundary
			if (obj->max[0] < boundaryX[0] || obj->min[0] > boundaryX[1]) {
				continue;
			}

			// check y boundary
			if (obj->max[1] < boundaryY[0] || obj->min[1] > boundaryY[1]) {
				continue;
			}

			obj->render();
		}
	}
} typedef qtNode;

int qtNode::nodeCount = 0;

class ObjectData {
	
	shared_ptr<qtNode> root;
	shared_ptr<MeshCollectionMemory> ms;

public:
	ObjectData(float Xmin, float Xmax, float Ymin, float Ymax) {
		root = make_shared<qtNode>(Xmin, Xmax, Ymin, Ymax);
		ms = make_shared<MeshCollectionMemory>();
	}

	void store(const shared_ptr<Object> obj, int& maxLevel) {
		root->store(obj, 0, maxLevel);
	}

	void render(int selectLevel, float boundaryX[], float boundaryY[]) {
		root->render(0, selectLevel, boundaryX, boundaryY);
	}
};

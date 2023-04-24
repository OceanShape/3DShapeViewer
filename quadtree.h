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
	int* objectIndices;
	int* objectVertexCount;
	int currentObjectVertexIndex = 0;

	float* allGridVertices;
	int borderCount = 0;

	~MeshCollectionMemory() {
		delete[] objectVertices;
		delete[] objectIndices;
		delete[] objectVertexCount;
		delete[] allGridVertices;
	}

	// set allObjectVertices / allObjectIndices
	void allocateObjectMemory(int _allObjectCount, int _allVertexCount, int _vertexCount[]) {
		objectVertices = new float[_allObjectCount * 3];
		objectIndices = new int[_allVertexCount];
		for (int i = 0; i < _allVertexCount; ++i) {
			objectIndices[i] = i;
		}
		std::memcpy(objectVertexCount, _vertexCount, sizeof(float) * _allObjectCount);
	}

	void allocateGridMemory(int nodeCount) {
		allGridVertices = new float[nodeCount * 3 * 4];
	}

	void addObjectVertices(shared_ptr<Object> obj) {
		//obj 전체 버텍스 추가(전체 버텍스 수 확인)
		std::memcpy(objectVertices, )

		//part 카운트(전체 파트 수 확인)
	}

	void addGridVertices(const float& Xmin, const float& Xmax, const float& Ymin, const float& Ymax) {
		float border[] = { Xmin, Ymin, 0.0f, Xmin, Ymax, 0.0f, Xmax, Ymax, 0.0f, Xmax, Ymin, 0.0f };
		std::memcpy(&allGridVertices[borderCount * 4 * 3], border, sizeof(float) * 4 * 3);
		++borderCount;
	}

	void clearObject() {
	}

	void clearBorder() {
		borderCount = 0;
	}
};

//static MeshCollection ms;

class QuadtreeNode {
	friend class ObjectData;

	static int nodeCount;

	vector<float> objectVertices; // deprecate
	vector<int> objectVertexCounts; // deprecate
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

	void renderObject(MeshCollectionMemory& ms, int level, int selectLevel) {
		if (level > selectLevel) {
			return;
		}

		for (auto n : nodes) {
			if (n != nullptr) {
				n->renderObject(ms, level + 1, selectLevel);
			}
		}

		for (auto obj : objects) {
			ms.addObjectVertices(obj);
		}
	}

	void getBorderVertices(MeshCollectionMemory& ms, int level, int selectLevel) {
		if (level > selectLevel) {
			return;
		}

		for (auto n : nodes) {
			if (n != nullptr) {
				n->getBorderVertices(ms, level + 1, selectLevel);
			}
		}

		ms.addGridVertices(Xmin, Xmax, Ymin, Ymax);
	}
} typedef qtNode;

int qtNode::nodeCount = 0;

class ObjectData {
	
	shared_ptr<qtNode> root;
	MeshCollectionMemory ms;

public:
	ObjectData(float Xmin, float Xmax, float Ymin, float Ymax) {
		root = make_shared<qtNode>(Xmin, Xmax, Ymin, Ymax);
		ms = MeshCollectionMemory();
	}

	void allocateObjectMemory(int _allObjectCount, int _allVertexCount, int _vertexCount[]) {
		ms.allocateObjectMemory(_allObjectCount, _allVertexCount, _vertexCount);
	}

	void allocateGridMemory() {
		ms.allocateGridMemory(QuadtreeNode::nodeCount);
	}

	void storeObject(const shared_ptr<Object> obj, int& maxLevel) {
		root->store(obj, 0, maxLevel);
	}

	void addVertexAndPoint(vector<float>& allObjectVertices, vector<float>& allObjectVertexCount, vector<float>& allBorderPoints, int currentLevel, int& count) {
		root->addVertexAndPoint(allObjectVertices, allObjectVertexCount, allBorderPoints, 0, currentLevel, count);
	}

	void renderObject(int currentLevel) {

		ms.clearObject();

		root->renderObject(ms, 0, currentLevel);

		/*
		root->getBorderVertices(ms, 0, currentLevel);

		//border
		glBufferData(GL_ARRAY_BUFFER, ms.borderCount * 3 * sizeof(float), ms.borderVertices, GL_STATIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, ms.borderCount * sizeof(GLuint), ms.borderIndices, GL_STATIC_DRAW);
		for (int i = 0; i < ms.borderCount; ++i) {
			glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_INT, index~~~~~);
		}
		*/
	}

	void renderBorder(int currentLevel) {

		ms.clearBorder();

		root->getBorderVertices(ms, 0, currentLevel);

		glBufferData(GL_ARRAY_BUFFER, ms.borderCount * 4 * 3 * sizeof(float), ms.allGridVertices, GL_STATIC_DRAW);
		for (int i = 0; i < ms.borderCount; ++i) {
			glDrawArrays(GL_LINE_LOOP, i * 4, 4);
		}
	}
};

#pragma once
#include <iostream>
#include <vector>
#include <memory>

#include "shapedata.h"
#include "object.h"

using namespace std;

struct QuadtreeNode {
	vector<float> objectVertices;
	vector<int> objectVertexCounts;
	vector<Object*> objects;

	float Xmin, Xmax, Ymin, Ymax;
	float Xmid, Ymid;
	float _min[2] = { FLT_MAX, FLT_MAX };
	float _max[2] = { FLT_MIN, FLT_MIN };

	int maxLevel = 0;
	shared_ptr<QuadtreeNode> nodes[4] = { nullptr, nullptr, nullptr, nullptr };

	QuadtreeNode(const float& _Xmin, const float& _Xmax, const float& _Ymin, const float& _Ymax, const int& _maxLevel) : Xmin(_Xmin), Xmax(_Xmax), Ymin(_Ymin), Ymax(_Ymax), Xmid((_Xmin + _Xmax) / 2), Ymid((_Ymin + _Ymax) / 2), maxLevel(_maxLevel) {
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

	void store(Object& obj, int level) {
		if (level == maxLevel || Xmax - Xmin < 120) {
			objects.push_back(&obj);
			return;
		}

		_min[0] = obj.min[0], _min[1] = obj.min[1], _max[0] = obj.max[0], _max[1] = obj.max[1];
		if (isLeft() && isUp()) {
			if (nodes[0] == nullptr) {
				nodes[0] = make_shared<QuadtreeNode>(Xmin, Xmid, Ymid, Ymax, maxLevel);
			}
			nodes[0]->store(obj, level + 1);
		}
		else if (isRight() && isUp()) {
			if (nodes[1] == nullptr) {
				nodes[1] = make_shared<QuadtreeNode>(Xmid, Xmax, Ymid, Ymax, maxLevel);
			}
			nodes[1]->store(obj, level + 1);
		}
		else if (isLeft() && isDown()) {
			if (nodes[2] == nullptr) {
				nodes[2] = make_shared<QuadtreeNode>(Xmin, Xmid, Ymin, Ymid, maxLevel);
			}
			nodes[2]->store(obj, level + 1);
		}
		else if (isRight() && isDown()) {
			if (nodes[3] == nullptr) {
				nodes[3] = make_shared<QuadtreeNode>(Xmid, Xmax, Ymin, Ymid, maxLevel);
			}
			nodes[3]->store(obj, level + 1);
		}
		else {
			objects.push_back(&obj);
		}
	}

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

	ObjectData(float Xmin, float Xmax, float Ymin, float Ymax, int maxLevel) {
		root = make_shared<qtNode>(Xmin, Xmax, Ymin, Ymax, maxLevel);
	}

	void storeObject(Object& obj) {
		root->store(obj, 0);
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

#pragma once
#include <iostream>
#include <vector>
#include <memory>

#include "shapedata.h"

using namespace std;

struct QuadtreeNode {
	vector<int> objectVertices;
	vector<int> objectVertexCounts;

	float Xmin, Xmax, Ymin, Ymax, Zmin, Zmax;
	float Xmid, Ymid;
	int maxLevel = 0;
	shared_ptr<QuadtreeNode> nodes[4] = { nullptr, nullptr, nullptr, nullptr };

	QuadtreeNode(const float& _Xmin, const float& _Xmax, const float& _Ymin, const float& _Ymax, const float& _Zmin, const float& _Zmax, const int& _maxLevel) : Xmin(_Xmin), Xmax(_Xmax), Ymin(_Ymin), Ymax(_Ymax), Zmin(_Zmin), Zmax(_Zmax), Xmid((_Xmin + _Xmax) / 2), Ymid((_Ymin + _Ymax) / 2), maxLevel(_maxLevel) {
	}

	bool isLeft(const float& _Xmin, const float& _Xmax) {
		return Xmin <= _Xmin && Xmin <= _Xmax && _Xmin <= Xmid && _Xmax <= Xmid;
	}
	bool isRight(const float& _Xmin, const float& _Xmax) {
		return Xmid <= _Xmin && Xmid <= _Xmax && _Xmin <= Xmax && _Xmax <= Xmax;
	}
	bool isUp(const float& _Ymin, const float& _Ymax) {
		return Ymid <= _Ymin && Ymid <= _Ymax && _Ymin <= Ymax && _Ymax <= Ymax;
	}
	bool isDown(const float& _Ymin, const float& _Ymax) {
		return Ymin <= _Ymin && Ymin <= _Ymax && _Ymin <= Ymid && _Ymax <= Ymid;
	}

	void store(const vector<float>& _objectVertices, float _min[], float _max[], int level) {
		if (level == maxLevel) {
			objectVertices.insert(objectVertices.end(), _objectVertices.begin(), _objectVertices.end());
			objectVertexCounts.push_back(_objectVertices.size() / 3);
			return;
		}

		if (isLeft(_min[0], _max[0]) && isUp(_min[1], _max[1])) {
			if (nodes[0] == nullptr) {
				nodes[0] = make_shared<QuadtreeNode>(Xmin, Xmid, Ymid, Ymax, Zmin, Zmax, maxLevel);
			}
			nodes[0]->store(_objectVertices, _min, _max, level + 1);
		}
		else if (isRight(_min[0], _max[0]) && isUp(_min[1], _max[1])) {
			if (nodes[1] == nullptr) {
				nodes[1] = make_shared<QuadtreeNode>(Xmid, Xmax, Ymid, Ymax, Zmin, Zmax, maxLevel);
			}
			nodes[1]->store(_objectVertices, _min, _max, level + 1);
		}
		else if (isLeft(_min[0], _max[0]) && isDown(_min[1], _max[1])) {
			if (nodes[2] == nullptr) {
				nodes[2] = make_shared<QuadtreeNode>(Xmin, Xmid, Ymin, Ymid, Zmin, Zmax, maxLevel);
			}
			nodes[2]->store(_objectVertices, _min, _max, level + 1);
		}
		else if (isRight(_min[0], _max[0]) && isDown(_min[1], _max[1])) {
			if (nodes[3] == nullptr) {
				nodes[3] = make_shared<QuadtreeNode>(Xmid, Xmax, Ymin, Ymid, Zmin, Zmax, maxLevel);
			}
			nodes[3]->store(_objectVertices, _min, _max, level + 1);
		}
		else {
			objectVertices.insert(objectVertices.end(), _objectVertices.begin(), _objectVertices.end());
			objectVertexCounts.push_back(_objectVertices.size() / 3);
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
		//count += objectVertexCounts.size();

		allObjectVertices.insert(allObjectVertices.end(), objectVertices.begin(), objectVertices.end());
		allObjectVertexCount.insert(allObjectVertexCount.end(), objectVertexCounts.begin(), objectVertexCounts.end());
		vector<float> border = { Xmin, Ymin, Zmin, Xmin, Ymax, Zmin, Xmax, Ymax, Zmin, Xmax, Ymin, Zmin, Xmin, Ymin, Zmin };
		count++;
		allBorderPoints.insert(allBorderPoints.end(), border.begin(), border.end());
	}
} typedef qtNode;

struct ObjectData {

	shared_ptr<qtNode> root;

	ObjectData(float Xmin, float Xmax, float Ymin, float Ymax, float Zmin, float Zmax, int maxLevel) {
		root = make_shared<qtNode>(Xmin, Xmax, Ymin, Ymax, Zmin, Zmax, maxLevel);
	}

	void storeObject(const vector<float>& objectVertices, bool trigger) {

		float _min[3] = { FLT_MAX, FLT_MAX, FLT_MAX };
		float _max[3] = { FLT_MIN, FLT_MIN, FLT_MIN };
		for (size_t i = 0; i < objectVertices.size() / 3; ++i) {
			_min[0] = std::min(_min[0], objectVertices[i * 3]);
			_max[0] = std::max(_max[0], objectVertices[i * 3]);
			_min[1] = std::min(_min[1], objectVertices[i * 3 + 1]);
			_max[1] = std::max(_max[1], objectVertices[i * 3 + 1]);
			_min[2] = std::min(_min[2], objectVertices[i * 3 + 2]);
			_max[2] = std::max(_max[2], objectVertices[i * 3 + 2]);
		}

		root->store(objectVertices, _min, _max, 0);
	}

	void addVertexAndPoint(vector<float>& allObjectVertices, vector<float>& allObjectVertexCount, vector<float>& allBorderPoints, int selectLevel, int& count) {
		root->addVertexAndPoint(allObjectVertices, allObjectVertexCount, allBorderPoints, 0, selectLevel, count);
	}
};


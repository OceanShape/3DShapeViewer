#pragma once
#include <iostream>
#include <vector>
#include <memory>

#include "shapedata.h"

struct QuadtreeNode {
	vector<int> objects;
	vector<int> objectVertexCounts;

	float Xmin, Xmax, Ymin, Ymax;
	float Xmid, Ymid;
	shared_ptr<QuadtreeNode> nodes[4];

	QuadtreeNode(const float& _Xmin, const float& _Xmax, const float& _Ymin, const float& _Ymax) : Xmin(_Xmin), Xmax(_Xmax), Ymin(_Ymin), Ymax(_Ymax), Xmid((_Xmin + _Xmax) / 2), Ymid((_Ymin + _Ymax) / 2) {};

	bool isLeft(const float& _Xmin, const float& _Xmax);
	bool isRight(const float& _Xmin, const float& _Xmax);
	bool isUp(const float& _Ymin, const float& _Ymax);
	bool isDown(const float& _Ymin, const float& _Ymax);

	void store(const vector<int>& objectVertices, const float& _Xmin, const float& _Xmax, const float& _Ymin, const float& _Ymax);
	void addBorderPoints(int level);
} typedef qtNode;

struct ObjectData {
	shared_ptr<qtNode> root;

	ObjectData(float Xmin, float Xmax, float Ymin, float Ymax);
	void store(const vector<int>& allObjectVertices);

	void addVertexAndPoint(int level);
};

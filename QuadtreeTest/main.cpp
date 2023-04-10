#include <iostream>
#include <vector>
#include <memory>

#include "shapedata.h"

using namespace std;

struct QuadtreeNode {
	vector<int> objects;
	vector<int> objectVertexCounts;

	float Xmin, Xmax, Ymin, Ymax;
	float Xmid, Ymid;
	shared_ptr<QuadtreeNode> nodes[4] = { nullptr, nullptr, nullptr, nullptr };

	QuadtreeNode(const float& _Xmin, const float& _Xmax, const float& _Ymin, const float& _Ymax) : Xmin(_Xmin), Xmax(_Xmax), Ymin(_Ymin), Ymax(_Ymax), Xmid((_Xmin + _Xmax) / 2), Ymid((_Ymin + _Ymax) / 2) {
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

	void store(const vector<int>& objectVertices, const float& _Xmin, const float& _Xmax, const float& _Ymin, const float& _Ymax) {
		if (isLeft(_Xmin, _Xmax) && isUp(_Ymin, _Ymax)) {
			nodes[0] = make_shared<QuadtreeNode>(Xmin, Xmid, Ymid, Ymax);
			nodes[0]->store(objectVertices, _Xmin, _Xmax, _Ymin, _Ymax);
		}
		else if (isRight(_Xmin, _Xmax) && isUp(_Ymin, _Ymax)) {
			nodes[1] = make_shared<QuadtreeNode>(Xmid, Xmax, Ymid, Ymax);
			nodes[1]->store(objectVertices, _Xmin, _Xmax, _Ymin, _Ymax);
		}
		else if (isLeft(_Xmin, _Xmax) && isDown(_Ymin, _Ymax)) {
			nodes[2] = make_shared<QuadtreeNode>(Xmin, Xmid, Ymin, Ymid);
			nodes[2]->store(objectVertices, _Xmin, _Xmax, _Ymin, _Ymax);
		}
		else if (isRight(_Xmin, _Xmax) && isDown(_Ymin, _Ymax)) {
			nodes[3] = make_shared<QuadtreeNode>(Xmid, Xmax, Ymin, Ymid);
			nodes[3]->store(objectVertices, _Xmin, _Xmax, _Ymin, _Ymax);
		}
		else {
			objects.insert(objects.end(), objectVertices.begin(), objectVertices.end());
			objectVertexCounts.push_back(objectVertices.size());
		}
	}

	void render(int level) {
		bool allNodeNull;
		for (auto n : nodes) {

		}
	}
} typedef qtNode;

struct ObjectData {

	shared_ptr<qtNode> root;

	ObjectData(float Xmin, float Xmax, float Ymin, float Ymax) {
		root = make_shared<qtNode>(Xmin, Xmax, Ymin, Ymax);
	}

	void store(const vector<int>& allObjectVertices) {
		//root->store(allObjectVertices);
	}

	void render(int level) {
		//root->render();
	}
};




int main() {
	std::vector<int> source{ 1, 2, 3, 4, 5 };
	std::vector<int> dest{ 9, 8, 7, 6 };

	
	for (auto i : dest) {
		cout << i << " ";
	}

	return 0;
}
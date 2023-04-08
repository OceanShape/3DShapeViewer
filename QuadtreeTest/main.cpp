#include <iostream>
#include <vector>
#include <memory>

#include "shapedata.h"

using namespace std;

struct Quadtree {
	vector<int> objects;
	vector<int> objectVertexCount;

	float Xmin, Xmax, Ymin, Ymax;
	shared_ptr<Quadtree> nodes[4] = { nullptr, nullptr, nullptr, nullptr };

	Quadtree(float Xmin, float Xmax, float Ymin, float Ymax) : Xmin(Xmin), Xmax(Xmax), Ymin(Ymin), Ymax(Ymax) {
	}

	void store(const vector<int>& vertices) {

	}

	void render() {
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

	void store(const vector<int>& vertices) {
		root->store(vertices);
	}

	void render() {
		root->render();
	}
};




int main() {
	cout << "Hi" << endl;

	return 0;
}
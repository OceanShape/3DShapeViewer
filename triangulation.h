#pragma once
#include <iostream>
#include <list>
#include <vector>
#include <cmath>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/string_cast.hpp>
#include <set>
#include <GLES3/gl3.h>

typedef glm::vec3 Vertex;

// https://bloodstrawberry.tistory.com/996
struct Triangle
{
    GLuint a, b, c;
    Triangle(GLuint a, GLuint b, GLuint c) : a(a), b(b), c(c) {}
};

bool isVertexInsideTriangle(const Vertex& v0, const Vertex& v1, const Vertex& v2, const Vertex& p) {
    auto m = glm::cross(p - v0, v1 - v0);
    auto n = glm::cross(p - v1, v2 - v1);
    auto o = glm::cross(p - v2, v0 - v2);

    return m.z > 0 && n.z > 0 && o.z > 0;
}

bool isVerticesInsideTriangle(const std::vector<Vertex>& v, const int& index) {
    Vertex v0 = v[index];
    Vertex v1 = v[index + 1];
    Vertex v2 = v[index + 2];

    for (int i = index + 3; i < v.size(); i++) {
        if (isVertexInsideTriangle(v0, v1, v2, v[i])) {
            return true;
        }
    }

    return false;
}

bool isCCW(Vertex v0, Vertex v1, Vertex v2) {
    glm::vec3 p = v1 - v0;
    glm::vec3 q = v2 - v1;

    return glm::cross(p, q).z > 0;
}

void Triangulate(std::vector<Triangle>& result, const Vertex vertices[], int verticesLength)
{
    // 전체 점을 벡터에 넣음
    std::vector<Vertex> v; // 체크해야할 점(마지막 점은 동일하므로 넣지 않음)
    for (int i = 0; i < verticesLength - 1; ++i) {
        v.push_back(vertices[i]);
    }

    //  삼각형의 개수가 (점의 개수 - 2)가 될 때까지 반복
    for (int i = 0; i < verticesLength - 1 - 2; i++) {
        for (int j = 0; j < v.size() - 2; ++i) {
            if (isCCW(v[j], v[j + 1], v[j + 2]) == true && isVerticesInsideTriangle(v, j) == false) {
                result.push_back(Triangle(i, i + 1, i + 2));
                v.erase(v.begin() + j + 1);
                break;
            }

        }
        std::cout << result.size() << "/" << v.size() << std::endl;
    }
}

int main()
{
    //Vertex points[] = {{0, 1, .0f}, {-1,0, .0f},{1,0, .0f},{0,-1, .0f}};
    //Vertex points[] = { {0, 2, .0f}, {-1,0, .0f},{1,0, .0f},{2,2, .0f},{-2,2, .0f},{0, 3, .0f} };
    //Vertex points[] = { {1, .0f, .0f}, {1, -1, .0f}, {-1, -1, .0f}, {-1, 1, .0f}, {0, 1, .0f}, {.0f, .0f, .0f}, {1, .0f, .0f} };
    float a = .0707107f;
    Vertex points[] = { {1, 0, 0}, { a, -a, .0}, {0, -1, 0}, {-a, -a, 0}, {-1, 0, 0}, {-a, a, 0},{0, 1, 0}, {a, a, 0}, {1, 0, 0} };
    std::vector<Triangle> triangulation;
    Triangulate(triangulation, points, 9);
    int* indices = new int[triangulation.size() * 3];

    std::cout << "tri " << triangulation.size() << std::endl;

    //for (int i = 0; i < triangulation.size(); i++) {
    //    std::cout << "(\t" << triangulation[i].a << ",\t" << triangulation[i].b << ",\t" << triangulation[i].c << ")" << std::endl;
    //}

    //delete[] indices;

    return 0;
}

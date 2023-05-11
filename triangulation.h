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

    bool isVertexInsideTriangle(std::list<Vertex>& list, const Vertex v[], int index) {
        Vertex v0 = v[index];
        Vertex v1 = v[index + 1];
        Vertex v2 = v[index + 2];


        return true;
    }
};

// ���� �ڵ�
//bool checkTriangleInPoint(Vector3 dot1, Vector3 dot2, Vector3 dot3, Vector3 checkPoint)
//{
//    float area = getAreaOfTriangle(dot1, dot2, dot3);
//    float dot12 = getAreaOfTriangle(dot1, dot2, checkPoint);
//    float dot23 = getAreaOfTriangle(dot2, dot3, checkPoint);
//    float dot31 = getAreaOfTriangle(dot3, dot1, checkPoint);
//
//    return (dot12 + dot23 + dot31) <= area + 0.1f /* ���� ��� */;
//}
//
//bool CrossCheckAll(List<Vector3> list, int index)
//{
//    Vector3 a = list[index];
//    Vector3 b = list[index + 1];
//    Vector3 c = list[index + 2];
//
//    for (int i = index + 3; i < list.Count; i++)
//    {
//        if (checkTriangleInPoint(a, b, c, list[i]) == true) return true;
//    }
//
//    return false;
//}

bool isCCW(Vertex v0, Vertex v1, Vertex v2) {
    glm::vec3 p = v1 - v0;
    glm::vec3 q = v2 - v1;

    return glm::cross(p, q).y > 0;
}

std::vector<Triangle> Triangulate(std::vector<Triangle>& result, const Vertex vertices[], int verticesLength)
{

    // ��ü ���� ���Ϳ� ����
    std::list<Vertex> vertexList; // üũ�ؾ��� ��(������ ���� �����ϹǷ� ���� ����)
    auto v = vertices;
    for (int i = 0; i < verticesLength - 1; ++i) {
        vertexList.push_back(vertices[i]);
    }

    //  �ﰢ���� ������ (���� ���� - 2)�� �� ������ �ݺ�
    while (result.size() < verticesLength - 1 - 2) {
        for (int i = 0; i < vertexList.size() - 2; ++i) {
            if (isCCW(v[i], v[i + 1], v[i + 2])) {
                continue;
            }


        }
    }


    //  v0�� ���� ���� �� ���� ����� �� �� v1, v2�� ���ʷ� ����
    //          CCW�� ��� �Ѿ
    //          CW�� ���
    //              v2 ���ĺ��� �������� ���� Ž��
    //              ���� �ﰢ���� �ٸ� ���� ���Ե� ��� �Ѿ
    //              ���� �ﰢ���� �ٸ� ���� ���Ե��� ���� ���
    //                  v0, v1, v2 �ﰢ�� ����=>����� �߰�
    //                  v1 ����
    //                  �ݺ��� ó������ ����







    // 0 ~ n - 3(����) ������ �� Ž��(�� ���� �ϳ� �� ����ֱ� ����)(n: �迭�� ����)
    //  v0�� pass ��Ͽ� ���Ե� ��� �Ѿ
    //  v0�� ���� ���� �� ���� ����� �� �� v1, v2�� ���ʷ� ����(pass ��Ͽ� ���Ե� ���, ���� ���� ������)
    //          CCW�� ��� �Ѿ
    //          CW�� ���
    //              v2 ���ĺ��� n-2(����)������ ���� Ž��(���� pass ��Ͽ� ���Ե� ���, Ȯ������ ����)
    //              ���� �ﰢ���� �ٸ� ���� ���Ե� ��� �Ѿ
    //              ���� �ﰢ���� �ٸ� ���� ���Ե��� ���� ���
    //                  v0, v1, v2 �ﰢ�� ����=>����� �߰�
    //                  ��� ���� pass ��Ͽ� �߰�(�������ʹ� ������� ����)
    //                  �� index 0���� ����
    return result;
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

    for (int i = 0; i < triangulation.size(); i++) {
        std::cout << "(\t" << triangulation[i].a << ",\t" << triangulation[i].b << ",\t" << triangulation[i].c << ")" << std::endl;
    }

    //delete[] indices;

    return 0;
}

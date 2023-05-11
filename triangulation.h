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

// 참고 코드
//bool checkTriangleInPoint(Vector3 dot1, Vector3 dot2, Vector3 dot3, Vector3 checkPoint)
//{
//    float area = getAreaOfTriangle(dot1, dot2, dot3);
//    float dot12 = getAreaOfTriangle(dot1, dot2, checkPoint);
//    float dot23 = getAreaOfTriangle(dot2, dot3, checkPoint);
//    float dot31 = getAreaOfTriangle(dot3, dot1, checkPoint);
//
//    return (dot12 + dot23 + dot31) <= area + 0.1f /* 오차 허용 */;
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

    // 전체 점을 벡터에 넣음
    std::list<Vertex> vertexList; // 체크해야할 점(마지막 점은 동일하므로 넣지 않음)
    auto v = vertices;
    for (int i = 0; i < verticesLength - 1; ++i) {
        vertexList.push_back(vertices[i]);
    }

    //  삼각형의 개수가 (점의 개수 - 2)가 될 때까지 반복
    while (result.size() < verticesLength - 1 - 2) {
        for (int i = 0; i < vertexList.size() - 2; ++i) {
            if (isCCW(v[i], v[i + 1], v[i + 2])) {
                continue;
            }


        }
    }


    //  v0의 이후 점들 중 가장 가까운 두 점 v1, v2를 차례로 지정
    //          CCW일 경우 넘어감
    //          CW일 경우
    //              v2 이후부터 끝까지의 점을 탐색
    //              현재 삼각형에 다른 점이 포함될 경우 넘어감
    //              현재 삼각형에 다른 점이 포함되지 않을 경우
    //                  v0, v1, v2 삼각형 판정=>결과에 추가
    //                  v1 제거
    //                  반복자 처음으로 돌림







    // 0 ~ n - 3(포함) 까지의 점 탐색(끝 점이 하나 더 들어있기 때문)(n: 배열의 길이)
    //  v0가 pass 목록에 포함될 경우 넘어감
    //  v0의 이후 점들 중 가장 가까운 두 점 v1, v2를 차례로 지정(pass 목록에 포함될 경우, 다음 점을 선택함)
    //          CCW일 경우 넘어감
    //          CW일 경우
    //              v2 이후부터 n-2(포함)까지의 점을 탐색(역시 pass 목록에 포함될 경우, 확인하지 않음)
    //              현재 삼각형에 다른 점이 포함될 경우 넘어감
    //              현재 삼각형에 다른 점이 포함되지 않을 경우
    //                  v0, v1, v2 삼각형 판정=>결과에 추가
    //                  가운데 점을 pass 목록에 추가(다음부터는 고려하지 않음)
    //                  점 index 0으로 돌림
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

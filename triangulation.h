#pragma once
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <glm/glm.hpp>

typedef glm::vec3 Point;
typedef std::pair<Point, Point> PairPoint;

struct Triangle
{
    Point a, b, c;
    Point points[3];
    Triangle(Point a, Point b, Point c) : a(a), b(b), c(c) {
        points[0] = a;
        points[1] = b;
        points[2] = c;
    }
    bool operator==(const Triangle& t) const {
        return (this->a == t.a && this->b == t.b && this->c == t.c);
    }
    void printStatus() {
        printf("(%f, %f)(%f, %f)(%f, %f)\n", a.x, a.y, b.x, b.y, c.x, c.y);
    }
    bool hasPoint(const Point& p) {
        return points[0] == p || points[1] == p || points[2] == p;
    }
};

template <>
bool std::operator==(const PairPoint& lhs, const PairPoint& rhs) {
    return (lhs.first == rhs.first && lhs.second == rhs.second) || (lhs.first == rhs.second && lhs.second == rhs.first);
}

// 중복된 점이 없다고 가정
bool CircumCircle(Point p1, Point p2, Point p3, Point& center, double& radius)
{
    double fabsy1y2 = std::abs(p1.y - p2.y);
    double fabsy2y3 = std::abs(p2.y - p3.y);
    double xc, yc;
    if (fabsy1y2 < 1e-10 && fabsy2y3 < 1e-10) {
        return false;
    }
    else if (fabsy1y2 < 1e-10)
    {
        double m2 = -((p3.x - p2.x) / (p3.y - p2.y));
        double mx2 = (p2.x + p3.x) / 2.0;
        double my2 = (p2.y + p3.y) / 2.0;
        xc = (p2.x + p1.x) / 2.0;
        yc = m2 * (xc - mx2) + my2;
    }
    else if (fabsy2y3 < 1e-10)
    {
        double m1 = -((p2.x - p1.x) / (p2.y - p1.y));
        double mx1 = (p1.x + p2.x) / 2.0;
        double my1 = (p1.y + p2.y) / 2.0;
        xc = (p3.x + p2.x) / 2.0;
        yc = m1 * (xc - mx1) + my1;
    }
    else
    {
        double m1 = -((p2.x - p1.x) / (p2.y - p1.y));
        double m2 = -((p3.x - p2.x) / (p3.y - p2.y));
        double mx1 = (p1.x + p2.x) / 2.0;
        double mx2 = (p2.x + p3.x) / 2.0;
        double my1 = (p1.y + p2.y) / 2.0;
        double my2 = (p2.y + p3.y) / 2.0;
        xc = (m1 * mx1 - m2 * mx2 + my2 - my1) / (m1 - m2);
        yc = m1 * (xc - mx1) + my1;
    }
    double dx = p2.x - xc;
    double dy = p2.y - yc;
    radius = std::sqrt(dx * dx + dy * dy);
    center = Point(xc, yc, .0f);
    return true;
}

bool InCircle(Point p, Triangle t)
{
    Point center;
    double radius;
    if (!CircumCircle(t.a, t.b, t.c, center, radius))
        return false;
    double dx = p.x - center.x;
    double dy = p.y - center.y;
    double dist_squared = dx * dx + dy * dy;
    return (dist_squared <= radius * radius);
}

void AddPoint(std::vector<Triangle>& triangulation, Point point)
{
    std::vector<Triangle> bad_triangles;
    std::vector<std::pair<Point, Point>> polygon;

    // triangulation의 각 삼각형에 대해 삽입으로 인해 더 이상 유효하지 않은 모든 삼각형을 찾아냄
    for (const Triangle& t : triangulation)
    {
        // point가 삼각형의 원 내부에 있다면 badTriangles에 삼각형을 추가
        if (InCircle(point, t))
        {
            bad_triangles.push_back(t);
        }
    }

    //        badTriangles의 각 삼각형에 대해 다각형 구멍의 경계를 찾습니다.
    //            삼각형의 각 변에 대해
    //                다른 badTriangles에 의해 공유되지 않는다면 polygon에 edge를 추가
    for (const Triangle& t : bad_triangles) {
        Point tmp[3] = { t.a, t.b, t.c };
        for (int i = 0; i < 3; ++i) {
            auto edge = std::make_pair(tmp[i], tmp[(i + 1) % 3]);
            auto iter = std::find(polygon.begin(), polygon.end(), edge);
            if (iter != polygon.end()) {
                polygon.erase(iter);
            }
            else {
                polygon.push_back(edge);
            }
        }
    }


    // badTriangles의 각 삼각형을 데이터 구조에서 제거
    triangulation.erase(std::remove_if(triangulation.begin(), triangulation.end(),
        [&](Triangle t) { return std::find(bad_triangles.begin(), bad_triangles.end(), t) != bad_triangles.end(); }),
        triangulation.end());

    // polygon의 각 edge에 대해 다각형 구멍을 다시 삼각화
    for (const std::pair<Point, Point>& edge : polygon)
    {
        // edge와 point로부터 삼각형을 생성
        Triangle new_triangle(point, edge.first, edge.second);
        // newTri를 triangulation에 추가
        triangulation.push_back(new_triangle);
    }
}

std::vector<Triangle> Triangulate(std::vector<Point> points)
{
    std::vector<Triangle> triangulation;

    // super - triangle을 triangulation에 추가
    // 모든 점을 완전히 포함할 수 있도록 충분히 큰 삼각형
    double xmin = points[0].x;
    double ymin = points[0].y;
    double xmax = xmin;
    double ymax = ymin;
    for (auto p : points)
    {
        if (p.x < xmin) xmin = p.x;
        if (p.y < ymin) ymin = p.y;
        if (p.x > xmax) xmax = p.x;
        if (p.y > ymax) ymax = p.y;
    }

    double dx = xmax - xmin;
    double dy = ymax - ymin;
    double dmax = std::max(dx, dy);
    double xmid = xmin + dx / 2.0;
    double ymid = ymin + dy / 2.0;

    Point p1(xmid - 20 * dmax, ymid - dmax, .0f);
    Point p2(xmid, ymid + 20 * dmax, .0f);
    Point p3(xmid + 20 * dmax, ymid - dmax, .0f);

    Triangle bounding_triangle(p1, p2, p3);
    triangulation.push_back(bounding_triangle);

    // 모든 점을 하나씩 삼각화에 추가
    for (const Point& p : points)
    {
        AddPoint(triangulation, p);
    }

    // 원래 super - triangle의 정점을 포함하는 경우 해당 삼각형을 triangulation에서 제거
    triangulation.erase(
        std::remove_if(
            triangulation.begin(), triangulation.end(),
            [&](Triangle t) {
                return std::find_if(std::begin(t.points), std::end(t.points), [&](const Point& p) {return bounding_triangle.hasPoint(p); }) != std::end(t.points);
            }
    ), triangulation.end());


    // triangulation을 반환
    return triangulation;
}

//int main()
//{
//    std::vector<Point> points = { {-1,0, .0f}, {1,0, .0f}, {2,2, .0f}, {-2,2, .0f}, {0, 3, .0f}, {0, 2, .0f} };
//    std::vector<Triangle> triangulation = Triangulate(points);
//    for (const Triangle& t : triangulation)
//    {
//        std::cout << "(" << t.a.x << ", " << t.a.y << ") ";
//        std::cout << "(" << t.b.x << ", " << t.b.y << ") ";
//        std::cout << "(" << t.c.x << ", " << t.c.y << ")" << std::endl;
//    }
//
//    return 0;
//}

//function BowyerWatson(pointList)
//    // pointList는 삼각화될 점을 정의하는 좌표 집합입니다.
//    triangulation : = 빈 삼각형 메쉬 데이터 구조
//    super - triangle을 triangulation에 추가합니다. // 모든 점을 완전히 포함할 수 있도록 충분히 큰 삼각형이어야 합니다.
//    pointList의 각 점에 대해 // 모든 점을 하나씩 삼각화에 추가합니다.
//        badTriangles : = 빈 집합
//        triangulation의 각 삼각형에 대해 // 삽입으로 인해 더 이상 유효하지 않은 모든 삼각형을 찾습니다.
//            if point가 삼각형의 원 내부에 있다면
//                badTriangles에 삼각형을 추가합니다.
//        polygon : = 빈 집합
//        badTriangles의 각 삼각형에 대해 // 다각형 구멍의 경계를 찾습니다.
//            삼각형의 각 변에 대해
//                if 다른 badTriangles에 의해 공유되지 않는다면
//                    polygon에 edge를 추가합니다.
//        badTriangles의 각 삼각형을 제거합니다. // 데이터 구조에서 제거합니다.
//        polygon의 각 edge에 대해 // 다각형 구멍을 다시 삼각화합니다.
//            edge와 point로부터 삼각형을 생성합니다.
//            newTri를 triangulation에 추가합니다.
//    triangulation의 각 삼각형에 대해 // 점 삽입이 완료되었습니다. 이제 정리합니다.
//        원래 super - triangle의 정점을 포함하는 경우
//            triangulation에서 삼각형을 제거합니다.
//    triangulation을 반환합니다.

// 삼차원 공간에 적용하는 방법
// 삼차원 공간에서도 외심의 성질을 활용
//   1. 세 점 A-B-C가 있을 때, 선분 A-B/B-C의 중간을 지나면서 각 선분에 수직인 직선의 방정식을 구함
//   2. 두 직선이 만나는 점이 외접원의 중심(외심)

// 현재 생각하는 알고리듬
// 목표: 선분 A-B의 중심을 지나면서 수직인 직선의 방정식
// 알고 있는 것: 지나는 점(A-B 중심)/수직인 직선/지나는 평면
// 1. 중심점을 지나면서, A-B에 수직인 평면의 방정식을 구함
// 2. 두 평면이 겹치는 직선의 방정식을 구함

//1. A-B의 중점 q구함
//2. 점q를 지나고 벡터 A->B를 법선벡터로 가지는 평면 L 구함
//3. 평면 L과 M의 교선(직선)의 방정식 구함

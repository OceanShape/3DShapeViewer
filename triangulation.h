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

// �ߺ��� ���� ���ٰ� ����
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

    // triangulation�� �� �ﰢ���� ���� �������� ���� �� �̻� ��ȿ���� ���� ��� �ﰢ���� ã�Ƴ�
    for (const Triangle& t : triangulation)
    {
        // point�� �ﰢ���� �� ���ο� �ִٸ� badTriangles�� �ﰢ���� �߰�
        if (InCircle(point, t))
        {
            bad_triangles.push_back(t);
        }
    }

    //        badTriangles�� �� �ﰢ���� ���� �ٰ��� ������ ��踦 ã���ϴ�.
    //            �ﰢ���� �� ���� ����
    //                �ٸ� badTriangles�� ���� �������� �ʴ´ٸ� polygon�� edge�� �߰�
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


    // badTriangles�� �� �ﰢ���� ������ �������� ����
    triangulation.erase(std::remove_if(triangulation.begin(), triangulation.end(),
        [&](Triangle t) { return std::find(bad_triangles.begin(), bad_triangles.end(), t) != bad_triangles.end(); }),
        triangulation.end());

    // polygon�� �� edge�� ���� �ٰ��� ������ �ٽ� �ﰢȭ
    for (const std::pair<Point, Point>& edge : polygon)
    {
        // edge�� point�κ��� �ﰢ���� ����
        Triangle new_triangle(point, edge.first, edge.second);
        // newTri�� triangulation�� �߰�
        triangulation.push_back(new_triangle);
    }
}

std::vector<Triangle> Triangulate(std::vector<Point> points)
{
    std::vector<Triangle> triangulation;

    // super - triangle�� triangulation�� �߰�
    // ��� ���� ������ ������ �� �ֵ��� ����� ū �ﰢ��
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

    // ��� ���� �ϳ��� �ﰢȭ�� �߰�
    for (const Point& p : points)
    {
        AddPoint(triangulation, p);
    }

    // ���� super - triangle�� ������ �����ϴ� ��� �ش� �ﰢ���� triangulation���� ����
    triangulation.erase(
        std::remove_if(
            triangulation.begin(), triangulation.end(),
            [&](Triangle t) {
                return std::find_if(std::begin(t.points), std::end(t.points), [&](const Point& p) {return bounding_triangle.hasPoint(p); }) != std::end(t.points);
            }
    ), triangulation.end());


    // triangulation�� ��ȯ
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
//    // pointList�� �ﰢȭ�� ���� �����ϴ� ��ǥ �����Դϴ�.
//    triangulation : = �� �ﰢ�� �޽� ������ ����
//    super - triangle�� triangulation�� �߰��մϴ�. // ��� ���� ������ ������ �� �ֵ��� ����� ū �ﰢ���̾�� �մϴ�.
//    pointList�� �� ���� ���� // ��� ���� �ϳ��� �ﰢȭ�� �߰��մϴ�.
//        badTriangles : = �� ����
//        triangulation�� �� �ﰢ���� ���� // �������� ���� �� �̻� ��ȿ���� ���� ��� �ﰢ���� ã���ϴ�.
//            if point�� �ﰢ���� �� ���ο� �ִٸ�
//                badTriangles�� �ﰢ���� �߰��մϴ�.
//        polygon : = �� ����
//        badTriangles�� �� �ﰢ���� ���� // �ٰ��� ������ ��踦 ã���ϴ�.
//            �ﰢ���� �� ���� ����
//                if �ٸ� badTriangles�� ���� �������� �ʴ´ٸ�
//                    polygon�� edge�� �߰��մϴ�.
//        badTriangles�� �� �ﰢ���� �����մϴ�. // ������ �������� �����մϴ�.
//        polygon�� �� edge�� ���� // �ٰ��� ������ �ٽ� �ﰢȭ�մϴ�.
//            edge�� point�κ��� �ﰢ���� �����մϴ�.
//            newTri�� triangulation�� �߰��մϴ�.
//    triangulation�� �� �ﰢ���� ���� // �� ������ �Ϸ�Ǿ����ϴ�. ���� �����մϴ�.
//        ���� super - triangle�� ������ �����ϴ� ���
//            triangulation���� �ﰢ���� �����մϴ�.
//    triangulation�� ��ȯ�մϴ�.

// ������ ������ �����ϴ� ���
// ������ ���������� �ܽ��� ������ Ȱ��
//   1. �� �� A-B-C�� ���� ��, ���� A-B/B-C�� �߰��� �����鼭 �� ���п� ������ ������ �������� ����
//   2. �� ������ ������ ���� �������� �߽�(�ܽ�)

// ���� �����ϴ� �˰���
// ��ǥ: ���� A-B�� �߽��� �����鼭 ������ ������ ������
// �˰� �ִ� ��: ������ ��(A-B �߽�)/������ ����/������ ���
// 1. �߽����� �����鼭, A-B�� ������ ����� �������� ����
// 2. �� ����� ��ġ�� ������ �������� ����

//1. A-B�� ���� q����
//2. ��q�� ������ ���� A->B�� �������ͷ� ������ ��� L ����
//3. ��� L�� M�� ����(����)�� ������ ����

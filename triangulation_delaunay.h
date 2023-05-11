#pragma once
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <glm/glm.hpp>
#include <set>

typedef glm::vec3 Vertex;

struct IndexVertex {
    glm::vec3 vertex = glm::vec3(.0f);
    int index = 0;

    IndexVertex() {}
    IndexVertex(glm::vec3 _vertex, int _index) : vertex(_vertex), index(_index) {}
    //IndexVertex(const IndexVertex& _iv) : vertex(_iv.vertex), index(_iv.index) {}

    bool operator<(const IndexVertex& v) const {
        if (vertex.x == v.vertex.x) {
            return vertex.y < v.vertex.y;
        }
        return vertex.x < v.vertex.x;
    }

    bool operator==(const IndexVertex& iv) const {
        return vertex[0] == iv.vertex[0] && vertex[1] == iv.vertex[1] && vertex[2] == iv.vertex[2];
    }

}typedef idxVertex;

struct Triangle
{
    idxVertex a, b, c;
    idxVertex points[3];
    Triangle(idxVertex a, idxVertex b, idxVertex c) : a(a), b(b), c(c) {
        points[0] = a;
        points[1] = b;
        points[2] = c;
    }

    bool operator==(const Triangle& t) const {
        std::set<idxVertex> s;
        s.insert(a), s.insert(b), s.insert(c);
        s.insert(t.a), s.insert(t.b), s.insert(t.c);
        return s.size() == 3;
    }
    void printStatus() {
        printf("(%f, %f)(%f, %f)(%f, %f)\n", a.vertex.x, a.vertex.y, b.vertex.x, b.vertex.y, c.vertex.x, c.vertex.y);
    }

    bool hasPoint(const idxVertex& p) {
        return points[0] == p || points[1] == p || points[2] == p;
    }
};

typedef std::pair<idxVertex, idxVertex> Line;
template <>
bool std::operator==(const Line& lhs, const Line& rhs) {
    return (lhs.first == rhs.first && lhs.second == rhs.second) || (lhs.first == rhs.second && lhs.second == rhs.first);
}
//
// �ߺ��� ���� ���ٰ� ����
bool CircumCircle(idxVertex p1, idxVertex p2, idxVertex p3, glm::vec3& center, double& radius)
{
    double fabsy1y2 = std::abs(p1.vertex.y - p2.vertex.y);
    double fabsy2y3 = std::abs(p2.vertex.y - p3.vertex.y);
    double xc, yc;
    if (fabsy1y2 < 1e-10 && fabsy2y3 < 1e-10) {
        return false;
    }
    else if (fabsy1y2 < 1e-10)
    {
        double m2 = -((p3.vertex.x - p2.vertex.x) / (p3.vertex.y - p2.vertex.y));
        double mx2 = (p2.vertex.x + p3.vertex.x) / 2.0;
        double my2 = (p2.vertex.y + p3.vertex.y) / 2.0;
        xc = (p2.vertex.x + p1.vertex.x) / 2.0;
        yc = m2 * (xc - mx2) + my2;
    }
    else if (fabsy2y3 < 1e-10)
    {
        double m1 = -((p2.vertex.x - p1.vertex.x) / (p2.vertex.y - p1.vertex.y));
        double mx1 = (p1.vertex.x + p2.vertex.x) / 2.0;
        double my1 = (p1.vertex.y + p2.vertex.y) / 2.0;
        xc = (p3.vertex.x + p2.vertex.x) / 2.0;
        yc = m1 * (xc - mx1) + my1;
    }
    else
    {
        double m1 = -((p2.vertex.x - p1.vertex.x) / (p2.vertex.y - p1.vertex.y));
        double m2 = -((p3.vertex.x - p2.vertex.x) / (p3.vertex.y - p2.vertex.y));
        double mx1 = (p1.vertex.x + p2.vertex.x) / 2.0;
        double mx2 = (p2.vertex.x + p3.vertex.x) / 2.0;
        double my1 = (p1.vertex.y + p2.vertex.y) / 2.0;
        double my2 = (p2.vertex.y + p3.vertex.y) / 2.0;
        xc = (m1 * mx1 - m2 * mx2 + my2 - my1) / (m1 - m2);
        yc = m1 * (xc - mx1) + my1;
    }
    double dx = p2.vertex.x - xc;
    double dy = p2.vertex.y - yc;
    radius = std::sqrt(dx * dx + dy * dy);
    center = glm::vec3(xc, yc, .0f);
    return true;
}

bool InCircle(idxVertex p, Triangle t)
{
    glm::vec3 center;
    double radius;
    if (!CircumCircle(t.a, t.b, t.c, center, radius))
        return false;
    double dx = p.vertex.x - center.x;
    double dy = p.vertex.y - center.y;
    double dist_squared = dx * dx + dy * dy;
    return (dist_squared <= radius * radius);
}

void AddPoint(std::vector<Triangle>& triangulation, idxVertex point)
{
    std::vector<Triangle> bad_triangles;
    std::vector<Line> polygon;

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
        idxVertex tmp[3] = { t.a, t.b, t.c };
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
    for (const Line& edge : polygon)
    {
        // edge�� point�κ��� �ﰢ�� ����
        Triangle new_triangle(point, edge.first, edge.second);
        triangulation.push_back(new_triangle);
    }
}

bool isOutside(std::vector<Line>& lines, Triangle& t) {
    bool outsideTrigger = false;
    int sameLineCount = 0;

    for (auto p : { (t.a.vertex + t.b.vertex) /= 2, (t.b.vertex + t.c.vertex) /= 2, (t.c.vertex + t.a.vertex) /= 2 }) {
        int intersectionCount = 0;
        bool isSameLine = false;

        for (int i = 0; i < lines.size(); i++) {
            auto ax = lines[i].first.vertex.x;
            auto ay = lines[i].first.vertex.y;
            auto bx = lines[i].second.vertex.x;
            auto by = lines[i].second.vertex.y;


            if (p.x == (ax + bx) / 2 && p.y == (ay + by) / 2) {
                sameLineCount++;
                isSameLine = true;
                break;
            }

            //�� B�� ���� (p[i], p[j])�� y��ǥ ���̿� ����
            if ((lines[i].first.vertex.y > p.y) != (lines[i].second.vertex.y > p.y)) {
                double atX = (lines[i].second.vertex.x - lines[i].first.vertex.x) * (p.y - lines[i].first.vertex.y) / (lines[i].second.vertex.y - lines[i].first.vertex.y) + lines[i].first.vertex.x;
                if (p.x < atX) {
                    intersectionCount++;
                }
            }
        }

        if (isSameLine == false && intersectionCount % 2 == 0) {
            outsideTrigger = true;
        }
    }

    return sameLineCount != 3 && outsideTrigger;
}

// �ߺ��� ���� ���ٰ� ����
std::vector<Triangle> Triangulate(std::vector<Triangle>& triangulation, Vertex vertices[], int pointCount)
{
    // �ٰ����� ������ ���� ����
    std::vector<Line> polyline;
    for (int i = 0; i < pointCount; ++i) {
        polyline.push_back(std::make_pair(idxVertex(vertices[i], i), idxVertex(vertices[i + 1], i + 1)));
    }

    // super - triangle�� triangulation�� �߰�
    // ��� ���� ������ ������ �� �ֵ��� ����� ū �ﰢ��
    double xmin = vertices[0].x;
    double ymin = vertices[0].y;
    double xmax = xmin;
    double ymax = ymin;
    for (int i = 0; i < pointCount; ++i) {
        if (vertices[i].x < xmin) xmin = vertices[i].x;
        if (vertices[i].y < ymin) ymin = vertices[i].y;
        if (vertices[i].x > xmax) xmax = vertices[i].x;
        if (vertices[i].y > ymax) ymax = vertices[i].y;
    }

    double dx = xmax - xmin;
    double dy = ymax - ymin;
    double dmax = std::max(dx, dy);
    double xmid = xmin + dx / 2.0;
    double ymid = ymin + dy / 2.0;

    idxVertex p1(glm::vec3(xmid - 20 * dmax, ymid - dmax, .0f), pointCount);
    idxVertex p2(glm::vec3(xmid, ymid + 20 * dmax, .0f), pointCount + 1);
    idxVertex p3(glm::vec3(xmid + 20 * dmax, ymid - dmax, .0f), pointCount + 2);

    Triangle bounding_triangle(p1, p2, p3);
    triangulation.push_back(bounding_triangle);

    // ��� ���� �ϳ��� �ﰢȭ�� �߰�
    for (int i = 0; i < pointCount; ++i) {
        AddPoint(triangulation, idxVertex(vertices[i], i));
    }

    // ���� super - triangle�� ������ �����ϴ� ��� �ش� �ﰢ���� triangulation���� ����
    triangulation.erase(
        std::remove_if(
            triangulation.begin(), triangulation.end(),
            [&](Triangle t) {
                return std::find_if(std::begin(t.points), std::end(t.points), [&](const idxVertex& p) {return bounding_triangle.hasPoint(p); }) != std::end(t.points);
            }
    ), triangulation.end());

    //������ �ܰ� �ﰢ�� �߰�
    //�ﰢ���� �� �� �ϳ��� �ٰ����� �ۿ� ���� ���
    //�ﰢ���� ���� �߽��� �ϳ��� �ٷ��
    triangulation.erase(
        std::remove_if(
            triangulation.begin(), triangulation.end(),
            [&](Triangle& t) {return isOutside(polyline, t); }
    ), triangulation.end());

    return triangulation;
}

//int main()
//{
//    //std::vector<idxVertex> points = { idxVertex({0, 2, .0f}, 8), idxVertex({-1,0, .0f}, 7), idxVertex({1,0, .0f}, 5), idxVertex({2,2, .0f}, 6), idxVertex({-2,2, .0f}, 3), idxVertex({0, 3, .0f}, 4)};
//    //Triangle tri1 = Triangle{ idxVertex({0, 1, .0f}, 8), idxVertex({-1,0, .0f}, 7), idxVertex({1,0, .0f}, 5) };
//    //Vertex points[] = {{0, 1, .0f}, {-1,0, .0f},{1,0, .0f},{0,-1, .0f}};
//
//    //Vertex points[] = { {0, 2, .0f}, {-1,0, .0f},{1,0, .0f},{2,2, .0f},{-2,2, .0f},{0, 3, .0f} };
//    //std::vector<Triangle> triangulation;
//    //Triangulate(triangulation, points, 6);
//    //int* indices = new int[triangulation.size() * 3];
//
//    std::vector<Triangle> triangulation;
//    Vertex points[] = { {1, .0f, .0f}, {1, -1, .0f}, {-1, -1, .0f}, {-1, 1, .0f}, {0, 1, .0f}, {.0f, .0f, .0f}, {1, .0f, .0f} };
//    Triangulate(triangulation, points, 6);
//
//    std::cout << "tri " << triangulation.size() << std::endl;
//
//    for (int i = 0; i < triangulation.size(); i++) {
//        std::cout << "(" << triangulation[i].a.vertex.x << "," << triangulation[i].a.vertex.y << "," << triangulation[i].a.vertex.z << ")";
//        std::cout << "(" << triangulation[i].b.vertex.x << "," << triangulation[i].b.vertex.y << "," << triangulation[i].b.vertex.z << ")";
//        std::cout << "(" << triangulation[i].c.vertex.x << "," << triangulation[i].c.vertex.y << "," << triangulation[i].c.vertex.z << ")";
//        std::cout << std::endl;
//    }
//
//    //for (int i = 0; i < triangulation.size(); i++){
//    //    indices[i * 3] = triangulation[i].a.index;
//    //    indices[i * 3 + 1] = triangulation[i].b.index;
//    //    indices[i * 3 + 2] = triangulation[i].c.index;
//    //}
//
//    ////for (const Triangle& t : triangulation)
//    //for (int i = 0; i < triangulation.size() * 3; i += 3)
//    //{
//    //    std::cout << "(" << indices[i] << indices[i + 1] << indices[i + 2] << ") " << std::endl;
//
//    //    //std::cout << "(" << t.a.index << ") ";
//    //    //std::cout << "(" << t.b.index << ") ";
//    //    //std::cout << "(" << t.c.index << ") " << std::endl;
//
//    //    //std::cout << "(" << t.a.vertex.x << ", " << t.a.vertex.y << ") ";
//    //    //std::cout << "(" << t.b.vertex.x << ", " << t.b.vertex.y << ") ";
//    //    //std::cout << "(" << t.c.vertex.x << ", " << t.c.vertex.y << ")" << std::endl;
//    //}
//
//    //delete[] indices;
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

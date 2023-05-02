#pragma once
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

struct Point
{
    double x, y;
    Point(double x = 0.0f, double y = 0.0f) : x(x), y(y) {}
    bool operator==(const Point& p) const {
        return (this->x == p.x && this->y == p.y);
    }
};

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
};

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
    center = Point(xc, yc);
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
    for (const Triangle& t : triangulation)
    {
        if (InCircle(point, t))
        {
            bad_triangles.push_back(t);
            polygon.push_back(std::make_pair(t.a, t.b));
            polygon.push_back(std::make_pair(t.b, t.c));
            polygon.push_back(std::make_pair(t.c, t.a));
        }
    }

    triangulation.erase(std::remove_if(triangulation.begin(), triangulation.end(),
        [&](Triangle t) { return std::find(bad_triangles.begin(), bad_triangles.end(), t) != bad_triangles.end(); }),
        triangulation.end());

    for (const std::pair<Point, Point>& edge : polygon)
    {
        Triangle new_triangle(point, edge.first, edge.second);
        triangulation.push_back(new_triangle);
    }
}

std::vector<Triangle> Triangulate(std::vector<Point> points)
{
    std::vector<Triangle> triangulation;
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

    Point p1(xmid - 20 * dmax, ymid - dmax);
    Point p2(xmid, ymid + 20 * dmax);
    Point p3(xmid + 20 * dmax, ymid - dmax);

    Triangle bounding_triangle(p1, p2, p3);
    triangulation.push_back(bounding_triangle);

    for (const Point& p : points)
    {
        AddPoint(triangulation, p);
        std::cout << "-----" << std::endl;
    }

    triangulation.erase(
        std::remove_if(
            triangulation.begin(), triangulation.end(),
            [&](Triangle t) {
                return std::find_if(std::begin(t.points), std::end(t.points), [&](Point p) { return std::abs(p.x - xmid) > 1.5 * dmax || std::abs(p.y - ymid) > 1.5 * dmax; }) != std::end(t.points);
            }
        ),
        triangulation.end()
                );

    return triangulation;
}

/*
int main()
{
    std::vector<Point> points = { {0,0}, {0,1}, {1,0}, {1,1} };
    std::vector<Triangle> triangulation = Triangulate(points);
    for (const Triangle& t : triangulation)
    {
        std::cout << "(" << t.a.x << ", " << t.a.y << ") ";
        std::cout << "(" << t.b.x << ", " << t.b.y << ") ";
        std::cout << "(" << t.c.x << ", " << t.c.y << ")" << std::endl;
    }

    return 0;
}
*/

/*
Expected Output:
(0, 0) (0, 1) (1, 0)
(0, 1) (1, 0) (1, 1)
(0, 0) (1, 0) (1, 1)
(0, 0) (0, 1) (1, 1)
*/

#pragma once
#include <iostream>
#include <list>
#include <vector>

namespace Triangulation {

    struct Vertex
    {
        double x, y, z;
        Vertex() {}
        Vertex(double _x, double _y, double _z) : x(_x), y(_y), z(_z) {}
    };

    const Vertex minusVertex(const Vertex &v1, const Vertex &v2)
    {
        return Vertex(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
    }

    Vertex cross(const Vertex &a, const Vertex &b)
    {
        return Vertex(
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x);
    }

    struct IndexVertex
    {
        Vertex vertex;
        unsigned index;

        IndexVertex(Vertex _vertex, unsigned _index) : vertex(_vertex), index(_index){};
    };

    // https://bloodstrawberry.tistory.com/996
    struct Triangle
    {
        unsigned a, b, c;
        Triangle(unsigned _a, unsigned _b, unsigned _c) : a(_a), b(_b), c(_c) {}
    };

    bool isVertexInsideTriangle(const Vertex &v0, const Vertex &v1, const Vertex &v2, const Vertex &p)
    {
        auto m = cross(minusVertex(p, v0), minusVertex(v1, v0));
        auto n = cross(minusVertex(p, v1), minusVertex(v2, v1));
        auto o = cross(minusVertex(p, v2), minusVertex(v0, v2));

        return m.z > 0 && n.z > 0 && o.z > 0;
    }

    bool isVerticesInsideTriangle(const std::vector<IndexVertex> &v, const int &index)
    {
        Vertex v0 = v[index].vertex;
        Vertex v1 = v[index + 1].vertex;
        Vertex v2 = v[index + 2].vertex;

        for (int i = index + 3; i < v.size(); i++)
        {
            if (isVertexInsideTriangle(v0, v1, v2, v[i].vertex))
            {
                return true;
            }
        }

        return false;
    }

    bool isCW(Vertex v0, Vertex v1, Vertex v2)
    {
        auto p = minusVertex(v1, v0);
        auto q = minusVertex(v2, v1);

        return cross(p, q).z < 0;
    }

    void Triangulate(std::vector<Triangle> &result, Vertex vertices[], int verticesLength, int32_t* partVertexCounts, int partCount)
    {
        std::vector<IndexVertex> v;
        v.reserve(verticesLength);
        //for (unsigned int idx = 0; idx < verticesLength - 1; ++idx)
        //{
        //    v.push_back(IndexVertex(vertices[idx], idx));
        //}
        for (unsigned int idx = 0; idx < verticesLength; ++idx)
        {
            v.push_back(IndexVertex(vertices[idx], idx));
        }

        result.reserve(verticesLength - 2);
        for (int i = 0; i < verticesLength  - 2; i++)
        {
            for (int j = 0; j < v.size() - 2; ++j)
            {
                bool cw = isCW(v[j].vertex, v[j + 1].vertex, v[j + 2].vertex);
                bool cross = isVerticesInsideTriangle(v, j);

                if (cw == true && cross == false)
                {
                    result.push_back(Triangle(v[j].index, v[j + 1].index, v[j + 2].index));
                    v.erase(v.begin() + j + 1);
                    break;
                }
            }
        }
    }
}

int main()
{
    // Vertex points[] = {{0, 1, .0f}, {-1,0, .0f},{1,0, .0f},{0,-1, .0f}};
    // Vertex points[] = { {0, 2, .0f}, {-1,0, .0f},{1,0, .0f},{2,2, .0f},{-2,2, .0f},{0, 3, .0f} };
    // Vertex points[] = { {1, .0f, .0f}, {1, -1, .0f}, {-1, -1, .0f}, {-1, 1, .0f}, {0, 1, .0f}, {.0f, .0f, .0f}, {1, .0f, .0f} };
    //Triangulation::Vertex points[] = { {1144668.03, 1688422.46, .0}, {1144684.23, 1688381.42, .0}, {1144672.0, 1688376.26, .0}, {1144667.235, 1688390.08680, .0}, {1144667.2335, 1688390.0906, .0}, {1144662.144, 1688401.688, .0}, {1144656.1, 1688417.6, .0}, {1144668.03, 1688422.46, .0} };

    // ///
    // float a = .707107f;
    // Vertex points[] = { {1, 0, 0}, { a, -a, .0}, {0, -1, 0}, {-a, -a, 0}, {-1, 0, 0}, {-a, a, 0},{0, 1, 0}, {a, a, 0}, {1, 0, 0} };
    //std::vector<Triangulation::Triangle> triangulation;
    //Triangulate(triangulation, points, 8);
    //int *indices = new int[triangulation.size() * 3];

    //std::cout << "tri " << triangulation.size() << std::endl;
    ///
    // for (int i = 0; i < triangulation.size(); i++) {
    //     std::cout << "(\t" << triangulation[i].a << ",\t" << triangulation[i].b << ",\t" << triangulation[i].c << ")" << std::endl;
    // }

    // delete[] indices;

    return 0;
}

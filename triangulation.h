#ifndef TRIANGULATION_H
#define TRIANGULATION_H

#include <vector>
#include <QDebug>
using VertexIndice = unsigned;

struct Vec2
{
    float x,y;
public:
    bool operator==(const Vec2& rhs) const { return (this->x == rhs.x && this->y == rhs.y);}
};
QDebug operator<< (QDebug d, const Vec2 &vec2);

float sqrDist(const Vec2&, const Vec2&);
float sqrLen(const Vec2&);

struct Triangle
{
    union
    {
        struct { VertexIndice a,b,c; };
        VertexIndice arr[3];
    };
};

class Triangulation
{
public:
    //Generates a regular, square triangulation over (0,0),(1,1) with n subdivisions (0 being a single square made of 2 triangles)
    explicit Triangulation(std::size_t n=0);
    Triangulation(std::vector<Vec2> vertices, std::vector<Triangle> triangles);
    Vec2 operator[](VertexIndice) const;
    Vec2& operator[](VertexIndice);
    const std::vector<Vec2>& vertices() const;
    const std::vector<Triangle>& triangles() const;
    const std::vector<std::vector<std::pair<unsigned,unsigned char>>>& trianglesPerVertex() const; //Format : for each vertex, a list of pair<triangleIndex,locInTriangle>
    VertexIndice size() const;
    float triangleArea(unsigned) const;
    bool isEdgeVertex(VertexIndice) const;

    void splitTriangle(unsigned triangle);
    void flipCommonEdge(unsigned l_triangle, unsigned r_triangle);
    void deleteVertex(VertexIndice);
    void deleteEdge(VertexIndice, VertexIndice);
    void deleteTriangle(unsigned);

    friend auto begin(Triangulation& t) { return t._vertices.begin(); }
    friend auto begin(const Triangulation& t) { return t._vertices.begin(); }
    friend auto cbegin(const Triangulation& t) { return begin(t); }
    friend auto end(Triangulation& t) { return t._vertices.end(); }
    friend auto end(const Triangulation& t) { return t._vertices.end(); }
    friend auto cend(const Triangulation& t) { return end(t); }
private:
    void addTrianglePerVertex(unsigned);
    void removeTrianglePerVertex(unsigned);
    void computeTrianglesPerVertex();
    std::vector<Vec2> _vertices;
    std::vector<Triangle> _triangles;
    std::vector<std::vector<std::pair<unsigned,unsigned char>>> _triangles_per_vertex;
};

#endif // TRIANGULATION_H

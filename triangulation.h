#ifndef TRIANGULATION_H
#define TRIANGULATION_H

#include <vector>

using VertexIndice = unsigned;

struct Vec2
{
    float x,y;
};

struct Triangle
{
    VertexIndice a,b,c;
};

class Triangulation
{
public:
    //Generates a regular, square triangulation over (0,0),(1,1) with n subdivisions (0 being a single square made of 2 triangles)
    explicit Triangulation(std::size_t n=0);
    Vec2 operator[](VertexIndice) const;
    Vec2& operator[](VertexIndice);
    const std::vector<Vec2>& vertices() const;
    const std::vector<Triangle>& triangles() const;
    VertexIndice size() const;

    friend auto begin(Triangulation& t) { return t._vertices.begin(); }
    friend auto begin(const Triangulation& t) { return t._vertices.begin(); }
    friend auto cbegin(const Triangulation& t) { return begin(t); }
    friend auto end(Triangulation& t) { return t._vertices.end(); }
    friend auto end(const Triangulation& t) { return t._vertices.end(); }
    friend auto cend(const Triangulation& t) { return end(t); }
private:
    std::vector<Vec2> _vertices;
    std::vector<Triangle> _triangles;
};

#endif // TRIANGULATION_H

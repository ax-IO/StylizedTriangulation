#ifndef TRIANGULATION_H
#define TRIANGULATION_H

#include <vector>

using VertexIndice = std::size_t;

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

private:
    std::vector<Vec2> _vertices;
    std::vector<Triangle> _triangles;
};

#endif // TRIANGULATION_H

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
    Vec2 operator[](VertexIndice) const;
    Vec2& operator[](VertexIndice);
    std::vector<Triangle> triangles() const;
    VertexIndice size() const;
};

#endif // TRIANGULATION_H

#include "triangulation.h"

Triangulation::Triangulation(std::size_t n)
    :_vertices((n+2)*(n+2)), _triangles((n+1)*(n+1)*2)
{
    const std::size_t LineSize = n+2;
    auto next_vertice = begin(_vertices);
    auto next_triangle = begin(_triangles);

    for(VertexIndice i = 0; i < LineSize; ++i)
    {
        float i_coord = (float)i / (LineSize-1);
        for(VertexIndice j = 0; j < LineSize; ++j)
        {
            float j_coord = (float)j / (LineSize-1);
            *(next_vertice++) = {i_coord,j_coord};
        }
    }

    for(VertexIndice i = 0; i < LineSize-1; ++i)
    {
        for(VertexIndice j = 0; j < LineSize-1; ++j)
        {
            *(next_triangle++) = {i+j*LineSize,
                                  i+(j+1)*LineSize,
                                  i+1+j*LineSize};

            *(next_triangle++) = {i+(j+1)*LineSize,
                                  i+1+(j+1)*LineSize,
                                  i+1+j*LineSize};
        }
    }
}

Vec2 Triangulation::operator[](VertexIndice i) const
{
    return _vertices[i];
}

Vec2& Triangulation::operator[](VertexIndice i)
{
    return _vertices[i];
}

const std::vector<Vec2>& Triangulation::vertices() const
{
    return _vertices;
}

const std::vector<Triangle>& Triangulation::triangles() const
{
    return _triangles;
}

VertexIndice Triangulation::size() const
{
    return _vertices.size();
}

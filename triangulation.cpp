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

void Triangulation::splitTriangle(unsigned triangle)
{
    unsigned next_tris = _triangles.size();
    _triangles.resize(next_tris + 2);
    Triangle& orig = _triangles[triangle];
    Triangle& first = _triangles[next_tris];
    Triangle& second = _triangles[next_tris+1];
    unsigned new_vert = _vertices.size();
    _vertices.push_back({(_vertices[orig.a].x + _vertices[orig.b].x + _vertices[orig.c].x) / 3,
                (_vertices[orig.a].y + _vertices[orig.b].y + _vertices[orig.c].y) / 3});

    first.a = orig.b;
    first.b = orig.c;
    first.c = new_vert;

    second.a = orig.c;
    second.b = orig.a;
    second.c = new_vert;

    orig.c = new_vert;
}

void Triangulation::flipCommonEdge(unsigned l_triangle, unsigned r_triangle)
{
    if(l_triangle == r_triangle) return;

    VertexIndice* const lptrs[]{&_triangles[l_triangle].a, &_triangles[l_triangle].b, &_triangles[l_triangle].c};
    VertexIndice* const rptrs[]{&_triangles[r_triangle].a, &_triangles[r_triangle].b, &_triangles[r_triangle].c};
    int l_edge_start;
    int r_edge_start;
    int common_count=0;
    for(int i = 0; i < 3; ++i)
    {
        for(int j = 0; j < 3; ++j)
        {
            if(*lptrs[i] != *rptrs[j]) continue;
            if(!common_count || l_edge_start == (i+1)%3) l_edge_start = i;
            if(!common_count || r_edge_start == (j+1)%3) r_edge_start = j;
            ++common_count;
        }
    }
    if(common_count != 2) return;

    *lptrs[l_edge_start] = *rptrs[(r_edge_start+2)%3];
    *rptrs[r_edge_start] = *lptrs[(l_edge_start+2)%3];
}

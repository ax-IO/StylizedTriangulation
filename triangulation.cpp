#include "triangulation.h"
#include <algorithm>
#include <iterator>
#include <limits>

float sqrLen(const Vec2& x)
{
    return x.x * x.x + x.y * x.y;
}

float sqrDist(const Vec2& a, const Vec2& b)
{
    return sqrLen({b.x-a.x, b.y-a.y});
}



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
    computeTrianglesPerVertex();
}

Triangulation::Triangulation(std::vector<Vec2> vertices, std::vector<Triangle> triangles)
    :_vertices(vertices), _triangles(triangles)
{
    computeTrianglesPerVertex();
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

const std::vector<std::vector<std::pair<unsigned,unsigned char>>>& Triangulation::trianglesPerVertex() const
{
    return _triangles_per_vertex;
}

VertexIndice Triangulation::size() const
{
    return _vertices.size();
}

float Triangulation::triangleArea(unsigned t) const
{
    Vec2 a = _vertices[_triangles[t].a];
    Vec2 b = _vertices[_triangles[t].b];
    Vec2 c = _vertices[_triangles[t].c];
    return std::abs((b.x - a.x) * (c.y - a.y) - (c.x - a.x) * (b.y - a.y)) / 2;
}

bool Triangulation::isEdgeVertex(VertexIndice i) const
{
    return _vertices[i].x == 0 || _vertices[i].x == 1 || _vertices[i].y == 0 || _vertices[i].y == 1;
}

void Triangulation::splitTriangle(unsigned triangle)
{
    removeTrianglePerVertex(triangle);

    unsigned next_tris = _triangles.size();
    _triangles.resize(next_tris + 2);
    Triangle& orig = _triangles[triangle];
    Triangle& first = _triangles[next_tris];
    Triangle& second = _triangles[next_tris+1];
    unsigned new_vert = _vertices.size();
    _vertices.push_back({(_vertices[orig.a].x + _vertices[orig.b].x + _vertices[orig.c].x) / 3,
                (_vertices[orig.a].y + _vertices[orig.b].y + _vertices[orig.c].y) / 3});
    _triangles_per_vertex.emplace_back();

    first.a = orig.b;
    first.b = orig.c;
    first.c = new_vert;

    second.a = orig.c;
    second.b = orig.a;
    second.c = new_vert;

    orig.c = new_vert;

    addTrianglePerVertex(triangle);
    addTrianglePerVertex(next_tris);
    addTrianglePerVertex(next_tris+1);
}

void Triangulation::flipCommonEdge(unsigned l_triangle, unsigned r_triangle)
{
    if(l_triangle == r_triangle) return;

    removeTrianglePerVertex(l_triangle);
    removeTrianglePerVertex(r_triangle);

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

    addTrianglePerVertex(l_triangle);
    addTrianglePerVertex(r_triangle);
}


void Triangulation::deleteVertex(VertexIndice to_del)
{
    if(isEdgeVertex(to_del)) return;

    VertexIndice closest;
    float min_sqr_dist = std::numeric_limits<float>::infinity();
    for(auto [t, pos] : _triangles_per_vertex[to_del])
    {
        for(int i = 0; i < 2; ++i)
        {
            VertexIndice other = _triangles[t].arr[(pos+i+1)%3];
            float sqr_dist = sqrDist(_vertices[to_del], _vertices[other]);
            if(sqr_dist < min_sqr_dist)
            {
                min_sqr_dist = sqr_dist;
                closest = other;
            }
        }
    }

    deleteEdge(to_del, closest);
}

void Triangulation::deleteEdge(VertexIndice from, VertexIndice to)
{
    if(isEdgeVertex(to)) std::swap(from, to);
    if(isEdgeVertex(to)) return; //Cannot delete an... "edge edge"

    if(!isEdgeVertex(from))
    {
        _vertices[from].x = (_vertices[from].x + _vertices[to].x) / 2;
        _vertices[from].y = (_vertices[from].y + _vertices[to].y) / 2;
    }

    _vertices.erase(_vertices.begin() + to);
    _triangles_per_vertex[from].insert(_triangles_per_vertex[from].end(), std::make_move_iterator(_triangles_per_vertex[to].begin()), std::make_move_iterator(_triangles_per_vertex[to].end()));
    _triangles_per_vertex.erase(_triangles_per_vertex.begin() + to);

    unsigned new_t_id = 0;
    for(unsigned old_t_id = 0; old_t_id < _triangles.size(); ++old_t_id)
    {
        Triangle& triangle = _triangles[new_t_id];
        if(old_t_id != new_t_id) triangle = std::move(_triangles[old_t_id]);
        int edge_elems = 0;
        for(VertexIndice& vi : triangle.arr)
        {
            if(vi == to) vi = from;
            if(vi == from) ++edge_elems;
            if(vi > to) --vi;
        }
        if(edge_elems >= 2)
        {
            removeTrianglePerVertex(new_t_id);
            for(auto& list : _triangles_per_vertex)
            {
                for(auto& [t, pos] : list)
                {
                    if(t > new_t_id) --t;
                }
            }
        }
        else
        {
            ++new_t_id;
        }
    }
    _triangles.erase(_triangles.begin() + new_t_id, _triangles.end());
}

void Triangulation::deleteTriangle(unsigned t)
{
    int shortest_edge;
    float shortest_sqr_dist = std::numeric_limits<float>::infinity();
    for(int i = 0; i < 3; ++i)
    {
        float dist = sqrDist(_vertices[_triangles[t].arr[i]], _vertices[_triangles[t].arr[(i+1)%3]]);
        if(dist < shortest_sqr_dist)
        {
            shortest_edge = i;
            shortest_sqr_dist = dist;
        }
    }
    deleteEdge(_triangles[t].arr[shortest_edge], _triangles[t].arr[(shortest_edge+1)%3]);
}

void Triangulation::addTrianglePerVertex(unsigned t)
{
    Triangle triangle = _triangles[t];
    _triangles_per_vertex[triangle.a].emplace_back(t,0);
    _triangles_per_vertex[triangle.b].emplace_back(t,1);
    _triangles_per_vertex[triangle.c].emplace_back(t,2);
}

void Triangulation::removeTrianglePerVertex(unsigned t)
{
    auto f = [t](auto& item) { return item.first == t; };
    Triangle triangle = _triangles[t];
    VertexIndice vertices[]{triangle.a, triangle.b, triangle.c};
    for(VertexIndice v : vertices) _triangles_per_vertex[v].erase(std::remove_if(begin(_triangles_per_vertex[v]), end(_triangles_per_vertex[v]), f), end(_triangles_per_vertex[v]));
}

void Triangulation::computeTrianglesPerVertex()
{
    _triangles_per_vertex.clear();
    _triangles_per_vertex.resize(size());
    for(unsigned i = 0; i < _triangles.size(); ++i)
    {
        addTrianglePerVertex(i);
    }
}

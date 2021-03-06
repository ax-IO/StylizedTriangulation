#include "triangulationoptimizer.h"
#include <QOpenGLShader>
#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>
#include <cmath>
#include <algorithm>

struct TriangulationOptimizer::ErrorData
{
    GLuint error_total = 0;
    GLuint count = 0;
};

namespace
{
    template<typename T>
    bool sens(const Triangle& t, const T& tri)
    {
        Vec2 a = tri[t.a];
        Vec2 l = tri[t.b];
        Vec2 r = tri[t.c];
        Vec2 al{l.x - a.x, l.y - a.y};
        Vec2 ar{r.x - a.x, r.y - a.y};
        return al.x * ar.y - ar.x * al.y > 0;
    }

    float angle(Vec2 l, Vec2 a, Vec2 r)
    {
        using std::sqrt, std::acos;
        Vec2 al{l.x - a.x, l.y - a.y};
        Vec2 ar{r.x - a.x, r.y - a.y};
        return acos((al.x * ar.x + al.y * ar.y) / sqrt(sqrLen(al)) / sqrt(sqrLen(ar)));
    }

    struct TriangleData
    {
        GLuint color_total_r = 0;
        GLuint color_total_g = 0;
        GLuint color_total_b = 0;
        GLuint count = 0;
    };

    auto computeRegularisationGradients(const Triangulation& triangulation, const std::vector<std::vector<std::pair<unsigned,unsigned char>>>& triangles_per_vertex, float pixel_width, float pixel_height)
    {
        std::vector<Vec2> result(triangulation.size());
        for(VertexIndice i = 0; i < result.size(); ++i)
        {
            auto& neighbor_tris = triangles_per_vertex[i];
            Vec2 point = triangulation[i];
            float energies[4]{};
            Vec2 offsets[4]{{pixel_width,0},{0,pixel_height},{-pixel_width,0},{0,-pixel_height}};
            for(int i = 0; i < 4; ++i)
            {
                Vec2 p{point.x + offsets[i].x, point.y + offsets[i].y};
                for(auto [tri_id, pos] : neighbor_tris)
                {
                    if(pos != 0) energies[i] += sqrDist(p, triangulation[triangulation.triangles()[tri_id].a]);
                    if(pos != 1) energies[i] += sqrDist(p, triangulation[triangulation.triangles()[tri_id].b]);
                    if(pos != 2) energies[i] += sqrDist(p, triangulation[triangulation.triangles()[tri_id].c]);
                }
            }
            auto& [right_energy, up_energy, left_energy, down_energy] = energies;
            result[i] = {(right_energy - left_energy) / neighbor_tris.size(), (up_energy - down_energy) / neighbor_tris.size()};
        }
        return result;
    }

    auto computeGradients(const Triangulation& triangulation, const std::vector<std::vector<std::pair<unsigned,unsigned char>>>& triangles_per_vertex, const std::vector<TriangulationOptimizer::ErrorData>& error_data, float pixel_width, float pixel_height)
    {
        std::vector<Vec2> result(triangulation.size());
        for(VertexIndice i = 0; i < result.size(); ++i)
        {
            Vec2& gradient = result[i];
            auto& neighbor_tris = triangles_per_vertex[i];
            for(auto [tri_id, pos] : neighbor_tris)
            {
                float errors[4];
                for(int i = 0; i < 4; ++i)
                {
                    auto data = error_data[tri_id * 13 + 1 + pos * 4 + i];
                    errors[i] = data.count ? data.error_total / 255.f / data.count / 2 : 0.f;
                }
                auto& [right_error, up_error, left_error, down_error] = errors;
                gradient.x += (right_error - left_error) / (2*pixel_width) / 3;
                gradient.y += (up_error - down_error) / (2*pixel_height) / 3;
            }
        }
        return result;
    }

    void checkForFlip(unsigned t, Triangulation& triangulation, const std::vector<std::vector<std::pair<unsigned,unsigned char>>>& triangles_per_vertex)
    {
        Triangle tri = triangulation.triangles()[t];
        VertexIndice indices[]{tri.a, tri.b, tri.c};
        for(int i = 0; i < 3; ++i)
        {
            VertexIndice v1 = indices[i];
            VertexIndice v2 = indices[(i+1)%3];
            VertexIndice v3 = indices[(i+2)%3];
            float local_angle = angle(triangulation[v1], triangulation[v3], triangulation[v2]);
            for(auto [other_t,l] : triangles_per_vertex[v1])
            {
                if(other_t==t) continue;
                Triangle other_tri = triangulation.triangles()[other_t];
                VertexIndice other_indices[]{other_tri.a, other_tri.b, other_tri.c};
                bool has_edge = false;
                VertexIndice opposite=v3;
                for(VertexIndice v : other_indices)
                {
                    if(v != v1 && v != v2 && v != v3) opposite = v;
                    if(v == v2) has_edge = true;
                }
                if(has_edge && local_angle + angle(triangulation[v1], triangulation[opposite], triangulation[v2]) >= M_PI)
                {
                    triangulation.flipCommonEdge(t, other_t);
                    return;
                }
            }
        }
    }
}


TriangulationOptimizer::TriangulationOptimizer()
    :_gl{QOpenGLContext::currentContext()->extraFunctions()}
{
    QOpenGLShader vertex_shader{QOpenGLShader::Vertex};
    QOpenGLShader geometry_shader{QOpenGLShader::Geometry};
    QOpenGLShader fragment_shaders[2] = {QOpenGLShader{QOpenGLShader::Fragment}, QOpenGLShader{QOpenGLShader::Fragment}};

    vertex_shader.compileSourceFile(":/Shaders/TriangulationOptimizer/vertex.glsl");
    geometry_shader.compileSourceFile(":/Shaders/TriangulationOptimizer/geometry.glsl");
    fragment_shaders[0].compileSourceFile(":/Shaders/TriangulationOptimizer/fragment1.glsl");
    fragment_shaders[1].compileSourceFile(":/Shaders/TriangulationOptimizer/fragment2.glsl");

    for(int i = 0; i < 2; ++i)
    {
        _programs[i].addShader(&vertex_shader);
        _programs[i].addShader(&geometry_shader);
        _programs[i].addShader(&fragment_shaders[i]);
        _programs[i].link();

        _image_locations[i] = _programs[i].uniformLocation("image");
        _programs[i].removeAllShaders();
    }
}

TriangulationOptimizer::~TriangulationOptimizer()
{
}

std::vector<TriangulationOptimizer::ErrorData> TriangulationOptimizer::computeErrors(const Triangulation& triangulation, int w, int h)
{
    GLuint vao;
    _gl->glGenVertexArrays(1,&vao);
    _gl->glBindVertexArray(vao);
    GLuint rbo;
    _gl->glGenRenderbuffers(1, &rbo);
    _gl->glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    _gl->glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, w, h);
    GLuint fbo;
    _gl->glGenFramebuffers(1, &fbo);
    _gl->glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    _gl->glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);
    _gl->glViewport(0,0,w,h);

    _gl->glDisable(GL_DEPTH_TEST);
    _gl->glDisable(GL_CULL_FACE);

    GLuint buffers[4];
    _gl->glGenBuffers(4, buffers);
    auto& [vertices_buffer, element_buffer, triangle_data_buffer, triangle_error_buffer] = buffers;
    _gl->glBindBuffer(GL_ARRAY_BUFFER, vertices_buffer);
    _gl->glBufferData(GL_ARRAY_BUFFER, triangulation.size() * sizeof(Vec2), triangulation.vertices().data(), GL_STATIC_DRAW);

    _gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);
    static_assert (sizeof(Triangle) == 3*sizeof(VertexIndice));
    _gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangulation.triangles().size() * sizeof(Triangle), triangulation.triangles().data(), GL_STATIC_DRAW);

    size_t total_triangles = triangulation.triangles().size() * 13;
    std::vector<TriangleData> triangle_data(total_triangles);
    _gl->glBindBuffer(GL_SHADER_STORAGE_BUFFER, triangle_data_buffer);
    _gl->glBufferData(GL_SHADER_STORAGE_BUFFER, total_triangles * sizeof(TriangleData), triangle_data.data(), GL_DYNAMIC_COPY);

    std::vector<ErrorData> error_data(total_triangles);
    _gl->glBindBuffer(GL_SHADER_STORAGE_BUFFER, triangle_error_buffer);
    _gl->glBufferData(GL_SHADER_STORAGE_BUFFER, total_triangles * sizeof(ErrorData), /*error_data.data()*/nullptr, GL_DYNAMIC_READ);


    _gl->glEnableVertexAttribArray(0);
    _gl->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    _gl->glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, triangle_data_buffer);
    _gl->glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, triangle_error_buffer);
    for(int i = 0; i < 2; ++i)
    {
        _programs[i].bind();
        _programs[i].setUniformValue(_image_locations[i], 0);
        _gl->glDrawElements(GL_TRIANGLES, triangulation.triangles().size() * 3, GL_UNSIGNED_INT, nullptr);
        _gl->glMemoryBarrier(GL_ALL_BARRIER_BITS /*GL_SHADER_STORAGE_BARRIER_BIT | GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT*/);
    }

    auto s = _gl->glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE,0);
    _gl->glClientWaitSync(s,0,-1);
    _gl->glDeleteSync(s);

    for(auto& e : error_data) e.error_total = 1;
    _gl->glBindBuffer(GL_SHADER_STORAGE_BUFFER, triangle_error_buffer);
    GLvoid* p = _gl->glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, total_triangles * sizeof(ErrorData), GL_MAP_READ_BIT);
    memcpy(error_data.data(), p, total_triangles * sizeof(ErrorData));
    _gl->glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);


    _gl->glDeleteBuffers(4, buffers);
    _gl->glDeleteFramebuffers(1, &fbo);
    _gl->glDeleteRenderbuffers(1, &rbo);
    _gl->glDeleteVertexArrays(1, &vao);

    return error_data;
}

bool tri_eq(const Triangle& l, const Triangle& r)
{
    VertexIndice la[]{l.a,l.b,l.c};
    VertexIndice ra[]{r.a,r.b,r.c};
    for(auto s : la)
    {
        if(std::find(std::begin(ra), std::end(ra), s) == std::end(ra)) return false;
    }
    return true;
}

void TriangulationOptimizer::optimize(Triangulation& triangulation, unsigned texture_handle, bool split)
{
    int w, h;
    _gl->glActiveTexture(GL_TEXTURE0);
    _gl->glBindTexture(GL_TEXTURE_2D, texture_handle);
    _gl->glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
    _gl->glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
    float pixel_width = 1.f / w;
    float pixel_height = 1.f / h;

    auto error_data = computeErrors(triangulation, w, h);

    const auto& triangles_per_vertex = triangulation.trianglesPerVertex();

    std::vector<Vec2> regularisation_gradients = computeRegularisationGradients(triangulation, triangles_per_vertex, pixel_width, pixel_height);
    std::vector<Vec2> gradients = computeGradients(triangulation, triangles_per_vertex, error_data, pixel_width, pixel_height);

    std::vector<Vec2> new_vertices = triangulation.vertices();
    for(VertexIndice i = 0; i < new_vertices.size(); ++i)
    {
        Vec2 gradient{ gradients[i].x * _step + regularisation_gradients[i].x * _regularisation,
            gradients[i].y * _step + regularisation_gradients[i].y * _regularisation };
        gradient.x = std::clamp(gradient.x, -pixel_width * _step_clamp_pixel, pixel_width * _step_clamp_pixel);
        gradient.y = std::clamp(gradient.y, -pixel_height * _step_clamp_pixel, pixel_height * _step_clamp_pixel);
        if(new_vertices[i].x != 0 && new_vertices[i].x != 1 && new_vertices[i].y != 0 && new_vertices[i].y != 1)
        {
            new_vertices[i].x -= gradient.x;
            new_vertices[i].y -= gradient.y;
        }

        for(auto [t,unused] : triangles_per_vertex[i])
        {
            Triangle tri = triangulation.triangles()[t];
            if(sens(tri, triangulation) != sens(tri, new_vertices))
            {
                new_vertices[i] = triangulation[i];
                break;
            }
        }
    }
    std::copy(begin(new_vertices), end(new_vertices), begin(triangulation));

    unsigned tri_count = triangulation.triangles().size();
    unsigned long long total_count = 0;
    for(unsigned t = 0; t < tri_count; ++t)
    {
        total_count += error_data[t*13].count;
    }

    for(unsigned t = 0; t < tri_count; ++t)
    {
        if(split && (float)error_data[t*13].error_total / total_count >= _energy_split_treshold) triangulation.splitTriangle(t);
    }
    for(unsigned t = 0; t < triangulation.triangles().size(); ++t)
    {
        if(triangulation.triangleArea(t) < _min_triangle_area) triangulation.deleteTriangle(t);
    }

    for(unsigned t = 0; t < triangulation.triangles().size(); ++t)
    {
        checkForFlip(t, triangulation, triangles_per_vertex);
    }
}

void TriangulationOptimizer::optimizeSplit(Triangulation& triangulation, unsigned texture_handle)
{
    optimize(triangulation, texture_handle, true);
}

float TriangulationOptimizer::energySplitThreshold() const
{
    return _energy_split_treshold;
}

void TriangulationOptimizer::energySplitThreshold(float val)
{
    _energy_split_treshold = val;
}

float TriangulationOptimizer::minTriangleArea() const
{
    return _min_triangle_area;
}

void TriangulationOptimizer::minTriangleArea(float val)
{
    _min_triangle_area = val;
}

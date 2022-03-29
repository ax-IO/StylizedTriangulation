#include "triangulationoptimizer.h"
#include <QOpenGLShader>
#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>

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

namespace
{
    struct TriangleData
    {
        GLuint color_total_r = 0;
        GLuint color_total_g = 0;
        GLuint color_total_b = 0;
        GLuint count = 0;
    };

    struct ErrorData
    {
        GLuint error_total = 0;
        GLuint count = 100;
    };
}



void TriangulationOptimizer::optimize(Triangulation& triangulation, unsigned texture_handle)
{
    //Appliquer equation 10 sur chaque vertice v
        //h est un parametre
        //Il faut d'abord calculer les gradients en v (7)
            //Calculés à partir des gradients des triangles adjacents
                //Calculer les énergies pour chacune des 12+1 variantes de chaque triangle (11)
                    //2 passes : Génération des couleurs moyennes, puis calcul de l'erreur (4)  -->  Formule de variance, faisable en une seule passe?
    int w, h;
    _gl->glActiveTexture(GL_TEXTURE0);
    _gl->glBindTexture(GL_TEXTURE_2D, texture_handle);
    _gl->glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
    _gl->glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
    float pixel_width = 1.f / w;
    float pixel_height = 1.f / h;

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


    std::vector<std::vector<std::pair<unsigned,unsigned char>>> triangles_per_vertex(triangulation.size());
    for(unsigned i = 0; i < triangulation.triangles().size(); ++i)
    {
        Triangle triangle = triangulation.triangles()[i];
        triangles_per_vertex[triangle.a].emplace_back(i,0);
        triangles_per_vertex[triangle.b].emplace_back(i,1);
        triangles_per_vertex[triangle.c].emplace_back(i,2);
    }

    std::vector<Vec2> new_vertices(triangulation.size());
    for(VertexIndice i = 0; i < new_vertices.size(); ++i)
    {
        Vec2 gradient{};
        auto& neighbor_tris = triangles_per_vertex[i];
        for(auto [tri_id, pos] : neighbor_tris)
        {
            float errors[4];
            for(int i = 0; i < 4; ++i)
            {
                auto data = error_data[tri_id * 13 + 1 + pos * 4 + i];
                errors[i] = data.error_total / 255.f / data.count / 2;
            }
            auto& [right_error, up_error, left_error, down_error] = errors;
            gradient.x += (right_error - left_error) / (2*pixel_width) / 3;
            gradient.y += (up_error - down_error) / (2*pixel_height) / 3;
        }
        //TODO : add normalization term
        new_vertices[i] = triangulation[i];
        if(new_vertices[i].x != 0 && new_vertices[i].x != 1 && new_vertices[i].y != 0 && new_vertices[i].y != 1)
        {
            new_vertices[i].x -= gradient.x * _step;
            new_vertices[i].y -= gradient.y * _step;
        }
    }
    std::copy(begin(new_vertices), end(new_vertices), begin(triangulation));
}

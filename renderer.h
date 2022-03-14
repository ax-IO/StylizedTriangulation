#ifndef RENDERER_H
#define RENDERER_H

#include "triangulation.h"
#include <QOpenGLShaderProgram>
#include <iostream>
#include <QOpenGLShader>
#include <QOpenGLBuffer>
#include <QOpenGLExtraFunctions>

class Renderer
{

private:
    QOpenGLShaderProgram program[2];
    QOpenGLBuffer vertex_buffer;
    QOpenGLBuffer indice_buffer;
    GLuint storage_buffer;

    QOpenGLExtraFunctions* gl_fct;

public:
    Renderer();
    void render(const Triangulation&, unsigned int);
    void init_buffers(const Triangulation&);
    void init_GL();

    ~Renderer();
};

#endif // RENDERER_H

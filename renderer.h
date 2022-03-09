#ifndef RENDERER_H
#define RENDERER_H

#include "triangulation.h"
#include <QOpenGLShaderProgram>
#include <iostream>
#include <QOpenGLShader>
#include <QOpenGLBuffer>

class Renderer
{

private:
    QOpenGLShaderProgram program;
    QOpenGLBuffer vertex_buffer;
    QOpenGLBuffer indice_buffer;

public:
    Renderer();
    void render(const Triangulation&, unsigned int);
    void init_buffers(const Triangulation&);
    void init_GL();

    ~Renderer();
};

#endif // RENDERER_H

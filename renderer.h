#ifndef RENDERER_H
#define RENDERER_H

#include "triangulation.h"
#include <QtOpenGL>
#include <QOpenGLFunctions_2_0>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <iostream>

class Renderer
{

private:
    QOpenGLContext *context;
    QOpenGLShaderProgram program;
    QOpenGLBuffer vertex_buffer;
    QOpenGLBuffer indice_buffer;

public:
    Renderer();
    void render(const Triangulation&, unsigned int);
    void init_buffers(const Triangulation&);
    void init_GL();
};

#endif // RENDERER_H

#ifndef RENDERER_H
#define RENDERER_H

#include "triangulation.h"
#include <QOpenGLShaderProgram>
#include <iostream>
#include <QOpenGLShader>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions_2_0>
#include <QOpenGLExtraFunctions>

#include "gsl/gsl_linalg.h" //librairie à installer (pour calcul de la couleur linéaire)
//#include "gsl_matrix.h"
//#include "gsl_errno.h"

//#include <gsl/gsl_matrix.h> //librairie à installer (pour calcul de la couleur linéaire)
//#include <gsl/gsl_linalg.h>
//#include <gsl/gsl_errno.h>

#define COLOR_LINEAR 0
#define COLOR_CONSTANT 1

class Renderer
{

private:
    QOpenGLShaderProgram program[2];
    QOpenGLBuffer vertex_buffer;
    QOpenGLBuffer indice_buffer;
    GLuint storage_buffer;

    QOpenGLExtraFunctions* gl_fct;
    QOpenGLFunctions_2_0* gl_fct_special;

public:
    Renderer();
    void render(const Triangulation&, unsigned int, int);
    void init_buffers(const Triangulation&, int);
    void init_GL();

    ~Renderer();
};

#endif // RENDERER_H

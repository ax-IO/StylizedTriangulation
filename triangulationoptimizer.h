#ifndef TRIANGULATIONOPTIMIZER_H
#define TRIANGULATIONOPTIMIZER_H

#include "triangulation.h"
#include <QOpenGLShaderProgram>

class QOpenGLExtraFunctions;

class TriangulationOptimizer
{
public:
    TriangulationOptimizer();
    TriangulationOptimizer(const TriangulationOptimizer&) = delete;
    ~TriangulationOptimizer();
    void optimize (Triangulation&, unsigned int);

private:
    QOpenGLShaderProgram _programs[2];
    QOpenGLExtraFunctions* _gl;
    GLuint _image_locations[2];
    float _step = 0.2f;
};

#endif // TRIANGULATIONOPTIMIZER_H

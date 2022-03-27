#ifndef TRIANGULATIONOPTIMIZER_H
#define TRIANGULATIONOPTIMIZER_H

#include "triangulation.h"
#include <QOpenGLShaderProgram>

class QOpenGLExtraFunctions;

class TriangulationOptimizer
{
public:
    struct ErrorData;
    TriangulationOptimizer();
    TriangulationOptimizer(const TriangulationOptimizer&) = delete;
    ~TriangulationOptimizer();
    void optimize (Triangulation&, unsigned int);

//private:
    std::vector<ErrorData> computeErrors(const Triangulation&, int, int);

    QOpenGLShaderProgram _programs[2];
    QOpenGLExtraFunctions* _gl;
    GLuint _image_locations[2];
    float _step = 0.2f;
    float _step_clamp_pixel = 0.2f;
    float _regularisation = 0.001f;
    float _energy_split_treshold = 0.05f;
};

#endif // TRIANGULATIONOPTIMIZER_H

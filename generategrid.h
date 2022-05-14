#ifndef GENERATEGRID_H
#define GENERATEGRID_H

#include "triangulation.h"
#include "common.h" //image ppm utilities
#include "delaunator.hpp" //Delaunay Triangulation
#include <cstdio>
#include <QString>
#include <QDebug>
#include <string>
class GenerateGrid
{
public:
    GenerateGrid(int width, int height);
    void computeTriangulationSplitAndMerge(QString filename, double maxVariance,int maxDist);

    std::vector<Vec2> getVertices();
    std::vector<Triangle> getTriangles();
private:
    std::vector<Vec2> m_vertices;
    std::vector<Triangle> m_triangles;
//    double m_splitAndMergeMaxVariance;
//    int m_splitAndMergeMaxDist;
    int m_width;
    int m_height;

};

#endif // GENERATEGRID_H

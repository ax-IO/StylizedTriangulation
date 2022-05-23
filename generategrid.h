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

    void echantillonageContour(std::vector<unsigned char>& edgeMap, int seuil, int maxPoints, float pointRate);
    void delaunayTriangulation();

    void computeTriangulationGradientMap(QString filename, int seuil, int maxPoints, float pointRate);
    void computeTriangulationSobelMap(QString filename, int seuilFiltre, int seuil, int maxPoints, float pointRate);



    std::vector<double> vectorOfVec2TovectorOfDouble(std::vector<Vec2>);
    std::vector<Vec2> getVertices();
    std::vector<Triangle> getTriangles();
private:
    std::vector<Vec2> m_vertices;
    std::vector<Triangle> m_triangles;
    int m_width;
    int m_height;

};

#endif // GENERATEGRID_H

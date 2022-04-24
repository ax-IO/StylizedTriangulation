#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <iostream>

MainWindow::MainWindow(QWindow *parent)
    : QWindow(parent)
{
    setSurfaceType(QWindow::OpenGLSurface);
}

MainWindow::~MainWindow()
{
}

bool MainWindow::event(QEvent *event)
{
    if(event->type() != QEvent::UpdateRequest) return QWindow::event(event);

    QOpenGLContext context(this);
    context.create();
    context.makeCurrent(this);

//    QImage img(QString(":/toast.png"));
    QImage img(QString(":/capy.png"));

    img.convertTo(QImage::Format_RGBA8888);

    //texture setup
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width(), img.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, img.bits());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    Triangulation tri{5};
    TriangulationOptimizer tri_opt;
    for(int i = 0; i < 1; i ++)
    {
        tri_opt.optimize(tri, tex);
    }

    Renderer renderer;
    renderer.render(tri, tex, COLOR_GRADIENT);

//    for(Vec2 coord : tri.vertices()){
//        std::cout<<"("<<coord.x<<", "<<coord.y<<")"<<std::endl;
//    }

//    for(Triangle tr : tri.triangles()){
//        std::cout<<"a,b,c: "<<tr.a<<", "<<tr.b<<", "<<tr.c<<std::endl;
//    }
    std::cout << "Coucou" << std::endl;
    context.swapBuffers(this);
    return true;
}

void MainWindow::exposeEvent(QExposeEvent *event)
{
    Q_UNUSED(event);

    if (isExposed())
        requestUpdate();
}

// Souvent : optimize(Triangulation, )
// A chaque frame : rendererender(triangulation_optimisee, )

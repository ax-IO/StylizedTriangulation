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
    static bool test = false;
    if(test) return true;
    test = true;
    QOpenGLContext context(this);
    context.create();
    context.makeCurrent(this);
    auto gl = context.functions();
    gl->glViewport(0,0,512,512);
    QImage img(QString(":/toast.png"));
    img.convertTo(QImage::Format_RGBA8888);
    GLuint text;
    gl->glGenTextures(1, &text);
    gl->glBindTexture(GL_TEXTURE_2D, text);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width(), img.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, img.bits());
    gl->glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    gl->glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    TriangulationOptimizer opt;
    Triangulation tri{1};
    opt.optimize(tri, text);
    std::cout << "Coucou" << std::endl;
    for(Vec2 v : tri)
    {
        std::cout << '(' << v.x << ',' << v.y << ")\n";
    }
    std::cout.flush();
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

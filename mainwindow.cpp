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

    Triangulation tri{1};
    Renderer renderer;
    renderer.render(tri, 0);

    for(Vec2 coord : tri.vertices()){
        std::cout<<"("<<coord.x<<", "<<coord.y<<")"<<std::endl;
    }
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

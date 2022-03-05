#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    Renderer renderer;
    Triangulation tri(6);
    renderer.init_buffers(tri);
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}

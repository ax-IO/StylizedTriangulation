#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setWidth(512);
    w.setHeight(512);
    w.show();
    return a.exec();
}

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWindow>
#include "triangulation.h"
#include "triangulationoptimizer.h"
#include "renderer.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QWindow
{
    Q_OBJECT

public:
    MainWindow(QWindow *parent = nullptr);
    ~MainWindow();

    bool event(QEvent *event) override;
    void exposeEvent(QExposeEvent *event) override;

private:
};
#endif // MAINWINDOW_H

#ifndef GLWINDOW_H
#define GLWINDOW_H

#include <QWindow>
#include "triangulation.h"
#include "triangulationoptimizer.h"
#include "renderer.h"


class GLWindow : public QWindow
{
    Q_OBJECT

public:
    GLWindow(QWindow *parent = nullptr);
    GLWindow(GLuint texture, QWindow *parent = nullptr);
    GLWindow(QString filepath, QWindow *parent = nullptr);

    ~GLWindow();

    bool event(QEvent *event) override;
    void exposeEvent(QExposeEvent *event) override;

    GLuint tex;
    QString fp ;

private:
};

#endif // GLWINDOW_H

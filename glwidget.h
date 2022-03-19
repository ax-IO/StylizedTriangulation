#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QColor>
#include <QOpenGLBuffer>
#include <QOpenGLExtraFunctions>
#include <QOpenGLFunctions>
#include <QOpenGLWidget>

#include <QPoint>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QOpenGLContext>

#include "triangulation.h"
#include "triangulationoptimizer.h"
#include "renderer.h"

#define RGB_MIN 0.0f
#define RGB_MAX 255.0f
class GLWidget : public QOpenGLWidget, public QOpenGLFunctions {
  Q_OBJECT

public:
  GLWidget(QString filepath, QWidget *parent = nullptr);

//~GLWidget();
protected:
  void initializeGL() override;
  void paintGL() override;
  void resizeGL(int w, int h) override;
  void mousePressEvent(QMouseEvent *event) override;
private:
  void qColorToRGB(const QColor &C, float&r, float &g, float &b) const;
  float normalize_0_1 (float val, float min, float max)const;

private:
  GLuint tex;
  QString fp ;

  QOpenGLExtraFunctions* gl_fct;

  QPoint point;
};


#endif // GLWIDGET_H

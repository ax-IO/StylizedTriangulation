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

#include "math.h"
#include <algorithm>

#define RGB_MIN 0.0f
#define RGB_MAX 255.0f

class GLWidget : public QOpenGLWidget, public QOpenGLFunctions {
  Q_OBJECT

public:
  GLWidget(QString filepath, QWidget *parent = nullptr);

  ~GLWidget();

  int getGridResolution();
  void changeRegularGridResolution(int resolution);

  void renderModeConstant();
  void renderModeGradient();

  void optimizationPass();
  void optimizationSplitPass();
protected:
  void initializeGL() override;
  void paintGL() override;
  void resizeGL(int w, int h) override;
  void mousePressEvent(QMouseEvent *event) override;
private:
  void qColorToRGB(const QColor &C, float&r, float &g, float &b) const;
  float normalize_0_1 (float val, float min, float max)const;
  Vec2 coord_imageToTexture(QPoint point_image, int width, int height);
  int coord_textureToIndex (Vec2 texturePoint, Triangulation tri);

private:
  GLuint tex;
  QString fp ;

  int m_width;
  int m_height;

  size_t m_gridResolution = 2;
  int m_renderMode;

  Renderer *m_renderer;
  Triangulation m_tri;
  TriangulationOptimizer *m_tri_opt;

  QOpenGLExtraFunctions* gl_fct;

  QPoint point;
};


#endif // GLWIDGET_H

#include "glwidget.h"


#include <QPaintEvent>

GLWidget::GLWidget(QString filepath, QWidget *parent)
    : QOpenGLWidget{parent}, fp(filepath)
{
    qDebug() << "Filepath = " << fp << Qt::endl;
}


void GLWidget::initializeGL() {
  qDebug() << "initializeGL()" << Qt::endl;
  gl_fct = QOpenGLContext::currentContext()->extraFunctions();
  //  initializeOpenGLFunctions();
  float r, g, b;
  qColorToRGB(Qt::red, r, g, b);
  gl_fct->glClearColor(r, g, b, 1.0f);

  QImage img(fp);

  img.convertTo(QImage::Format_RGBA8888);

  // texture setup
  gl_fct->glGenTextures(1, &tex);
  gl_fct->glBindTexture(GL_TEXTURE_2D, tex);
  gl_fct->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width(), img.height(), 0,
                       GL_RGBA, GL_UNSIGNED_BYTE, img.bits());
  gl_fct->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  gl_fct->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void GLWidget::paintGL() {
//  qDebug() << "paintGL()" << Qt::endl;
  gl_fct->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


  Triangulation tri{12};
  //      TriangulationOptimizer tri_opt;
  //  tri_opt.optimize(tri, tex);
    Renderer renderer;
    renderer.render(tri, tex);

}

void GLWidget::resizeGL(int w, int h) {
  qDebug() << "resizeGL(" << w << "," << h << ")" << Qt::endl;
    gl_fct->glViewport(0, 0, w, h);
  //  glMatrixMode(GL_PROJECTION);
  //  glLoadIdentity();
  //  glMatrixMode(GL_MODELVIEW);
  //  glLoadIdentity();
}

void GLWidget::qColorToRGB(const QColor &C, float &r, float &g,
                                 float &b) const {
  r = normalize_0_1(C.red(), RGB_MIN, RGB_MAX);
  g = normalize_0_1(C.green(), RGB_MIN, RGB_MAX);
  b = normalize_0_1(C.blue(), RGB_MIN, RGB_MAX);
}

float GLWidget::normalize_0_1(float val, float min, float max) const {
  return (val - min) / (max - min);
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
    point = event->pos();
    qDebug()<< "(" <<point.x()<< ","<<point.y()<<")"<<Qt::endl;
}

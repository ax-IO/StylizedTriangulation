#include "glwindow.h"

#include <QImageReader>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <iostream>

GLWindow::GLWindow(QWindow *parent) : QWindow(parent) {
  setSurfaceType(QWindow::OpenGLSurface);
}

GLWindow::GLWindow(GLuint texture, QWindow *parent)
    : QWindow(parent), tex(texture) {
  setSurfaceType(QWindow::OpenGLSurface);
  //    qDebug() << tex << Qt::endl;
}

GLWindow::GLWindow(QString filepath, QWindow *parent)
    : QWindow(parent), fp(filepath) {
    setSurfaceType(QWindow::OpenGLSurface);
}

GLWindow::~GLWindow() {}

bool GLWindow::event(QEvent *event) {
  if (event->type() != QEvent::UpdateRequest)
    return QWindow::event(event);

  QOpenGLContext context(this);
  context.create();
  context.makeCurrent(this);

  //   QImage img(QString(":/toast.png"));
//  QImage img(QString(":/capy.png"));
//  QImage img(QString("/home/axiom/dev/Qt/StylizedTriangulation/lena.png"));
  QImage img(fp);


  //   QImage img;
  //   img.load(QString("/home/axiom/dev/Qt/StylizedTriangulation/capy.png"));

  //  QImageReader reader("/home/axiom/dev/Qt/StylizedTriangulation/capy.png");
  //  reader.setAutoTransform(true);
  //  const QImage newImage = reader.read();
  //  qDebug() << fp << Qt::endl;

  img.convertTo(QImage::Format_RGBA8888);

  // texture setup
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width(), img.height(), 0, GL_RGBA,
               GL_UNSIGNED_BYTE, img.bits());
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

//  Triangulation tri{12};
  Triangulation tri{50};

  Renderer renderer;
  renderer.render(tri, tex);

  //    for(Vec2 coord : tri.vertices()){
  //        std::cout<<"("<<coord.x<<", "<<coord.y<<")"<<std::endl;
  //    }

  //    for(Triangle tr : tri.triangles()){
  //        std::cout<<"a,b,c: "<<tr.a<<", "<<tr.b<<", "<<tr.c<<std::endl;
  //    }
  //    std::cout << "Coucou" << std::endl;
  context.swapBuffers(this);
  return true;
}

void GLWindow::exposeEvent(QExposeEvent *event) {
  Q_UNUSED(event);

  if (isExposed())
    requestUpdate();
}
